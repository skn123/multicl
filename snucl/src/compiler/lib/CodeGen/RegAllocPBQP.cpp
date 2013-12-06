//===------ RegAllocPBQP.cpp ---- PBQP Register Allocator -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a Partitioned Boolean Quadratic Programming (PBQP) based
// register allocator for LLVM. This allocator works by constructing a PBQP
// problem representing the register allocation problem under consideration,
// solving this using a PBQP solver, and mapping the solution back to a
// register assignment. If any variables are selected for spilling then spill
// code is inserted and the process repeated.
//
// The PBQP solver (pbqp.c) provided for this allocator uses a heuristic tuned
// for register allocation. For more information on PBQP for register
// allocation, see the following papers:
//
//   (1) Hames, L. and Scholz, B. 2006. Nearly optimal register allocation with
//   PBQP. In Proceedings of the 7th Joint Modular Languages Conference
//   (JMLC'06). LNCS, vol. 4228. Springer, New York, NY, USA. 346-361.
//
//   (2) Scholz, B., Eckstein, E. 2002. Register allocation for irregular
//   architectures. In Proceedings of the Joint Conference on Languages,
//   Compilers and Tools for Embedded Systems (LCTES'02), ACM Press, New York,
//   NY, USA, 139-148.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "regalloc"

#include "RenderMachineFunction.h"
#include "Splitter.h"
#include "VirtRegMap.h"
#include "VirtRegRewriter.h"
#include "llvm/CodeGen/CalcSpillWeights.h"
#include "llvm/CodeGen/LiveIntervalAnalysis.h"
#include "llvm/CodeGen/LiveStackAnalysis.h"
#include "llvm/CodeGen/RegAllocPBQP.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/PBQP/HeuristicSolver.h"
#include "llvm/CodeGen/PBQP/Graph.h"
#include "llvm/CodeGen/PBQP/Heuristics/Briggs.h"
#include "llvm/CodeGen/RegAllocRegistry.h"
#include "llvm/CodeGen/RegisterCoalescer.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetMachine.h"
#include <limits>
#include <memory>
#include <set>
#include <vector>

using namespace llvm;

static RegisterRegAlloc
registerPBQPRepAlloc("pbqp", "PBQP register allocator",
                       createDefaultPBQPRegisterAllocator);

static cl::opt<bool>
pbqpCoalescing("pbqp-coalescing",
                cl::desc("Attempt coalescing during PBQP register allocation."),
                cl::init(false), cl::Hidden);

static cl::opt<bool>
pbqpPreSplitting("pbqp-pre-splitting",
                 cl::desc("Pre-split before PBQP register allocation."),
                 cl::init(false), cl::Hidden);

namespace {

///
/// PBQP based allocators solve the register allocation problem by mapping
/// register allocation problems to Partitioned Boolean Quadratic
/// Programming problems.
class RegAllocPBQP : public MachineFunctionPass {
public:

  static char ID;

  /// Construct a PBQP register allocator.
  RegAllocPBQP(std::auto_ptr<PBQPBuilder> b)
      : MachineFunctionPass(ID), builder(b) {
    initializeSlotIndexesPass(*PassRegistry::getPassRegistry());
    initializeLiveIntervalsPass(*PassRegistry::getPassRegistry());
    initializeRegisterCoalescerAnalysisGroup(*PassRegistry::getPassRegistry());
    initializeCalculateSpillWeightsPass(*PassRegistry::getPassRegistry());
    initializeLiveStacksPass(*PassRegistry::getPassRegistry());
    initializeMachineLoopInfoPass(*PassRegistry::getPassRegistry());
    initializeLoopSplitterPass(*PassRegistry::getPassRegistry());
    initializeVirtRegMapPass(*PassRegistry::getPassRegistry());
    initializeRenderMachineFunctionPass(*PassRegistry::getPassRegistry());
  }

  /// Return the pass name.
  virtual const char* getPassName() const {
    return "PBQP Register Allocator";
  }

  /// PBQP analysis usage.
  virtual void getAnalysisUsage(AnalysisUsage &au) const;

  /// Perform register allocation
  virtual bool runOnMachineFunction(MachineFunction &MF);

private:

  typedef std::map<const LiveInterval*, unsigned> LI2NodeMap;
  typedef std::vector<const LiveInterval*> Node2LIMap;
  typedef std::vector<unsigned> AllowedSet;
  typedef std::vector<AllowedSet> AllowedSetMap;
  typedef std::pair<unsigned, unsigned> RegPair;
  typedef std::map<RegPair, PBQP::PBQPNum> CoalesceMap;
  typedef std::vector<PBQP::Graph::NodeItr> NodeVector;
  typedef std::set<unsigned> RegSet;


  std::auto_ptr<PBQPBuilder> builder;

  MachineFunction *mf;
  const TargetMachine *tm;
  const TargetRegisterInfo *tri;
  const TargetInstrInfo *tii;
  const MachineLoopInfo *loopInfo;
  MachineRegisterInfo *mri;
  RenderMachineFunction *rmf;

  LiveIntervals *lis;
  LiveStacks *lss;
  VirtRegMap *vrm;

  RegSet vregsToAlloc, emptyIntervalVRegs;

  /// \brief Finds the initial set of vreg intervals to allocate.
  void findVRegIntervalsToAlloc();

  /// \brief Adds a stack interval if the given live interval has been
  /// spilled. Used to support stack slot coloring.
  void addStackInterval(const LiveInterval *spilled,MachineRegisterInfo* mri);

  /// \brief Given a solved PBQP problem maps this solution back to a register
  /// assignment.
  bool mapPBQPToRegAlloc(const PBQPRAProblem &problem,
                         const PBQP::Solution &solution);

  /// \brief Postprocessing before final spilling. Sets basic block "live in"
  /// variables.
  void finalizeAlloc() const;

};

char RegAllocPBQP::ID = 0;

} // End anonymous namespace.

unsigned PBQPRAProblem::getVRegForNode(PBQP::Graph::ConstNodeItr node) const {
  Node2VReg::const_iterator vregItr = node2VReg.find(node);
  assert(vregItr != node2VReg.end() && "No vreg for node.");
  return vregItr->second;
}

PBQP::Graph::NodeItr PBQPRAProblem::getNodeForVReg(unsigned vreg) const {
  VReg2Node::const_iterator nodeItr = vreg2Node.find(vreg);
  assert(nodeItr != vreg2Node.end() && "No node for vreg.");
  return nodeItr->second;
  
}

const PBQPRAProblem::AllowedSet&
  PBQPRAProblem::getAllowedSet(unsigned vreg) const {
  AllowedSetMap::const_iterator allowedSetItr = allowedSets.find(vreg);
  assert(allowedSetItr != allowedSets.end() && "No pregs for vreg.");
  const AllowedSet &allowedSet = allowedSetItr->second;
  return allowedSet;
}

unsigned PBQPRAProblem::getPRegForOption(unsigned vreg, unsigned option) const {
  assert(isPRegOption(vreg, option) && "Not a preg option.");

  const AllowedSet& allowedSet = getAllowedSet(vreg);
  assert(option <= allowedSet.size() && "Option outside allowed set.");
  return allowedSet[option - 1];
}

std::auto_ptr<PBQPRAProblem> PBQPBuilder::build(MachineFunction *mf,
                                                const LiveIntervals *lis,
                                                const MachineLoopInfo *loopInfo,
                                                const RegSet &vregs) {

  typedef std::vector<const LiveInterval*> LIVector;

  MachineRegisterInfo *mri = &mf->getRegInfo();
  const TargetRegisterInfo *tri = mf->getTarget().getRegisterInfo();  

  std::auto_ptr<PBQPRAProblem> p(new PBQPRAProblem());
  PBQP::Graph &g = p->getGraph();
  RegSet pregs;

  // Collect the set of preg intervals, record that they're used in the MF.
  for (LiveIntervals::const_iterator itr = lis->begin(), end = lis->end();
       itr != end; ++itr) {
    if (TargetRegisterInfo::isPhysicalRegister(itr->first)) {
      pregs.insert(itr->first);
      mri->setPhysRegUsed(itr->first);
    }
  }

  BitVector reservedRegs = tri->getReservedRegs(*mf);

  // Iterate over vregs. 
  for (RegSet::const_iterator vregItr = vregs.begin(), vregEnd = vregs.end();
       vregItr != vregEnd; ++vregItr) {
    unsigned vreg = *vregItr;
    const TargetRegisterClass *trc = mri->getRegClass(vreg);
    const LiveInterval *vregLI = &lis->getInterval(vreg);

    // Compute an initial allowed set for the current vreg.
    typedef std::vector<unsigned> VRAllowed;
    VRAllowed vrAllowed;
    for (TargetRegisterClass::iterator aoItr = trc->allocation_order_begin(*mf),
                                       aoEnd = trc->allocation_order_end(*mf);
         aoItr != aoEnd; ++aoItr) {
      unsigned preg = *aoItr;
      if (!reservedRegs.test(preg)) {
        vrAllowed.push_back(preg);
      }
    }

    // Remove any physical registers which overlap.
    for (RegSet::const_iterator pregItr = pregs.begin(),
                                pregEnd = pregs.end();
         pregItr != pregEnd; ++pregItr) {
      unsigned preg = *pregItr;
      const LiveInterval *pregLI = &lis->getInterval(preg);

      if (pregLI->empty()) {
        continue;
      }

      if (!vregLI->overlaps(*pregLI)) {
        continue;
      }

      // Remove the register from the allowed set.
      VRAllowed::iterator eraseItr =
        std::find(vrAllowed.begin(), vrAllowed.end(), preg);

      if (eraseItr != vrAllowed.end()) {
        vrAllowed.erase(eraseItr);
      }

      // Also remove any aliases.
      const unsigned *aliasItr = tri->getAliasSet(preg);
      if (aliasItr != 0) {
        for (; *aliasItr != 0; ++aliasItr) {
          VRAllowed::iterator eraseItr =
            std::find(vrAllowed.begin(), vrAllowed.end(), *aliasItr);

          if (eraseItr != vrAllowed.end()) {
            vrAllowed.erase(eraseItr);
          }
        }
      }
    }

    // Construct the node.
    PBQP::Graph::NodeItr node = 
      g.addNode(PBQP::Vector(vrAllowed.size() + 1, 0));

    // Record the mapping and allowed set in the problem.
    p->recordVReg(vreg, node, vrAllowed.begin(), vrAllowed.end());

    PBQP::PBQPNum spillCost = (vregLI->weight != 0.0) ?
        vregLI->weight : std::numeric_limits<PBQP::PBQPNum>::min();

    addSpillCosts(g.getNodeCosts(node), spillCost);
  }

  for (RegSet::const_iterator vr1Itr = vregs.begin(), vrEnd = vregs.end();
         vr1Itr != vrEnd; ++vr1Itr) {
    unsigned vr1 = *vr1Itr;
    const LiveInterval &l1 = lis->getInterval(vr1);
    const PBQPRAProblem::AllowedSet &vr1Allowed = p->getAllowedSet(vr1);

    for (RegSet::const_iterator vr2Itr = llvm::next(vr1Itr);
         vr2Itr != vrEnd; ++vr2Itr) {
      unsigned vr2 = *vr2Itr;
      const LiveInterval &l2 = lis->getInterval(vr2);
      const PBQPRAProblem::AllowedSet &vr2Allowed = p->getAllowedSet(vr2);

      assert(!l2.empty() && "Empty interval in vreg set?");
      if (l1.overlaps(l2)) {
        PBQP::Graph::EdgeItr edge =
          g.addEdge(p->getNodeForVReg(vr1), p->getNodeForVReg(vr2),
                    PBQP::Matrix(vr1Allowed.size()+1, vr2Allowed.size()+1, 0));

        addInterferenceCosts(g.getEdgeCosts(edge), vr1Allowed, vr2Allowed, tri);
      }
    }
  }

  return p;
}

void PBQPBuilder::addSpillCosts(PBQP::Vector &costVec,
                                PBQP::PBQPNum spillCost) {
  costVec[0] = spillCost;
}

void PBQPBuilder::addInterferenceCosts(
                                    PBQP::Matrix &costMat,
                                    const PBQPRAProblem::AllowedSet &vr1Allowed,
                                    const PBQPRAProblem::AllowedSet &vr2Allowed,
                                    const TargetRegisterInfo *tri) {
  assert(costMat.getRows() == vr1Allowed.size() + 1 && "Matrix height mismatch.");
  assert(costMat.getCols() == vr2Allowed.size() + 1 && "Matrix width mismatch.");

  for (unsigned i = 0; i != vr1Allowed.size(); ++i) {
    unsigned preg1 = vr1Allowed[i];

    for (unsigned j = 0; j != vr2Allowed.size(); ++j) {
      unsigned preg2 = vr2Allowed[j];

      if (tri->regsOverlap(preg1, preg2)) {
        costMat[i + 1][j + 1] = std::numeric_limits<PBQP::PBQPNum>::infinity();
      }
    }
  }
}

std::auto_ptr<PBQPRAProblem> PBQPBuilderWithCoalescing::build(
                                                MachineFunction *mf,
                                                const LiveIntervals *lis,
                                                const MachineLoopInfo *loopInfo,
                                                const RegSet &vregs) {

  std::auto_ptr<PBQPRAProblem> p = PBQPBuilder::build(mf, lis, loopInfo, vregs);
  PBQP::Graph &g = p->getGraph();

  const TargetMachine &tm = mf->getTarget();
  CoalescerPair cp(*tm.getInstrInfo(), *tm.getRegisterInfo());

  // Scan the machine function and add a coalescing cost whenever CoalescerPair
  // gives the Ok.
  for (MachineFunction::const_iterator mbbItr = mf->begin(),
                                       mbbEnd = mf->end();
       mbbItr != mbbEnd; ++mbbItr) {
    const MachineBasicBlock *mbb = &*mbbItr;

    for (MachineBasicBlock::const_iterator miItr = mbb->begin(),
                                           miEnd = mbb->end();
         miItr != miEnd; ++miItr) {
      const MachineInstr *mi = &*miItr;

      if (!cp.setRegisters(mi)) {
        continue; // Not coalescable.
      }

      if (cp.getSrcReg() == cp.getDstReg()) {
        continue; // Already coalesced.
      }

      unsigned dst = cp.getDstReg(),
               src = cp.getSrcReg();

      const float copyFactor = 0.5; // Cost of copy relative to load. Current
      // value plucked randomly out of the air.
                                      
      PBQP::PBQPNum cBenefit =
        copyFactor * LiveIntervals::getSpillWeight(false, true,
                                                   loopInfo->getLoopDepth(mbb));

      if (cp.isPhys()) {
        if (!lis->isAllocatable(dst)) {
          continue;
        }

        const PBQPRAProblem::AllowedSet &allowed = p->getAllowedSet(src);
        unsigned pregOpt = 0;  
        while (pregOpt < allowed.size() && allowed[pregOpt] != dst) {
          ++pregOpt;
        }
        if (pregOpt < allowed.size()) {
          ++pregOpt; // +1 to account for spill option.
          PBQP::Graph::NodeItr node = p->getNodeForVReg(src);
          addPhysRegCoalesce(g.getNodeCosts(node), pregOpt, cBenefit);
        }
      } else {
        const PBQPRAProblem::AllowedSet *allowed1 = &p->getAllowedSet(dst);
        const PBQPRAProblem::AllowedSet *allowed2 = &p->getAllowedSet(src);
        PBQP::Graph::NodeItr node1 = p->getNodeForVReg(dst);
        PBQP::Graph::NodeItr node2 = p->getNodeForVReg(src);
        PBQP::Graph::EdgeItr edge = g.findEdge(node1, node2);
        if (edge == g.edgesEnd()) {
          edge = g.addEdge(node1, node2, PBQP::Matrix(allowed1->size() + 1,
                                                      allowed2->size() + 1,
                                                      0));
        } else {
          if (g.getEdgeNode1(edge) == node2) {
            std::swap(node1, node2);
            std::swap(allowed1, allowed2);
          }
        }
            
        addVirtRegCoalesce(g.getEdgeCosts(edge), *allowed1, *allowed2,
                           cBenefit);
      }
    }
  }

  return p;
}

void PBQPBuilderWithCoalescing::addPhysRegCoalesce(PBQP::Vector &costVec,
                                                   unsigned pregOption,
                                                   PBQP::PBQPNum benefit) {
  costVec[pregOption] += -benefit;
}

void PBQPBuilderWithCoalescing::addVirtRegCoalesce(
                                    PBQP::Matrix &costMat,
                                    const PBQPRAProblem::AllowedSet &vr1Allowed,
                                    const PBQPRAProblem::AllowedSet &vr2Allowed,
                                    PBQP::PBQPNum benefit) {

  assert(costMat.getRows() == vr1Allowed.size() + 1 && "Size mismatch.");
  assert(costMat.getCols() == vr2Allowed.size() + 1 && "Size mismatch.");

  for (unsigned i = 0; i != vr1Allowed.size(); ++i) {
    unsigned preg1 = vr1Allowed[i];
    for (unsigned j = 0; j != vr2Allowed.size(); ++j) {
      unsigned preg2 = vr2Allowed[j];

      if (preg1 == preg2) {
        costMat[i + 1][j + 1] += -benefit;
      } 
    }
  }
}


void RegAllocPBQP::getAnalysisUsage(AnalysisUsage &au) const {
  au.addRequired<SlotIndexes>();
  au.addPreserved<SlotIndexes>();
  au.addRequired<LiveIntervals>();
  //au.addRequiredID(SplitCriticalEdgesID);
  au.addRequired<RegisterCoalescer>();
  au.addRequired<CalculateSpillWeights>();
  au.addRequired<LiveStacks>();
  au.addPreserved<LiveStacks>();
  au.addRequired<MachineLoopInfo>();
  au.addPreserved<MachineLoopInfo>();
  if (pbqpPreSplitting)
    au.addRequired<LoopSplitter>();
  au.addRequired<VirtRegMap>();
  au.addRequired<RenderMachineFunction>();
  MachineFunctionPass::getAnalysisUsage(au);
}

void RegAllocPBQP::findVRegIntervalsToAlloc() {

  // Iterate over all live ranges.
  for (LiveIntervals::iterator itr = lis->begin(), end = lis->end();
       itr != end; ++itr) {

    // Ignore physical ones.
    if (TargetRegisterInfo::isPhysicalRegister(itr->first))
      continue;

    LiveInterval *li = itr->second;

    // If this live interval is non-empty we will use pbqp to allocate it.
    // Empty intervals we allocate in a simple post-processing stage in
    // finalizeAlloc.
    if (!li->empty()) {
      vregsToAlloc.insert(li->reg);
    } else {
      emptyIntervalVRegs.insert(li->reg);
    }
  }
}

void RegAllocPBQP::addStackInterval(const LiveInterval *spilled,
                                    MachineRegisterInfo* mri) {
  int stackSlot = vrm->getStackSlot(spilled->reg);

  if (stackSlot == VirtRegMap::NO_STACK_SLOT) {
    return;
  }

  const TargetRegisterClass *RC = mri->getRegClass(spilled->reg);
  LiveInterval &stackInterval = lss->getOrCreateInterval(stackSlot, RC);

  VNInfo *vni;
  if (stackInterval.getNumValNums() != 0) {
    vni = stackInterval.getValNumInfo(0);
  } else {
    vni = stackInterval.getNextValue(
      SlotIndex(), 0, lss->getVNInfoAllocator());
  }

  LiveInterval &rhsInterval = lis->getInterval(spilled->reg);
  stackInterval.MergeRangesInAsValue(rhsInterval, vni);
}

bool RegAllocPBQP::mapPBQPToRegAlloc(const PBQPRAProblem &problem,
                                     const PBQP::Solution &solution) {
  // Set to true if we have any spills
  bool anotherRoundNeeded = false;

  // Clear the existing allocation.
  vrm->clearAllVirt();

  const PBQP::Graph &g = problem.getGraph();
  // Iterate over the nodes mapping the PBQP solution to a register
  // assignment.
  for (PBQP::Graph::ConstNodeItr node = g.nodesBegin(),
                                 nodeEnd = g.nodesEnd();
       node != nodeEnd; ++node) {
    unsigned vreg = problem.getVRegForNode(node);
    unsigned alloc = solution.getSelection(node);

    if (problem.isPRegOption(vreg, alloc)) {
      unsigned preg = problem.getPRegForOption(vreg, alloc);    
      DEBUG(dbgs() << "VREG " << vreg << " -> " << tri->getName(preg) << "\n");
      assert(preg != 0 && "Invalid preg selected.");
      vrm->assignVirt2Phys(vreg, preg);      
    } else if (problem.isSpillOption(vreg, alloc)) {
      vregsToAlloc.erase(vreg);
      const LiveInterval* spillInterval = &lis->getInterval(vreg);
      double oldWeight = spillInterval->weight;
      SmallVector<LiveInterval*, 8> spillIs;
      rmf->rememberUseDefs(spillInterval);
      std::vector<LiveInterval*> newSpills =
        lis->addIntervalsForSpills(*spillInterval, spillIs, loopInfo, *vrm);
      addStackInterval(spillInterval, mri);
      rmf->rememberSpills(spillInterval, newSpills);

      (void) oldWeight;
      DEBUG(dbgs() << "VREG " << vreg << " -> SPILLED (Cost: "
                   << oldWeight << ", New vregs: ");

      // Copy any newly inserted live intervals into the list of regs to
      // allocate.
      for (std::vector<LiveInterval*>::const_iterator
           itr = newSpills.begin(), end = newSpills.end();
           itr != end; ++itr) {
        assert(!(*itr)->empty() && "Empty spill range.");
        DEBUG(dbgs() << (*itr)->reg << " ");
        vregsToAlloc.insert((*itr)->reg);
      }

      DEBUG(dbgs() << ")\n");

      // We need another round if spill intervals were added.
      anotherRoundNeeded |= !newSpills.empty();
    } else {
      assert(false && "Unknown allocation option.");
    }
  }

  return !anotherRoundNeeded;
}


void RegAllocPBQP::finalizeAlloc() const {
  typedef LiveIntervals::iterator LIIterator;
  typedef LiveInterval::Ranges::const_iterator LRIterator;

  // First allocate registers for the empty intervals.
  for (RegSet::const_iterator
         itr = emptyIntervalVRegs.begin(), end = emptyIntervalVRegs.end();
         itr != end; ++itr) {
    LiveInterval *li = &lis->getInterval(*itr);

    unsigned physReg = vrm->getRegAllocPref(li->reg);

    if (physReg == 0) {
      const TargetRegisterClass *liRC = mri->getRegClass(li->reg);
      physReg = *liRC->allocation_order_begin(*mf);
    }

    vrm->assignVirt2Phys(li->reg, physReg);
  }

  // Finally iterate over the basic blocks to compute and set the live-in sets.
  SmallVector<MachineBasicBlock*, 8> liveInMBBs;
  MachineBasicBlock *entryMBB = &*mf->begin();

  for (LIIterator liItr = lis->begin(), liEnd = lis->end();
       liItr != liEnd; ++liItr) {

    const LiveInterval *li = liItr->second;
    unsigned reg = 0;

    // Get the physical register for this interval
    if (TargetRegisterInfo::isPhysicalRegister(li->reg)) {
      reg = li->reg;
    } else if (vrm->isAssignedReg(li->reg)) {
      reg = vrm->getPhys(li->reg);
    } else {
      // Ranges which are assigned a stack slot only are ignored.
      continue;
    }

    if (reg == 0) {
      // Filter out zero regs - they're for intervals that were spilled.
      continue;
    }

    // Iterate over the ranges of the current interval...
    for (LRIterator lrItr = li->begin(), lrEnd = li->end();
         lrItr != lrEnd; ++lrItr) {

      // Find the set of basic blocks which this range is live into...
      if (lis->findLiveInMBBs(lrItr->start, lrItr->end,  liveInMBBs)) {
        // And add the physreg for this interval to their live-in sets.
        for (unsigned i = 0; i != liveInMBBs.size(); ++i) {
          if (liveInMBBs[i] != entryMBB) {
            if (!liveInMBBs[i]->isLiveIn(reg)) {
              liveInMBBs[i]->addLiveIn(reg);
            }
          }
        }
        liveInMBBs.clear();
      }
    }
  }

}

bool RegAllocPBQP::runOnMachineFunction(MachineFunction &MF) {

  mf = &MF;
  tm = &mf->getTarget();
  tri = tm->getRegisterInfo();
  tii = tm->getInstrInfo();
  mri = &mf->getRegInfo(); 

  lis = &getAnalysis<LiveIntervals>();
  lss = &getAnalysis<LiveStacks>();
  loopInfo = &getAnalysis<MachineLoopInfo>();
  rmf = &getAnalysis<RenderMachineFunction>();

  vrm = &getAnalysis<VirtRegMap>();


  DEBUG(dbgs() << "PBQP Register Allocating for " << mf->getFunction()->getName() << "\n");

  // Allocator main loop:
  //
  // * Map current regalloc problem to a PBQP problem
  // * Solve the PBQP problem
  // * Map the solution back to a register allocation
  // * Spill if necessary
  //
  // This process is continued till no more spills are generated.

  // Find the vreg intervals in need of allocation.
  findVRegIntervalsToAlloc();

  // If there are non-empty intervals allocate them using pbqp.
  if (!vregsToAlloc.empty()) {

    bool pbqpAllocComplete = false;
    unsigned round = 0;

    while (!pbqpAllocComplete) {
      DEBUG(dbgs() << "  PBQP Regalloc round " << round << ":\n");

      std::auto_ptr<PBQPRAProblem> problem =
        builder->build(mf, lis, loopInfo, vregsToAlloc);
      PBQP::Solution solution =
        PBQP::HeuristicSolver<PBQP::Heuristics::Briggs>::solve(
          problem->getGraph());

      pbqpAllocComplete = mapPBQPToRegAlloc(*problem, solution);

      ++round;
    }
  }

  // Finalise allocation, allocate empty ranges.
  finalizeAlloc();

  rmf->renderMachineFunction("After PBQP register allocation.", vrm);

  vregsToAlloc.clear();
  emptyIntervalVRegs.clear();

  DEBUG(dbgs() << "Post alloc VirtRegMap:\n" << *vrm << "\n");

  // Run rewriter
  std::auto_ptr<VirtRegRewriter> rewriter(createVirtRegRewriter());

  rewriter->runOnMachineFunction(*mf, *vrm, lis);

  return true;
}

FunctionPass* llvm::createPBQPRegisterAllocator(
                                           std::auto_ptr<PBQPBuilder> builder) {
  return new RegAllocPBQP(builder);
}

FunctionPass* llvm::createDefaultPBQPRegisterAllocator() {
  if (pbqpCoalescing) {
    return createPBQPRegisterAllocator(
             std::auto_ptr<PBQPBuilder>(new PBQPBuilderWithCoalescing()));
  } // else
  return createPBQPRegisterAllocator(
           std::auto_ptr<PBQPBuilder>(new PBQPBuilder()));
}

#undef DEBUG_TYPE
