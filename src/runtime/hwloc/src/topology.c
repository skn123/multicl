/*
 * Copyright © 2009 CNRS
 * Copyright © 2009-2014 Inria.  All rights reserved.
 * Copyright © 2009-2012 Université Bordeaux 1
 * Copyright © 2009-2011 Cisco Systems, Inc.  All rights reserved.
 * See COPYING in top-level directory.
 */

#include <private/autogen/config.h>

#define _ATFILE_SOURCE
#include <assert.h>
#include <sys/types.h>
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <float.h>

#include <hwloc.h>
#include <private/private.h>
#include <private/debug.h>

#ifdef HAVE_MACH_MACH_INIT_H
#include <mach/mach_init.h>
#endif
#ifdef HAVE_MACH_MACH_HOST_H
#include <mach/mach_host.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif

#ifdef HWLOC_WIN_SYS
#include <windows.h>
#endif

unsigned hwloc_get_api_version(void)
{
  return HWLOC_API_VERSION;
}

int hwloc_hide_errors(void)
{
  static int hide = 0;
  static int checked = 0;
  if (!checked) {
    const char *envvar = getenv("HWLOC_HIDE_ERRORS");
    if (envvar)
      hide = atoi(envvar);
    checked = 1;
  }
  return hide;
}

void hwloc_report_os_error(const char *msg, int line)
{
    static int reported = 0;

    if (!reported && !hwloc_hide_errors()) {
        fprintf(stderr, "****************************************************************************\n");
        fprintf(stderr, "* hwloc has encountered what looks like an error from the operating system.\n");
        fprintf(stderr, "*\n");
        fprintf(stderr, "* %s\n", msg);
        fprintf(stderr, "* Error occurred in topology.c line %d\n", line);
        fprintf(stderr, "*\n");
        fprintf(stderr, "* Please report this error message to the hwloc user's mailing list,\n");
#ifdef HWLOC_LINUX_SYS
        fprintf(stderr, "* along with the output from the hwloc-gather-topology.sh script.\n");
#else
	fprintf(stderr, "* along with any relevant topology information from your platform.\n");
#endif
        fprintf(stderr, "****************************************************************************\n");
        reported = 1;
    }
}

#if defined(HAVE_SYSCTLBYNAME)
int hwloc_get_sysctlbyname(const char *name, int64_t *ret)
{
  union {
    int32_t i32;
    int64_t i64;
  } n;
  size_t size = sizeof(n);
  if (sysctlbyname(name, &n, &size, NULL, 0))
    return -1;
  switch (size) {
    case sizeof(n.i32):
      *ret = n.i32;
      break;
    case sizeof(n.i64):
      *ret = n.i64;
      break;
    default:
      return -1;
  }
  return 0;
}
#endif

#if defined(HAVE_SYSCTL)
int hwloc_get_sysctl(int name[], unsigned namelen, int *ret)
{
  int n;
  size_t size = sizeof(n);
  if (sysctl(name, namelen, &n, &size, NULL, 0))
    return -1;
  if (size != sizeof(n))
    return -1;
  *ret = n;
  return 0;
}
#endif

/* Return the OS-provided number of processors.  Unlike other methods such as
   reading sysfs on Linux, this method is not virtualizable; thus it's only
   used as a fall-back method, allowing `hwloc_set_fsroot ()' to
   have the desired effect.  */
unsigned
hwloc_fallback_nbprocessors(struct hwloc_topology *topology) {
  int n;
#if HAVE_DECL__SC_NPROCESSORS_ONLN
  n = sysconf(_SC_NPROCESSORS_ONLN);
#elif HAVE_DECL__SC_NPROC_ONLN
  n = sysconf(_SC_NPROC_ONLN);
#elif HAVE_DECL__SC_NPROCESSORS_CONF
  n = sysconf(_SC_NPROCESSORS_CONF);
#elif HAVE_DECL__SC_NPROC_CONF
  n = sysconf(_SC_NPROC_CONF);
#elif defined(HAVE_HOST_INFO) && HAVE_HOST_INFO
  struct host_basic_info info;
  mach_msg_type_number_t count = HOST_BASIC_INFO_COUNT;
  host_info(mach_host_self(), HOST_BASIC_INFO, (integer_t*) &info, &count);
  n = info.avail_cpus;
#elif defined(HAVE_SYSCTLBYNAME)
  int64_t nn;
  if (hwloc_get_sysctlbyname("hw.ncpu", &nn))
    nn = -1;
  n = nn;
#elif defined(HAVE_SYSCTL) && HAVE_DECL_CTL_HW && HAVE_DECL_HW_NCPU
  static int name[2] = {CTL_HW, HW_NPCU};
  if (hwloc_get_sysctl(name, sizeof(name)/sizeof(*name)), &n)
    n = -1;
#elif defined(HWLOC_WIN_SYS)
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  n = sysinfo.dwNumberOfProcessors;
#else
#ifdef __GNUC__
#warning No known way to discover number of available processors on this system
#warning hwloc_fallback_nbprocessors will default to 1
#endif
  n = -1;
#endif
  if (n >= 1)
    topology->support.discovery->pu = 1;
  else
    n = 1;
  return n;
}

/*
 * Use the given number of processors and the optional online cpuset if given
 * to set a PU level.
 */
void
hwloc_setup_pu_level(struct hwloc_topology *topology,
		     unsigned nb_pus)
{
  struct hwloc_obj *obj;
  unsigned oscpu,cpu;

  hwloc_debug("%s", "\n\n * CPU cpusets *\n\n");
  for (cpu=0,oscpu=0; cpu<nb_pus; oscpu++)
    {
      obj = hwloc_alloc_setup_object(HWLOC_OBJ_PU, oscpu);
      obj->cpuset = hwloc_bitmap_alloc();
      hwloc_bitmap_only(obj->cpuset, oscpu);

      hwloc_debug_2args_bitmap("cpu %u (os %u) has cpuset %s\n",
		 cpu, oscpu, obj->cpuset);
      hwloc_insert_object_by_cpuset(topology, obj);

      cpu++;
    }
}

static void
print_object(struct hwloc_topology *topology, int indent __hwloc_attribute_unused, hwloc_obj_t obj)
{
  char line[256], *cpuset = NULL;
  hwloc_debug("%*s", 2*indent, "");
  hwloc_obj_snprintf(line, sizeof(line), topology, obj, "#", 1);
  hwloc_debug("%s", line);
  if (obj->name)
    hwloc_debug(" name %s", obj->name);
  if (obj->cpuset) {
    hwloc_bitmap_asprintf(&cpuset, obj->cpuset);
    hwloc_debug(" cpuset %s", cpuset);
    free(cpuset);
  }
  if (obj->complete_cpuset) {
    hwloc_bitmap_asprintf(&cpuset, obj->complete_cpuset);
    hwloc_debug(" complete %s", cpuset);
    free(cpuset);
  }
  if (obj->online_cpuset) {
    hwloc_bitmap_asprintf(&cpuset, obj->online_cpuset);
    hwloc_debug(" online %s", cpuset);
    free(cpuset);
  }
  if (obj->allowed_cpuset) {
    hwloc_bitmap_asprintf(&cpuset, obj->allowed_cpuset);
    hwloc_debug(" allowed %s", cpuset);
    free(cpuset);
  }
  if (obj->nodeset) {
    hwloc_bitmap_asprintf(&cpuset, obj->nodeset);
    hwloc_debug(" nodeset %s", cpuset);
    free(cpuset);
  }
  if (obj->complete_nodeset) {
    hwloc_bitmap_asprintf(&cpuset, obj->complete_nodeset);
    hwloc_debug(" completeN %s", cpuset);
    free(cpuset);
  }
  if (obj->allowed_nodeset) {
    hwloc_bitmap_asprintf(&cpuset, obj->allowed_nodeset);
    hwloc_debug(" allowedN %s", cpuset);
    free(cpuset);
  }
  if (obj->arity)
    hwloc_debug(" arity %u", obj->arity);
  hwloc_debug("%s", "\n");
}

/* Just for debugging.  */
static void
print_objects(struct hwloc_topology *topology __hwloc_attribute_unused, int indent __hwloc_attribute_unused, hwloc_obj_t obj __hwloc_attribute_unused)
{
#ifdef HWLOC_DEBUG
  print_object(topology, indent, obj);
  for (obj = obj->first_child; obj; obj = obj->next_sibling)
    print_objects(topology, indent + 1, obj);
#endif
}

void hwloc__free_infos(struct hwloc_obj_info_s *infos, unsigned count)
{
  unsigned i;
  for(i=0; i<count; i++) {
    free(infos[i].name);
    free(infos[i].value);
  }
  free(infos);
}

void hwloc__add_info(struct hwloc_obj_info_s **infosp, unsigned *countp, const char *name, const char *value)
{
  unsigned count = *countp;
  struct hwloc_obj_info_s *infos = *infosp;
#define OBJECT_INFO_ALLOC 8
  /* nothing allocated initially, (re-)allocate by multiple of 8 */
  unsigned alloccount = (count + 1 + (OBJECT_INFO_ALLOC-1)) & ~(OBJECT_INFO_ALLOC-1);
  if (count != alloccount)
    infos = realloc(infos, alloccount*sizeof(*infos));
  infos[count].name = strdup(name);
  infos[count].value = strdup(value);
  *infosp = infos;
  *countp = count+1;
}

void hwloc__move_infos(struct hwloc_obj_info_s **dst_infosp, unsigned *dst_countp,
		       struct hwloc_obj_info_s **src_infosp, unsigned *src_countp)
{
  unsigned dst_count = *dst_countp;
  struct hwloc_obj_info_s *dst_infos = *dst_infosp;
  unsigned src_count = *src_countp;
  struct hwloc_obj_info_s *src_infos = *src_infosp;
  unsigned i;
#define OBJECT_INFO_ALLOC 8
  /* nothing allocated initially, (re-)allocate by multiple of 8 */
  unsigned alloccount = (dst_count + src_count + (OBJECT_INFO_ALLOC-1)) & ~(OBJECT_INFO_ALLOC-1);
  if (dst_count != alloccount)
    dst_infos = realloc(dst_infos, alloccount*sizeof(*dst_infos));
  for(i=0; i<src_count; i++, dst_count++) {
    dst_infos[dst_count].name = src_infos[i].name;
    dst_infos[dst_count].value = src_infos[i].value;
  }
  *dst_infosp = dst_infos;
  *dst_countp = dst_count;
  free(src_infos);
  *src_infosp = NULL;
  *src_countp = 0;
}

void hwloc_obj_add_info(hwloc_obj_t obj, const char *name, const char *value)
{
  hwloc__add_info(&obj->infos, &obj->infos_count, name, value);
}

void hwloc_obj_add_info_nodup(hwloc_obj_t obj, const char *name, const char *value, int nodup)
{
  if (nodup && hwloc_obj_get_info_by_name(obj, name))
    return;
  hwloc__add_info(&obj->infos, &obj->infos_count, name, value);
}

/* Free an object and all its content.  */
void
hwloc_free_unlinked_object(hwloc_obj_t obj)
{
  switch (obj->type) {
  default:
    break;
  }
  hwloc__free_infos(obj->infos, obj->infos_count);
  hwloc_clear_object_distances(obj);
  free(obj->memory.page_types);
  free(obj->attr);
  free(obj->children);
  free(obj->name);
  hwloc_bitmap_free(obj->cpuset);
  hwloc_bitmap_free(obj->complete_cpuset);
  hwloc_bitmap_free(obj->online_cpuset);
  hwloc_bitmap_free(obj->allowed_cpuset);
  hwloc_bitmap_free(obj->nodeset);
  hwloc_bitmap_free(obj->complete_nodeset);
  hwloc_bitmap_free(obj->allowed_nodeset);
  free(obj);
}

static void
hwloc__duplicate_object(struct hwloc_obj *newobj,
			struct hwloc_obj *src)
{
  size_t len;
  unsigned i;

  newobj->type = src->type;
  newobj->os_index = src->os_index;

  if (src->name)
    newobj->name = strdup(src->name);
  newobj->userdata = src->userdata;

  memcpy(&newobj->memory, &src->memory, sizeof(struct hwloc_obj_memory_s));
  if (src->memory.page_types_len) {
    len = src->memory.page_types_len * sizeof(struct hwloc_obj_memory_page_type_s);
    newobj->memory.page_types = malloc(len);
    memcpy(newobj->memory.page_types, src->memory.page_types, len);
  }

  memcpy(newobj->attr, src->attr, sizeof(*newobj->attr));

  newobj->cpuset = hwloc_bitmap_dup(src->cpuset);
  newobj->complete_cpuset = hwloc_bitmap_dup(src->complete_cpuset);
  newobj->allowed_cpuset = hwloc_bitmap_dup(src->allowed_cpuset);
  newobj->online_cpuset = hwloc_bitmap_dup(src->online_cpuset);
  newobj->nodeset = hwloc_bitmap_dup(src->nodeset);
  newobj->complete_nodeset = hwloc_bitmap_dup(src->complete_nodeset);
  newobj->allowed_nodeset = hwloc_bitmap_dup(src->allowed_nodeset);

  /* don't duplicate distances, they'll be recreated at the end of the topology build */

  for(i=0; i<src->infos_count; i++)
    hwloc__add_info(&newobj->infos, &newobj->infos_count, src->infos[i].name, src->infos[i].value);
}

void
hwloc__duplicate_objects(struct hwloc_topology *newtopology,
			 struct hwloc_obj *newparent,
			 struct hwloc_obj *src)
{
  hwloc_obj_t newobj;
  hwloc_obj_t child;

  newobj = hwloc_alloc_setup_object(src->type, src->os_index);
  hwloc__duplicate_object(newobj, src);

  child = NULL;
  while ((child = hwloc_get_next_child(newtopology, src, child)) != NULL)
    hwloc__duplicate_objects(newtopology, newobj, child);

  hwloc_insert_object_by_parent(newtopology, newparent, newobj);
}

int
hwloc_topology_dup(hwloc_topology_t *newp,
		   hwloc_topology_t old)
{
  hwloc_topology_t new;
  hwloc_obj_t newroot;
  hwloc_obj_t oldroot = hwloc_get_root_obj(old);
  unsigned i;

  if (!old->is_loaded) {
    errno = -EINVAL;
    return -1;
  }

  hwloc_topology_init(&new);

  new->flags = old->flags;
  memcpy(new->ignored_types, old->ignored_types, sizeof(old->ignored_types));
  new->is_thissystem = old->is_thissystem;
  new->is_loaded = 1;
  new->pid = old->pid;

  memcpy(&new->binding_hooks, &old->binding_hooks, sizeof(old->binding_hooks));

  memcpy(new->support.discovery, old->support.discovery, sizeof(*old->support.discovery));
  memcpy(new->support.cpubind, old->support.cpubind, sizeof(*old->support.cpubind));
  memcpy(new->support.membind, old->support.membind, sizeof(*old->support.membind));

  new->userdata_export_cb = old->userdata_export_cb;
  new->userdata_import_cb = old->userdata_import_cb;

  newroot = hwloc_get_root_obj(new);
  hwloc__duplicate_object(newroot, oldroot);
  for(i=0; i<oldroot->arity; i++)
    hwloc__duplicate_objects(new, newroot, oldroot->children[i]);

  if (old->first_osdist) {
    struct hwloc_os_distances_s *olddist = old->first_osdist;
    while (olddist) {
      struct hwloc_os_distances_s *newdist = malloc(sizeof(*newdist));
      newdist->type = olddist->type;
      newdist->nbobjs = olddist->nbobjs;
      newdist->indexes = malloc(newdist->nbobjs * sizeof(*newdist->indexes));
      memcpy(newdist->indexes, olddist->indexes, newdist->nbobjs * sizeof(*newdist->indexes));
      newdist->objs = NULL; /* will be recomputed when needed */
      newdist->distances = malloc(newdist->nbobjs * newdist->nbobjs * sizeof(*newdist->distances));
      memcpy(newdist->distances, olddist->distances, newdist->nbobjs * newdist->nbobjs * sizeof(*newdist->distances));

      newdist->forced = olddist->forced;
      if (new->first_osdist) {
	new->last_osdist->next = newdist;
	newdist->prev = new->last_osdist;
      } else {
	new->first_osdist = newdist;
	newdist->prev = NULL;
      }
      new->last_osdist = newdist;
      newdist->next = NULL;

      olddist = olddist->next;
    }
  } else
    new->first_osdist = old->last_osdist = NULL;

  /* no need to duplicate backends, topology is already loaded */
  new->backends = NULL;

  hwloc_connect_children(new->levels[0][0]);
  if (hwloc_connect_levels(new) < 0)
    goto out;

  hwloc_distances_finalize_os(new);
  hwloc_distances_finalize_logical(new);

#ifndef HWLOC_DEBUG
  if (getenv("HWLOC_DEBUG_CHECK"))
#endif
    hwloc_topology_check(new);

  *newp = new;
  return 0;

 out:
  hwloc_topology_clear(new);
  hwloc_distances_destroy(new);
  hwloc_topology_setup_defaults(new);
  return -1;
}

/*
 * How to compare objects based on types.
 *
 * Note that HIGHER/LOWER is only a (consistent) heuristic, used to sort
 * objects with same cpuset consistently.
 * Only EQUAL / not EQUAL can be relied upon.
 */

enum hwloc_type_cmp_e {
  HWLOC_TYPE_HIGHER,
  HWLOC_TYPE_DEEPER,
  HWLOC_TYPE_EQUAL
};

/* WARNING: The indexes of this array MUST match the ordering that of
   the obj_order_type[] array, below.  Specifically, the values must
   be laid out such that:

       obj_order_type[obj_type_order[N]] = N

   for all HWLOC_OBJ_* values of N.  Put differently:

       obj_type_order[A] = B

   where the A values are in order of the hwloc_obj_type_t enum, and
   the B values are the corresponding indexes of obj_order_type.

   We can't use C99 syntax to initialize this in a little safer manner
   -- bummer.  :-(

   *************************************************************
   *** DO NOT CHANGE THE ORDERING OF THIS ARRAY WITHOUT TRIPLE
   *** CHECKING ITS CORRECTNESS!
   *************************************************************
   */
static const unsigned obj_type_order[] = {
    /* first entry is HWLOC_OBJ_SYSTEM */  0,
    /* next entry is HWLOC_OBJ_MACHINE */  1,
    /* next entry is HWLOC_OBJ_NODE */     3,
    /* next entry is HWLOC_OBJ_SOCKET */   4,
    /* next entry is HWLOC_OBJ_CACHE */    5,
    /* next entry is HWLOC_OBJ_CORE */     6,
    /* next entry is HWLOC_OBJ_PU */       10,
    /* next entry is HWLOC_OBJ_GROUP */    2,
    /* next entry is HWLOC_OBJ_MISC */     11,
    /* next entry is HWLOC_OBJ_BRIDGE */   7,
    /* next entry is HWLOC_OBJ_PCI_DEVICE */  8,
    /* next entry is HWLOC_OBJ_OS_DEVICE */   9
};

static const hwloc_obj_type_t obj_order_type[] = {
  HWLOC_OBJ_SYSTEM,
  HWLOC_OBJ_MACHINE,
  HWLOC_OBJ_GROUP,
  HWLOC_OBJ_NODE,
  HWLOC_OBJ_SOCKET,
  HWLOC_OBJ_CACHE,
  HWLOC_OBJ_CORE,
  HWLOC_OBJ_BRIDGE,
  HWLOC_OBJ_PCI_DEVICE,
  HWLOC_OBJ_OS_DEVICE,
  HWLOC_OBJ_PU,
  HWLOC_OBJ_MISC,
};

/* priority to be used when merging identical parent/children object
 * (in merge_useless_child), keep the highest priority one.
 *
 * Always keep Machine/PU/PCIDev/OSDev
 * then System/Node
 * then Core
 * then Socket
 * then Cache
 * then always drop Group/Misc/Bridge.
 *
 * Some type won't actually ever be involved in such merging.
 */
static const int obj_type_priority[] = {
  /* first entry is HWLOC_OBJ_SYSTEM */     80,
  /* next entry is HWLOC_OBJ_MACHINE */     100,
  /* next entry is HWLOC_OBJ_NODE */        80,
  /* next entry is HWLOC_OBJ_SOCKET */      40,
  /* next entry is HWLOC_OBJ_CACHE */       20,
  /* next entry is HWLOC_OBJ_CORE */        60,
  /* next entry is HWLOC_OBJ_PU */          100,
  /* next entry is HWLOC_OBJ_GROUP */       0,
  /* next entry is HWLOC_OBJ_MISC */        0,
  /* next entry is HWLOC_OBJ_BRIDGE */      0,
  /* next entry is HWLOC_OBJ_PCI_DEVICE */  100,
  /* next entry is HWLOC_OBJ_OS_DEVICE */   100
};

static unsigned __hwloc_attribute_const
hwloc_get_type_order(hwloc_obj_type_t type)
{
  return obj_type_order[type];
}

#if !defined(NDEBUG)
static hwloc_obj_type_t hwloc_get_order_type(int order)
{
  return obj_order_type[order];
}
#endif

static int hwloc_obj_type_is_io (hwloc_obj_type_t type)
{
  return type == HWLOC_OBJ_BRIDGE || type == HWLOC_OBJ_PCI_DEVICE || type == HWLOC_OBJ_OS_DEVICE;
}

int hwloc_compare_types (hwloc_obj_type_t type1, hwloc_obj_type_t type2)
{
  unsigned order1 = hwloc_get_type_order(type1);
  unsigned order2 = hwloc_get_type_order(type2);

  /* bridge and devices are only comparable with each others and with machine and system */
  if (hwloc_obj_type_is_io(type1)
      && !hwloc_obj_type_is_io(type2) && type2 != HWLOC_OBJ_SYSTEM && type2 != HWLOC_OBJ_MACHINE)
    return HWLOC_TYPE_UNORDERED;
  if (hwloc_obj_type_is_io(type2)
      && !hwloc_obj_type_is_io(type1) && type1 != HWLOC_OBJ_SYSTEM && type1 != HWLOC_OBJ_MACHINE)
    return HWLOC_TYPE_UNORDERED;

  return order1 - order2;
}

static enum hwloc_type_cmp_e
hwloc_type_cmp(hwloc_obj_t obj1, hwloc_obj_t obj2)
{
  hwloc_obj_type_t type1 = obj1->type;
  hwloc_obj_type_t type2 = obj2->type;
  int compare;

  compare = hwloc_compare_types(type1, type2);
  if (compare == HWLOC_TYPE_UNORDERED)
    return HWLOC_TYPE_EQUAL; /* we cannot do better */
  if (compare > 0)
    return HWLOC_TYPE_DEEPER;
  if (compare < 0)
    return HWLOC_TYPE_HIGHER;

  /* Caches have the same types but can have different depths.  */
  if (type1 == HWLOC_OBJ_CACHE) {
    if (obj1->attr->cache.depth < obj2->attr->cache.depth)
      return HWLOC_TYPE_DEEPER;
    else if (obj1->attr->cache.depth > obj2->attr->cache.depth)
      return HWLOC_TYPE_HIGHER;
    else if (obj1->attr->cache.type > obj2->attr->cache.type)
      /* consider icache deeper than dcache and dcache deeper than unified */
      return HWLOC_TYPE_DEEPER;
    else if (obj1->attr->cache.type < obj2->attr->cache.type)
      /* consider icache deeper than dcache and dcache deeper than unified */
      return HWLOC_TYPE_HIGHER;
  }

  /* Group objects have the same types but can have different depths.  */
  if (type1 == HWLOC_OBJ_GROUP) {
    if (obj1->attr->group.depth == (unsigned) -1
	|| obj2->attr->group.depth == (unsigned) -1)
      return HWLOC_TYPE_EQUAL;
    if (obj1->attr->group.depth < obj2->attr->group.depth)
      return HWLOC_TYPE_DEEPER;
    else if (obj1->attr->group.depth > obj2->attr->group.depth)
      return HWLOC_TYPE_HIGHER;
  }

  /* Bridges objects have the same types but can have different depths.  */
  if (type1 == HWLOC_OBJ_BRIDGE) {
    if (obj1->attr->bridge.depth < obj2->attr->bridge.depth)
      return HWLOC_TYPE_DEEPER;
    else if (obj1->attr->bridge.depth > obj2->attr->bridge.depth)
      return HWLOC_TYPE_HIGHER;
  }

  return HWLOC_TYPE_EQUAL;
}

/*
 * How to compare objects based on cpusets.
 */

enum hwloc_obj_cmp_e {
  HWLOC_OBJ_EQUAL,	/**< \brief Equal */
  HWLOC_OBJ_INCLUDED,	/**< \brief Strictly included into */
  HWLOC_OBJ_CONTAINS,	/**< \brief Strictly contains */
  HWLOC_OBJ_INTERSECTS,	/**< \brief Intersects, but no inclusion! */
  HWLOC_OBJ_DIFFERENT	/**< \brief No intersection */
};

static int
hwloc_obj_cmp(hwloc_obj_t obj1, hwloc_obj_t obj2)
{
  hwloc_bitmap_t set1, set2;

  /* compare cpusets if possible, or fallback to nodeset, or return */
  if (obj1->cpuset && !hwloc_bitmap_iszero(obj1->cpuset)
      && obj2->cpuset && !hwloc_bitmap_iszero(obj2->cpuset)) {
    set1 = obj1->cpuset;
    set2 = obj2->cpuset;
  } else if (obj1->nodeset && !hwloc_bitmap_iszero(obj1->nodeset)
	     && obj2->nodeset && !hwloc_bitmap_iszero(obj2->nodeset)) {
    set1 = obj1->nodeset;
    set2 = obj2->nodeset;
  } else {
    return HWLOC_OBJ_DIFFERENT;
  }

  if (hwloc_bitmap_isequal(set1, set2)) {

    /* Same sets, subsort by type to have a consistent ordering.  */

    switch (hwloc_type_cmp(obj1, obj2)) {
      case HWLOC_TYPE_DEEPER:
	return HWLOC_OBJ_INCLUDED;
      case HWLOC_TYPE_HIGHER:
	return HWLOC_OBJ_CONTAINS;

      case HWLOC_TYPE_EQUAL:
        if (obj1->type == HWLOC_OBJ_MISC) {
          /* Misc objects may vary by name */
          int res = strcmp(obj1->name, obj2->name);
          if (res < 0)
            return HWLOC_OBJ_INCLUDED;
          if (res > 0)
            return HWLOC_OBJ_CONTAINS;
          if (res == 0)
            return HWLOC_OBJ_EQUAL;
        }

	/* Same sets and types!  Let's hope it's coherent.  */
	return HWLOC_OBJ_EQUAL;
    }

    /* For dumb compilers */
    abort();

  } else {

    /* Different sets, sort by inclusion.  */

    if (hwloc_bitmap_isincluded(set1, set2))
      return HWLOC_OBJ_INCLUDED;

    if (hwloc_bitmap_isincluded(set2, set1))
      return HWLOC_OBJ_CONTAINS;

    if (hwloc_bitmap_intersects(set1, set2))
      return HWLOC_OBJ_INTERSECTS;

    return HWLOC_OBJ_DIFFERENT;
  }
}

/* format must contain a single %s where to print obj infos */
static void
hwloc___insert_object_by_cpuset_report_error(hwloc_report_error_t report_error, const char *fmt, hwloc_obj_t obj, int line)
{
	char typestr[64];
	char objstr[512];
	char msg[640];
	char *cpusetstr;

	hwloc_obj_type_snprintf(typestr, sizeof(typestr), obj, 0);
	hwloc_bitmap_asprintf(&cpusetstr, obj->cpuset);
	if (obj->os_index != (unsigned) -1)
	  snprintf(objstr, sizeof(objstr), "%s P#%u cpuset %s",
		   typestr, obj->os_index, cpusetstr);
	else
	  snprintf(objstr, sizeof(objstr), "%s cpuset %s",
		   typestr, cpusetstr);
	free(cpusetstr);

	snprintf(msg, sizeof(msg), fmt,
		 objstr);
	report_error(msg, line);
}

/*
 * How to insert objects into the topology.
 *
 * Note: during detection, only the first_child and next_sibling pointers are
 * kept up to date.  Others are computed only once topology detection is
 * complete.
 */

#define merge_index(new, old, field, type) \
  if ((old)->field == (type) -1) \
    (old)->field = (new)->field;
#define merge_sizes(new, old, field) \
  if (!(old)->field) \
    (old)->field = (new)->field;
#ifdef HWLOC_DEBUG
#define check_sizes(new, old, field) \
  if ((new)->field) \
    assert((old)->field == (new)->field)
#else
#define check_sizes(new, old, field)
#endif

/* Try to insert OBJ in CUR, recurse if needed.
 * Returns the object if it was inserted,
 * the remaining object it was merged,
 * NULL if failed to insert.
 */
static struct hwloc_obj *
hwloc___insert_object_by_cpuset(struct hwloc_topology *topology, hwloc_obj_t cur, hwloc_obj_t obj,
			        hwloc_report_error_t report_error)
{
  hwloc_obj_t child, container, *cur_children, *obj_children, next_child = NULL;
  int put;

  /* Make sure we haven't gone too deep.  */
  if (!hwloc_bitmap_isincluded(obj->cpuset, cur->cpuset)) {
    fprintf(stderr,"recursion has gone too deep?!\n");
    return NULL;
  }

  /* Check whether OBJ is included in some child.  */
  container = NULL;
  for (child = cur->first_child; child; child = child->next_sibling) {
    switch (hwloc_obj_cmp(obj, child)) {
      case HWLOC_OBJ_EQUAL:
        merge_index(obj, child, os_level, signed);
	if (obj->os_level != child->os_level) {
	  static int reported = 0;
	  if (!reported && !hwloc_hide_errors()) {
	    fprintf(stderr, "Cannot merge similar %s objects with different OS levels %u and %u\n",
		    hwloc_obj_type_string(obj->type), child->os_level, obj->os_level);
	    reported = 1;
	  }
          return NULL;
        }
        merge_index(obj, child, os_index, unsigned);
	if (obj->os_index != child->os_index) {
	  static int reported = 0;
	  if (!reported && !hwloc_hide_errors()) {
	    fprintf(stderr, "Cannot merge similar %s objects with different OS indexes %u and %u\n",
		    hwloc_obj_type_string(obj->type), child->os_index, obj->os_index);
	    reported = 1;
	  }
          return NULL;
        }
	if (obj->distances_count) {
	  if (child->distances_count) {
	    child->distances_count += obj->distances_count;
	    child->distances = realloc(child->distances, child->distances_count * sizeof(*child->distances));
	    memcpy(child->distances + obj->distances_count, obj->distances, obj->distances_count * sizeof(*child->distances));
	    free(obj->distances);
	  } else {
	    child->distances_count = obj->distances_count;
	    child->distances = obj->distances;
	  }
	  obj->distances_count = 0;
	  obj->distances = NULL;
	}
	if (obj->infos_count) {
	  if (child->infos_count) {
	    child->infos_count += obj->infos_count;
	    child->infos = realloc(child->infos, child->infos_count * sizeof(*child->infos));
	    memcpy(child->infos + obj->infos_count, obj->infos, obj->infos_count * sizeof(*child->infos));
	    free(obj->infos);
	  } else {
	    child->infos_count = obj->infos_count;
	    child->infos = obj->infos;
	  }
	  obj->infos_count = 0;
	  obj->infos = NULL;
	}
	if (obj->name) {
	  if (child->name)
	    free(child->name);
	  child->name = obj->name;
	  obj->name = NULL;
	}
	assert(!obj->userdata); /* user could not set userdata here (we're before load() */
	switch(obj->type) {
	  case HWLOC_OBJ_NODE:
	    /* Do not check these, it may change between calls */
	    merge_sizes(obj, child, memory.local_memory);
	    merge_sizes(obj, child, memory.total_memory);
	    /* if both objects have a page_types array, just keep the biggest one for now */
	    if (obj->memory.page_types_len && child->memory.page_types_len)
	      hwloc_debug("%s", "merging page_types by keeping the biggest one only\n");
	    if (obj->memory.page_types_len < child->memory.page_types_len) {
	      free(obj->memory.page_types);
	    } else {
	      free(child->memory.page_types);
	      child->memory.page_types_len = obj->memory.page_types_len;
	      child->memory.page_types = obj->memory.page_types;
	      obj->memory.page_types = NULL;
	      obj->memory.page_types_len = 0;
	    }
	    break;
	  case HWLOC_OBJ_CACHE:
	    merge_sizes(obj, child, attr->cache.size);
	    check_sizes(obj, child, attr->cache.size);
	    merge_sizes(obj, child, attr->cache.linesize);
	    check_sizes(obj, child, attr->cache.linesize);
	    break;
	  default:
	    break;
	}
	/* Already present, no need to insert.  */
	return child;
      case HWLOC_OBJ_INCLUDED:
	if (container) {
          if (report_error)
	    hwloc___insert_object_by_cpuset_report_error(report_error, "object (%s) included in several different objects!", obj, __LINE__);
	  /* We can't handle that.  */
	  return NULL;
	}
	/* This child contains OBJ.  */
	container = child;
	break;
      case HWLOC_OBJ_INTERSECTS:
        if (report_error)
          hwloc___insert_object_by_cpuset_report_error(report_error, "object (%s) intersection without inclusion!", obj, __LINE__);
	/* We can't handle that.  */
	return NULL;
      case HWLOC_OBJ_CONTAINS:
	/* OBJ will be above CHILD.  */
	break;
      case HWLOC_OBJ_DIFFERENT:
	/* OBJ will be alongside CHILD.  */
	break;
    }
  }

  if (container) {
    /* OBJ is strictly contained is some child of CUR, go deeper.  */
    return hwloc___insert_object_by_cpuset(topology, container, obj, report_error);
  }

  /*
   * Children of CUR are either completely different from or contained into
   * OBJ. Take those that are contained (keeping sorting order), and sort OBJ
   * along those that are different.
   */

  /* OBJ is not put yet.  */
  put = 0;

  /* These will always point to the pointer to their next last child. */
  cur_children = &cur->first_child;
  obj_children = &obj->first_child;

  /* Construct CUR's and OBJ's children list.  */

  /* Iteration with prefetching to be completely safe against CHILD removal.  */
  for (child = cur->first_child, child ? next_child = child->next_sibling : NULL;
       child;
       child = next_child, child ? next_child = child->next_sibling : NULL) {

    switch (hwloc_obj_cmp(obj, child)) {

      case HWLOC_OBJ_DIFFERENT:
	/* Leave CHILD in CUR.  */
	if (!put && (!child->cpuset || hwloc_bitmap_compare_first(obj->cpuset, child->cpuset) < 0)) {
	  /* Sort children by cpuset: put OBJ before CHILD in CUR's children.  */
	  *cur_children = obj;
	  cur_children = &obj->next_sibling;
	  put = 1;
	}
	/* Now put CHILD in CUR's children.  */
	*cur_children = child;
	cur_children = &child->next_sibling;
	break;

      case HWLOC_OBJ_CONTAINS:
	/* OBJ contains CHILD, put the latter in the former.  */
	*obj_children = child;
	obj_children = &child->next_sibling;
	break;

      case HWLOC_OBJ_EQUAL:
      case HWLOC_OBJ_INCLUDED:
      case HWLOC_OBJ_INTERSECTS:
	/* Shouldn't ever happen as we have handled them above.  */
	abort();
    }
  }

  /* Put OBJ last in CUR's children if not already done so.  */
  if (!put) {
    *cur_children = obj;
    cur_children = &obj->next_sibling;
  }

  /* Close children lists.  */
  *obj_children = NULL;
  *cur_children = NULL;

  return obj;
}

/* insertion routine that lets you change the error reporting callback */
struct hwloc_obj *
hwloc__insert_object_by_cpuset(struct hwloc_topology *topology, hwloc_obj_t obj,
			       hwloc_report_error_t report_error)
{
  struct hwloc_obj *result;
  /* Start at the top.  */
  /* Add the cpuset to the top */
  hwloc_bitmap_or(topology->levels[0][0]->complete_cpuset, topology->levels[0][0]->complete_cpuset, obj->cpuset);
  if (obj->nodeset)
    hwloc_bitmap_or(topology->levels[0][0]->complete_nodeset, topology->levels[0][0]->complete_nodeset, obj->nodeset);
  result = hwloc___insert_object_by_cpuset(topology, topology->levels[0][0], obj, report_error);
  if (result != obj)
    /* either failed to insert, or got merged, free the original object */
    hwloc_free_unlinked_object(obj);
  return result;
}

/* the default insertion routine warns in case of error.
 * it's used by most backends */
struct hwloc_obj *
hwloc_insert_object_by_cpuset(struct hwloc_topology *topology, hwloc_obj_t obj)
{
  return hwloc__insert_object_by_cpuset(topology, obj, hwloc_report_os_error);
}

void
hwloc_insert_object_by_parent(struct hwloc_topology *topology, hwloc_obj_t parent, hwloc_obj_t obj)
{
  hwloc_obj_t child, next_child = obj->first_child;
  hwloc_obj_t *current;

  /* Append to the end of the list */
  for (current = &parent->first_child; *current; current = &(*current)->next_sibling) {
    hwloc_bitmap_t curcpuset = (*current)->cpuset;
    if (obj->cpuset && (!curcpuset || hwloc_bitmap_compare_first(obj->cpuset, curcpuset) < 0)) {
      static int reported = 0;
      if (!reported && !hwloc_hide_errors()) {
	char *a = "NULL", *b;
	if (curcpuset)
	  hwloc_bitmap_asprintf(&a, curcpuset);
	hwloc_bitmap_asprintf(&b, obj->cpuset);
        fprintf(stderr, "****************************************************************************\n");
        fprintf(stderr, "* hwloc has encountered an out-of-order topology discovery.\n");
        fprintf(stderr, "* An object with cpuset %s was inserted after object with %s\n", b, a);
        fprintf(stderr, "* Please check that your input topology (XML file, etc.) is valid.\n");
        fprintf(stderr, "****************************************************************************\n");
	if (curcpuset)
	  free(a);
	free(b);
        reported = 1;
      }
    }
  }

  *current = obj;
  obj->next_sibling = NULL;
  obj->first_child = NULL;

  /* Use the new object to insert children */
  parent = obj;

  /* Recursively insert children below */
  while (next_child) {
    child = next_child;
    next_child = child->next_sibling;
    hwloc_insert_object_by_parent(topology, parent, child);
  }

  if (obj->type == HWLOC_OBJ_MISC) {
    /* misc objects go in no level (needed here because level building doesn't see Misc objects inside I/O trees) */
    obj->depth = (unsigned) HWLOC_TYPE_DEPTH_UNKNOWN;
  }
}

/* Adds a misc object _after_ detection, and thus has to reconnect all the pointers */
hwloc_obj_t
hwloc_topology_insert_misc_object_by_cpuset(struct hwloc_topology *topology, hwloc_const_bitmap_t cpuset, const char *name)
{
  hwloc_obj_t obj, child;

  if (!topology->is_loaded) {
    errno = EINVAL;
    return NULL;
  }

  if (hwloc_bitmap_iszero(cpuset))
    return NULL;
  if (!hwloc_bitmap_isincluded(cpuset, hwloc_topology_get_topology_cpuset(topology)))
    return NULL;

  obj = hwloc_alloc_setup_object(HWLOC_OBJ_MISC, -1);
  if (name)
    obj->name = strdup(name);

  /* misc objects go in no level */
  obj->depth = (unsigned) HWLOC_TYPE_DEPTH_UNKNOWN;

  obj->cpuset = hwloc_bitmap_dup(cpuset);
  /* initialize default cpusets, we'll adjust them later */
  obj->complete_cpuset = hwloc_bitmap_dup(cpuset);
  obj->allowed_cpuset = hwloc_bitmap_dup(cpuset);
  obj->online_cpuset = hwloc_bitmap_dup(cpuset);

  obj = hwloc__insert_object_by_cpuset(topology, obj, NULL /* do not show errors on stdout */);
  if (!obj)
    return NULL;

  hwloc_connect_children(topology->levels[0][0]);

  if ((child = obj->first_child) != NULL && child->cpuset) {
    /* keep the main cpuset untouched, but update other cpusets and nodesets from children */
    obj->nodeset = hwloc_bitmap_alloc();
    obj->complete_nodeset = hwloc_bitmap_alloc();
    obj->allowed_nodeset = hwloc_bitmap_alloc();
    while (child) {
      if (child->complete_cpuset)
	hwloc_bitmap_or(obj->complete_cpuset, obj->complete_cpuset, child->complete_cpuset);
      if (child->allowed_cpuset)
	hwloc_bitmap_or(obj->allowed_cpuset, obj->allowed_cpuset, child->allowed_cpuset);
      if (child->online_cpuset)
	hwloc_bitmap_or(obj->online_cpuset, obj->online_cpuset, child->online_cpuset);
      if (child->nodeset)
	hwloc_bitmap_or(obj->nodeset, obj->nodeset, child->nodeset);
      if (child->complete_nodeset)
	hwloc_bitmap_or(obj->complete_nodeset, obj->complete_nodeset, child->complete_nodeset);
      if (child->allowed_nodeset)
	hwloc_bitmap_or(obj->allowed_nodeset, obj->allowed_nodeset, child->allowed_nodeset);
      child = child->next_sibling;
    }
  } else {
    /* copy the parent nodesets */
    obj->nodeset = hwloc_bitmap_dup(obj->parent->nodeset);
    obj->complete_nodeset = hwloc_bitmap_dup(obj->parent->complete_nodeset);
    obj->allowed_nodeset = hwloc_bitmap_dup(obj->parent->allowed_nodeset);
  }

  return obj;
}

hwloc_obj_t
hwloc_topology_insert_misc_object_by_parent(struct hwloc_topology *topology, hwloc_obj_t parent, const char *name)
{
  hwloc_obj_t obj = hwloc_alloc_setup_object(HWLOC_OBJ_MISC, -1);
  if (name)
    obj->name = strdup(name);

  if (!topology->is_loaded) {
    hwloc_free_unlinked_object(obj);
    errno = EINVAL;
    return NULL;
  }

  hwloc_insert_object_by_parent(topology, parent, obj);

  hwloc_connect_children(topology->levels[0][0]);
  /* no need to hwloc_connect_levels() since misc object are not in levels */

  return obj;
}

/* Traverse children of a parent in a safe way: reread the next pointer as
 * appropriate to prevent crash on child deletion:  */
#define for_each_child_safe(child, parent, pchild) \
  for (pchild = &(parent)->first_child, child = *pchild; \
       child; \
       /* Check whether the current child was not dropped.  */ \
       (*pchild == child ? pchild = &(child->next_sibling) : NULL), \
       /* Get pointer to next childect.  */ \
        child = *pchild)

/* Append I/O devices below this object to their list */
static void
append_iodevs(hwloc_topology_t topology, hwloc_obj_t obj)
{
  hwloc_obj_t child, *temp;

  /* make sure we don't have remaining stale pointers from a previous load */
  obj->next_cousin = NULL;
  obj->prev_cousin = NULL;

  if (obj->type == HWLOC_OBJ_BRIDGE) {
    obj->depth = HWLOC_TYPE_DEPTH_BRIDGE;
    /* Insert in the main bridge list */
    if (topology->first_bridge) {
      obj->prev_cousin = topology->last_bridge;
      obj->prev_cousin->next_cousin = obj;
      topology->last_bridge = obj;
    } else {
      topology->first_bridge = topology->last_bridge = obj;
    }
  } else if (obj->type == HWLOC_OBJ_PCI_DEVICE) {
    obj->depth = HWLOC_TYPE_DEPTH_PCI_DEVICE;
    /* Insert in the main pcidev list */
    if (topology->first_pcidev) {
      obj->prev_cousin = topology->last_pcidev;
      obj->prev_cousin->next_cousin = obj;
      topology->last_pcidev = obj;
    } else {
      topology->first_pcidev = topology->last_pcidev = obj;
    }
  } else if (obj->type == HWLOC_OBJ_OS_DEVICE) {
    obj->depth = HWLOC_TYPE_DEPTH_OS_DEVICE;
    /* Insert in the main osdev list */
    if (topology->first_osdev) {
      obj->prev_cousin = topology->last_osdev;
      obj->prev_cousin->next_cousin = obj;
      topology->last_osdev = obj;
    } else {
      topology->first_osdev = topology->last_osdev = obj;
    }
  }

  for_each_child_safe(child, obj, temp)
    append_iodevs(topology, child);
}

static int hwloc_memory_page_type_compare(const void *_a, const void *_b)
{
  const struct hwloc_obj_memory_page_type_s *a = _a;
  const struct hwloc_obj_memory_page_type_s *b = _b;
  /* consider 0 as larger so that 0-size page_type go to the end */
  if (!b->size)
    return -1;
  /* don't cast a-b in int since those are ullongs */
  if (b->size == a->size)
    return 0;
  return a->size < b->size ? -1 : 1;
}

/* Propagate memory counts */
static void
propagate_total_memory(hwloc_obj_t obj)
{
  hwloc_obj_t *temp, child;
  unsigned i;

  /* reset total before counting local and children memory */
  obj->memory.total_memory = 0;

  /* Propagate memory up */
  for_each_child_safe(child, obj, temp) {
    propagate_total_memory(child);
    obj->memory.total_memory += child->memory.total_memory;
  }
  obj->memory.total_memory += obj->memory.local_memory;

  /* By the way, sort the page_type array.
   * Cannot do it on insert since some backends (e.g. XML) add page_types after inserting the object.
   */
  qsort(obj->memory.page_types, obj->memory.page_types_len, sizeof(*obj->memory.page_types), hwloc_memory_page_type_compare);
  /* Ignore 0-size page_types, they are at the end */
  for(i=obj->memory.page_types_len; i>=1; i--)
    if (obj->memory.page_types[i-1].size)
      break;
  obj->memory.page_types_len = i;
}

/* Collect the cpuset of all the PU objects. */
static void
collect_proc_cpuset(hwloc_obj_t obj, hwloc_obj_t sys)
{
  hwloc_obj_t child, *temp;

  if (sys) {
    /* We are already given a pointer to a system object */
    if (obj->type == HWLOC_OBJ_PU)
      hwloc_bitmap_or(sys->cpuset, sys->cpuset, obj->cpuset);
  } else {
    if (obj->cpuset) {
      /* This object is the root of a machine */
      sys = obj;
      /* Assume no PU for now */
      hwloc_bitmap_zero(obj->cpuset);
    }
  }

  for_each_child_safe(child, obj, temp)
    collect_proc_cpuset(child, sys);
}

/* While traversing down and up, propagate the offline/disallowed cpus by
 * and'ing them to and from the first object that has a cpuset */
static void
propagate_unused_cpuset(hwloc_obj_t obj, hwloc_obj_t sys)
{
  hwloc_obj_t child, *temp;

  if (obj->cpuset) {
    if (sys) {
      /* We are already given a pointer to an system object, update it and update ourselves */
      hwloc_bitmap_t mask = hwloc_bitmap_alloc();

      /* Apply the topology cpuset */
      hwloc_bitmap_and(obj->cpuset, obj->cpuset, sys->cpuset);

      /* Update complete cpuset down */
      if (obj->complete_cpuset) {
	hwloc_bitmap_and(obj->complete_cpuset, obj->complete_cpuset, sys->complete_cpuset);
      } else {
	obj->complete_cpuset = hwloc_bitmap_dup(sys->complete_cpuset);
	hwloc_bitmap_and(obj->complete_cpuset, obj->complete_cpuset, obj->cpuset);
      }

      /* Update online cpusets */
      if (obj->online_cpuset) {
	/* Update ours */
	hwloc_bitmap_and(obj->online_cpuset, obj->online_cpuset, sys->online_cpuset);

	/* Update the given cpuset, but only what we know */
	hwloc_bitmap_copy(mask, obj->cpuset);
	hwloc_bitmap_not(mask, mask);
	hwloc_bitmap_or(mask, mask, obj->online_cpuset);
	hwloc_bitmap_and(sys->online_cpuset, sys->online_cpuset, mask);
      } else {
	/* Just take it as such */
	obj->online_cpuset = hwloc_bitmap_dup(sys->online_cpuset);
	hwloc_bitmap_and(obj->online_cpuset, obj->online_cpuset, obj->cpuset);
      }

      /* Update allowed cpusets */
      if (obj->allowed_cpuset) {
	/* Update ours */
	hwloc_bitmap_and(obj->allowed_cpuset, obj->allowed_cpuset, sys->allowed_cpuset);

	/* Update the given cpuset, but only what we know */
	hwloc_bitmap_copy(mask, obj->cpuset);
	hwloc_bitmap_not(mask, mask);
	hwloc_bitmap_or(mask, mask, obj->allowed_cpuset);
	hwloc_bitmap_and(sys->allowed_cpuset, sys->allowed_cpuset, mask);
      } else {
	/* Just take it as such */
	obj->allowed_cpuset = hwloc_bitmap_dup(sys->allowed_cpuset);
	hwloc_bitmap_and(obj->allowed_cpuset, obj->allowed_cpuset, obj->cpuset);
      }

      hwloc_bitmap_free(mask);
    } else {
      /* This object is the root of a machine */
      sys = obj;
      /* Apply complete cpuset to cpuset, online_cpuset and allowed_cpuset, it
       * will automatically be applied below */
      if (obj->complete_cpuset)
        hwloc_bitmap_and(obj->cpuset, obj->cpuset, obj->complete_cpuset);
      else
        obj->complete_cpuset = hwloc_bitmap_dup(obj->cpuset);
      if (obj->online_cpuset)
        hwloc_bitmap_and(obj->online_cpuset, obj->online_cpuset, obj->complete_cpuset);
      else
        obj->online_cpuset = hwloc_bitmap_dup(obj->complete_cpuset);
      if (obj->allowed_cpuset)
        hwloc_bitmap_and(obj->allowed_cpuset, obj->allowed_cpuset, obj->complete_cpuset);
      else
        obj->allowed_cpuset = hwloc_bitmap_dup(obj->complete_cpuset);
    }
  }

  for_each_child_safe(child, obj, temp)
    propagate_unused_cpuset(child, sys);
}

/* Force full nodeset for non-NUMA machines */
static void
add_default_object_sets(hwloc_obj_t obj, int parent_has_sets)
{
  hwloc_obj_t child, *temp;

  /* I/O devices (and their children) have no sets */
  if (hwloc_obj_type_is_io(obj->type))
    return;

  if (parent_has_sets && obj->type != HWLOC_OBJ_MISC) {
    /* non-MISC object must have cpuset if parent has one. */
    assert(obj->cpuset);
  }

  /* other sets must be consistent with main cpuset:
   * check cpusets and add nodesets if needed.
   *
   * MISC may have no sets at all (if added by parent), or usual ones (if added by cpuset),
   * but that's not easy to detect, so just make sure sets are consistent as usual.
   */
  if (obj->cpuset) {
    assert(obj->online_cpuset);
    assert(obj->complete_cpuset);
    assert(obj->allowed_cpuset);
    if (!obj->nodeset)
      obj->nodeset = hwloc_bitmap_alloc_full();
    if (!obj->complete_nodeset)
      obj->complete_nodeset = hwloc_bitmap_alloc_full();
    if (!obj->allowed_nodeset)
      obj->allowed_nodeset = hwloc_bitmap_alloc_full();
  } else {
    assert(!obj->online_cpuset);
    assert(!obj->complete_cpuset);
    assert(!obj->allowed_cpuset);
    assert(!obj->nodeset);
    assert(!obj->complete_nodeset);
    assert(!obj->allowed_nodeset);
  }

  for_each_child_safe(child, obj, temp)
    add_default_object_sets(child, obj->cpuset != NULL);
}

/* Setup object cpusets/nodesets by OR'ing its children. */
HWLOC_DECLSPEC int
hwloc_fill_object_sets(hwloc_obj_t obj)
{
  hwloc_obj_t child;
  assert(obj->cpuset != NULL);
  child = obj->first_child;
  while (child) {
    assert(child->cpuset != NULL);
    if (child->complete_cpuset) {
      if (!obj->complete_cpuset)
	obj->complete_cpuset = hwloc_bitmap_alloc();
      hwloc_bitmap_or(obj->complete_cpuset, obj->complete_cpuset, child->complete_cpuset);
    }
    if (child->online_cpuset) {
      if (!obj->online_cpuset)
	obj->online_cpuset = hwloc_bitmap_alloc();
      hwloc_bitmap_or(obj->online_cpuset, obj->online_cpuset, child->online_cpuset);
    }
    if (child->allowed_cpuset) {
      if (!obj->allowed_cpuset)
	obj->allowed_cpuset = hwloc_bitmap_alloc();
      hwloc_bitmap_or(obj->allowed_cpuset, obj->allowed_cpuset, child->allowed_cpuset);
    }
    if (child->nodeset) {
      if (!obj->nodeset)
	obj->nodeset = hwloc_bitmap_alloc();
      hwloc_bitmap_or(obj->nodeset, obj->nodeset, child->nodeset);
    }
    if (child->complete_nodeset) {
      if (!obj->complete_nodeset)
	obj->complete_nodeset = hwloc_bitmap_alloc();
      hwloc_bitmap_or(obj->complete_nodeset, obj->complete_nodeset, child->complete_nodeset);
    }
    if (child->allowed_nodeset) {
      if (!obj->allowed_nodeset)
	obj->allowed_nodeset = hwloc_bitmap_alloc();
      hwloc_bitmap_or(obj->allowed_nodeset, obj->allowed_nodeset, child->allowed_nodeset);
    }
    child = child->next_sibling;
  }
  return 0;
}

/* Propagate nodesets up and down */
static void
propagate_nodeset(hwloc_obj_t obj, hwloc_obj_t sys)
{
  hwloc_obj_t child, *temp;
  hwloc_bitmap_t parent_nodeset = NULL;
  int parent_weight = 0;

  if (!sys && obj->nodeset) {
    sys = obj;
    if (!obj->complete_nodeset)
      obj->complete_nodeset = hwloc_bitmap_dup(obj->nodeset);
    if (!obj->allowed_nodeset)
      obj->allowed_nodeset = hwloc_bitmap_dup(obj->complete_nodeset);
  }

  if (sys) {
    if (obj->nodeset) {
      /* Some existing nodeset coming from above, to possibly propagate down */
      parent_nodeset = obj->nodeset;
      parent_weight = hwloc_bitmap_weight(parent_nodeset);
    } else
      obj->nodeset = hwloc_bitmap_alloc();
  }

  for_each_child_safe(child, obj, temp) {
    /* don't propagate nodesets in I/O objects, keep them NULL */
    if (hwloc_obj_type_is_io(child->type))
      return;
    /* don't propagate nodesets in Misc inserted by parent (no nodeset if no cpuset) */
    if (child->type == HWLOC_OBJ_MISC && !child->cpuset)
      return;

    /* Propagate singleton nodesets down */
    if (parent_weight == 1) {
      if (!child->nodeset)
        child->nodeset = hwloc_bitmap_dup(obj->nodeset);
      else if (!hwloc_bitmap_isequal(child->nodeset, parent_nodeset)) {
        hwloc_debug_bitmap("Oops, parent nodeset %s", parent_nodeset);
        hwloc_debug_bitmap(" is different from child nodeset %s, ignoring the child one\n", child->nodeset);
        hwloc_bitmap_copy(child->nodeset, parent_nodeset);
      }
    }

    /* Recurse */
    propagate_nodeset(child, sys);

    /* Propagate children nodesets up */
    if (sys && child->nodeset)
      hwloc_bitmap_or(obj->nodeset, obj->nodeset, child->nodeset);
  }
}

/* Propagate allowed and complete nodesets */
static void
propagate_nodesets(hwloc_obj_t obj)
{
  hwloc_bitmap_t mask = hwloc_bitmap_alloc();
  hwloc_obj_t child, *temp;

  for_each_child_safe(child, obj, temp) {
    /* don't propagate nodesets in I/O objects, keep them NULL */
    if (hwloc_obj_type_is_io(child->type))
      continue;

    if (obj->nodeset) {
      /* Update complete nodesets down */
      if (child->complete_nodeset) {
        hwloc_bitmap_and(child->complete_nodeset, child->complete_nodeset, obj->complete_nodeset);
      } else if (child->nodeset) {
        child->complete_nodeset = hwloc_bitmap_dup(obj->complete_nodeset);
        hwloc_bitmap_and(child->complete_nodeset, child->complete_nodeset, child->nodeset);
      } /* else the child doesn't have nodeset information, we can not provide a complete nodeset */

      /* Update allowed nodesets down */
      if (child->allowed_nodeset) {
        hwloc_bitmap_and(child->allowed_nodeset, child->allowed_nodeset, obj->allowed_nodeset);
      } else if (child->nodeset) {
        child->allowed_nodeset = hwloc_bitmap_dup(obj->allowed_nodeset);
        hwloc_bitmap_and(child->allowed_nodeset, child->allowed_nodeset, child->nodeset);
      }
    }

    propagate_nodesets(child);

    if (obj->nodeset) {
      /* Update allowed nodesets up */
      if (child->nodeset && child->allowed_nodeset) {
        hwloc_bitmap_copy(mask, child->nodeset);
        hwloc_bitmap_andnot(mask, mask, child->allowed_nodeset);
        hwloc_bitmap_andnot(obj->allowed_nodeset, obj->allowed_nodeset, mask);
      }
    }
  }
  hwloc_bitmap_free(mask);

  if (obj->nodeset) {
    /* Apply complete nodeset to nodeset and allowed_nodeset */
    if (obj->complete_nodeset)
      hwloc_bitmap_and(obj->nodeset, obj->nodeset, obj->complete_nodeset);
    else
      obj->complete_nodeset = hwloc_bitmap_dup(obj->nodeset);
    if (obj->allowed_nodeset)
      hwloc_bitmap_and(obj->allowed_nodeset, obj->allowed_nodeset, obj->complete_nodeset);
    else
      obj->allowed_nodeset = hwloc_bitmap_dup(obj->complete_nodeset);
  }
}

static void
apply_nodeset(hwloc_obj_t obj, hwloc_obj_t sys)
{
  unsigned i;
  hwloc_obj_t child, *temp;

  if (sys) {
    if (obj->type == HWLOC_OBJ_NODE && obj->os_index != (unsigned) -1 &&
        !hwloc_bitmap_isset(sys->allowed_nodeset, obj->os_index)) {
      hwloc_debug("Dropping memory from disallowed node %u\n", obj->os_index);
      obj->memory.local_memory = 0;
      obj->memory.total_memory = 0;
      for(i=0; i<obj->memory.page_types_len; i++)
	obj->memory.page_types[i].count = 0;
    }
  } else {
    if (obj->allowed_nodeset) {
      sys = obj;
    }
  }

  for_each_child_safe(child, obj, temp)
    apply_nodeset(child, sys);
}

static void
remove_unused_cpusets(hwloc_obj_t obj)
{
  hwloc_obj_t child, *temp;

  if (obj->cpuset) {
    hwloc_bitmap_and(obj->cpuset, obj->cpuset, obj->online_cpuset);
    hwloc_bitmap_and(obj->cpuset, obj->cpuset, obj->allowed_cpuset);
  }

  for_each_child_safe(child, obj, temp)
    remove_unused_cpusets(child);
}

/* Remove an object from its parent and free it.
 * Only updates next_sibling/first_child pointers,
 * so may only be used during early discovery.
 * Children are inserted where the object was.
 */
static void
unlink_and_free_single_object(hwloc_obj_t *pparent)
{
  hwloc_obj_t parent = *pparent;
  hwloc_obj_t child = parent->first_child;
  /* Replace object with its list of children */
  if (child) {
    *pparent = child;
    while (child->next_sibling)
      child = child->next_sibling;
    child->next_sibling = parent->next_sibling;
  } else
    *pparent = parent->next_sibling;
  hwloc_free_unlinked_object(parent);
}

/* Remove all ignored objects.  */
static int
remove_ignored(hwloc_topology_t topology, hwloc_obj_t *pparent)
{
  hwloc_obj_t parent = *pparent, child, *pchild;
  int dropped_children = 0;
  int dropped = 0;

  for_each_child_safe(child, parent, pchild)
    dropped_children += remove_ignored(topology, pchild);

  if ((parent != topology->levels[0][0] &&
       topology->ignored_types[parent->type] == HWLOC_IGNORE_TYPE_ALWAYS)
      || (parent->type == HWLOC_OBJ_CACHE && parent->attr->cache.type == HWLOC_OBJ_CACHE_INSTRUCTION
	  && !(topology->flags & HWLOC_TOPOLOGY_FLAG_ICACHES))) {
    hwloc_debug("%s", "\nDropping ignored object ");
    print_object(topology, 0, parent);
    unlink_and_free_single_object(pparent);
    dropped = 1;

  } else if (dropped_children) {
    /* we keep this object but its children changed, reorder them by cpuset */

    /* move the children list on the side */
    hwloc_obj_t *prev, children = parent->first_child;
    parent->first_child = NULL;
    while (children) {
      /* dequeue child */
      child = children;
      children = child->next_sibling;
      /* find where to enqueue it */
      prev = &parent->first_child;
      while (*prev
	     && (!child->cpuset || !(*prev)->cpuset
		 || hwloc_bitmap_compare_first(child->cpuset, (*prev)->cpuset) > 0))
	prev = &((*prev)->next_sibling);
      /* enqueue */
      child->next_sibling = *prev;
      *prev = child;
    }
  }

  return dropped;
}

/* Remove an object and its children from its parent and free them.
 * Only updates next_sibling/first_child pointers,
 * so may only be used during early discovery.
 */
static void
unlink_and_free_object_and_children(hwloc_obj_t *pobj)
{
  hwloc_obj_t obj = *pobj, child, *pchild;

  for_each_child_safe(child, obj, pchild)
    unlink_and_free_object_and_children(pchild);

  *pobj = obj->next_sibling;
  hwloc_free_unlinked_object(obj);
}

/* Remove all children whose cpuset is empty, except NUMA nodes
 * since we want to keep memory information, and except PCI bridges and devices.
 */
static void
remove_empty(hwloc_topology_t topology, hwloc_obj_t *pobj)
{
  hwloc_obj_t obj = *pobj, child, *pchild;

  for_each_child_safe(child, obj, pchild)
    remove_empty(topology, pchild);

  if (obj->type != HWLOC_OBJ_NODE
      && !obj->first_child /* only remove if all children were removed above, so that we don't remove parents of NUMAnode */
      && !hwloc_obj_type_is_io(obj->type) && obj->type != HWLOC_OBJ_MISC
      && obj->cpuset /* don't remove if no cpuset at all, there's likely a good reason why it's different from having an empty cpuset */
      && hwloc_bitmap_iszero(obj->cpuset)) {
    /* Remove empty children */
    hwloc_debug("%s", "\nRemoving empty object ");
    print_object(topology, 0, obj);
    unlink_and_free_single_object(pobj);
  }
}

/* adjust object cpusets according the given droppedcpuset,
 * drop object whose cpuset becomes empty,
 * and mark dropped nodes in droppednodeset
 */
static void
restrict_object(hwloc_topology_t topology, unsigned long flags, hwloc_obj_t *pobj, hwloc_const_cpuset_t droppedcpuset, hwloc_nodeset_t droppednodeset, int droppingparent)
{
  hwloc_obj_t obj = *pobj, child, *pchild;
  int dropping;
  int modified = obj->complete_cpuset && hwloc_bitmap_intersects(obj->complete_cpuset, droppedcpuset);

  hwloc_clear_object_distances(obj);

  if (obj->cpuset)
    hwloc_bitmap_andnot(obj->cpuset, obj->cpuset, droppedcpuset);
  if (obj->complete_cpuset)
    hwloc_bitmap_andnot(obj->complete_cpuset, obj->complete_cpuset, droppedcpuset);
  if (obj->online_cpuset)
    hwloc_bitmap_andnot(obj->online_cpuset, obj->online_cpuset, droppedcpuset);
  if (obj->allowed_cpuset)
    hwloc_bitmap_andnot(obj->allowed_cpuset, obj->allowed_cpuset, droppedcpuset);

  if (obj->type == HWLOC_OBJ_MISC) {
    dropping = droppingparent && !(flags & HWLOC_RESTRICT_FLAG_ADAPT_MISC);
  } else if (hwloc_obj_type_is_io(obj->type)) {
    dropping = droppingparent && !(flags & HWLOC_RESTRICT_FLAG_ADAPT_IO);
  } else {
    dropping = droppingparent || (obj->cpuset && hwloc_bitmap_iszero(obj->cpuset));
  }

  if (modified)
    for_each_child_safe(child, obj, pchild)
      restrict_object(topology, flags, pchild, droppedcpuset, droppednodeset, dropping);

  if (dropping) {
    hwloc_debug("%s", "\nRemoving object during restrict");
    print_object(topology, 0, obj);
    if (obj->type == HWLOC_OBJ_NODE)
      hwloc_bitmap_set(droppednodeset, obj->os_index);
    /* remove the object from the tree (no need to remove from levels, they will be entirely rebuilt by the caller) */
    unlink_and_free_single_object(pobj);
    /* do not remove children. if they were to be removed, they would have been already */
  }
}

/* adjust object nodesets accordingly the given droppednodeset
 */
static void
restrict_object_nodeset(hwloc_topology_t topology, hwloc_obj_t *pobj, hwloc_nodeset_t droppednodeset)
{
  hwloc_obj_t obj = *pobj, child, *pchild;

  /* if this object isn't modified, don't bother looking at children */
  if (obj->complete_nodeset && !hwloc_bitmap_intersects(obj->complete_nodeset, droppednodeset))
    return;

  if (obj->nodeset)
    hwloc_bitmap_andnot(obj->nodeset, obj->nodeset, droppednodeset);
  if (obj->complete_nodeset)
    hwloc_bitmap_andnot(obj->complete_nodeset, obj->complete_nodeset, droppednodeset);
  if (obj->allowed_nodeset)
    hwloc_bitmap_andnot(obj->allowed_nodeset, obj->allowed_nodeset, droppednodeset);

  for_each_child_safe(child, obj, pchild)
    restrict_object_nodeset(topology, pchild, droppednodeset);
}

/* we don't want to merge groups that were inserted explicitly with the custom interface */
static int
can_merge_group(hwloc_topology_t topology, hwloc_obj_t obj)
{
  const char *value;
  /* custom-inserted groups are in custom topologies and have no cpusets,
   * don't bother calling hwloc_obj_get_info_by_name() and strcmp() uselessly.
   */
  if (!topology->backends->is_custom || obj->cpuset)
    return 1;
  value = hwloc_obj_get_info_by_name(obj, "Backend");
  return (!value) || strcmp(value, "Custom");
}

/*
 * Merge with the only child if either the parent or the child has a type to be
 * ignored while keeping structure
 */
static void
merge_useless_child(hwloc_topology_t topology, hwloc_obj_t *pparent)
{
  hwloc_obj_t parent = *pparent, child, *pchild, ios;
  int replacechild = 0, replaceparent = 0;

  for_each_child_safe(child, parent, pchild)
    merge_useless_child(topology, pchild);

  child = parent->first_child;
  if (!child)
    /* There are no child, nothing to merge. */
    return;

  if (child->next_sibling && !hwloc_obj_type_is_io(child->next_sibling->type))
    /* There are several non-I/O children */
    return;

  /* There is one non-I/O child and possible some I/O children.
   * I/O children shouldn't prevent merging because they can be attached
   * to anything with the same locality.
   * Move them to the side during merging, and append them back later.
   * This is easy because I/O children are always last in the list.
   */
  ios = child->next_sibling;
  child->next_sibling = NULL;

  /* Check whether parent and/or child can be replaced */
  if (topology->ignored_types[parent->type] == HWLOC_IGNORE_TYPE_KEEP_STRUCTURE) {
    if (parent->type != HWLOC_OBJ_GROUP || can_merge_group(topology, parent))
      /* Parent can be ignored in favor of the child.  */
      replaceparent = 1;
  }
  if (topology->ignored_types[child->type] == HWLOC_IGNORE_TYPE_KEEP_STRUCTURE) {
    if (child->type != HWLOC_OBJ_GROUP || can_merge_group(topology, child))
      /* Child can be ignored in favor of the parent.  */
      replacechild = 1;
  }

  /* Decide which one to actually replace */
  if (replaceparent && replacechild) {
    /* If both may be replaced, look at obj_type_priority */
    if (obj_type_priority[parent->type] > obj_type_priority[child->type])
      replaceparent = 0;
    else
      replacechild = 0;
  }

  if (replaceparent) {
    /* Replace parent with child */
    hwloc_debug("%s", "\nIgnoring parent ");
    print_object(topology, 0, parent);
    if (parent == topology->levels[0][0]) {
      child->parent = NULL;
      child->depth = 0;
    }
    *pparent = child;
    child->next_sibling = parent->next_sibling;
    hwloc_free_unlinked_object(parent);

  } else if (replacechild) {
    /* Replace child with parent */
    hwloc_debug("%s", "\nIgnoring child ");
    print_object(topology, 0, child);
    parent->first_child = child->first_child;
    hwloc_free_unlinked_object(child);
  }

  if (ios) {
    /* append I/O children to the list of children of the remaining object */
    pchild = &((*pparent)->first_child);
    while (*pchild)
      pchild = &((*pchild)->next_sibling);
    *pchild = ios;
  }
}

static void
hwloc_drop_all_io(hwloc_topology_t topology, hwloc_obj_t root)
{
  hwloc_obj_t child, *pchild;
  for_each_child_safe(child, root, pchild) {
    if (hwloc_obj_type_is_io(child->type))
      unlink_and_free_object_and_children(pchild);
    else
      hwloc_drop_all_io(topology, child);
  }
}

/*
 * If IO_DEVICES and WHOLE_IO are not set, we drop everything.
 * If WHOLE_IO is not set, we drop non-interesting devices,
 * and bridges that have no children.
 * If IO_BRIDGES is also not set, we also drop all bridges
 * except the hostbridges.
 */
static void
hwloc_drop_useless_io(hwloc_topology_t topology, hwloc_obj_t root)
{
  hwloc_obj_t child, *pchild;

  if (!(topology->flags & (HWLOC_TOPOLOGY_FLAG_IO_DEVICES|HWLOC_TOPOLOGY_FLAG_WHOLE_IO))) {
    /* drop all I/O children */
    hwloc_drop_all_io(topology, root);
    return;
  }

  if (!(topology->flags & HWLOC_TOPOLOGY_FLAG_WHOLE_IO)) {
    /* drop non-interesting devices */
    for_each_child_safe(child, root, pchild) {
      if (child->type == HWLOC_OBJ_PCI_DEVICE) {
	unsigned classid = child->attr->pcidev.class_id;
	unsigned baseclass = classid >> 8;
	if (baseclass != 0x03 /* PCI_BASE_CLASS_DISPLAY */
	    && baseclass != 0x02 /* PCI_BASE_CLASS_NETWORK */
	    && baseclass != 0x01 /* PCI_BASE_CLASS_STORAGE */
	    && baseclass != 0x0b /* PCI_BASE_CLASS_PROCESSOR */
	    && classid != 0x0c06 /* PCI_CLASS_SERIAL_INFINIBAND */)
	  unlink_and_free_object_and_children(pchild);
      }
    }
  }

  /* look at remaining children, process recursively, and remove useless bridges */
  for_each_child_safe(child, root, pchild) {
    hwloc_drop_useless_io(topology, child);

    if (child->type == HWLOC_OBJ_BRIDGE) {
      hwloc_obj_t grandchildren = child->first_child;

      if (!grandchildren) {
	/* bridges with no children are removed if WHOLE_IO isn't given */
	if (!(topology->flags & (HWLOC_TOPOLOGY_FLAG_WHOLE_IO))) {
	  *pchild = child->next_sibling;
	  hwloc_free_unlinked_object(child);
	}

      } else if (child->attr->bridge.upstream_type != HWLOC_OBJ_BRIDGE_HOST) {
	/* only hostbridges are kept if WHOLE_IO or IO_BRIDGE are not given */
	if (!(topology->flags & (HWLOC_TOPOLOGY_FLAG_IO_BRIDGES|HWLOC_TOPOLOGY_FLAG_WHOLE_IO))) {
	  /* insert grandchildren in place of child */
	  *pchild = grandchildren;
	  for( ; grandchildren->next_sibling != NULL ; grandchildren = grandchildren->next_sibling);
	  grandchildren->next_sibling = child->next_sibling;
	  hwloc_free_unlinked_object(child);
	}
      }
    }
  }
}

static void
hwloc_propagate_bridge_depth(hwloc_topology_t topology, hwloc_obj_t root, unsigned depth)
{
  hwloc_obj_t child = root->first_child;
  while (child) {
    if (child->type == HWLOC_OBJ_BRIDGE) {
      child->attr->bridge.depth = depth;
      hwloc_propagate_bridge_depth(topology, child, depth+1);
    }
    child = child->next_sibling;
  }
}

static void
hwloc_propagate_symmetric_subtree(hwloc_topology_t topology, hwloc_obj_t root)
{
  hwloc_obj_t child, *array;

  /* assume we're not symmetric by default */
  root->symmetric_subtree = 0;

  /* if no child, we are symmetric */
  if (!root->arity) {
    root->symmetric_subtree = 1;
    return;
  }

  /* look at children, and return if they are not symmetric */
  child = NULL;
  while ((child = hwloc_get_next_child(topology, root, child)) != NULL)
    hwloc_propagate_symmetric_subtree(topology, child);
  while ((child = hwloc_get_next_child(topology, root, child)) != NULL)
    if (!child->symmetric_subtree)
      return;

  /* now check that children subtrees are identical.
   * just walk down the first child in each tree and compare their depth and arities
   */
  array = malloc(root->arity * sizeof(*array));
  memcpy(array, root->children, root->arity * sizeof(*array));
  while (1) {
    unsigned i;
    /* check current level arities and depth */
    for(i=1; i<root->arity; i++)
      if (array[i]->depth != array[0]->depth
	  || array[i]->arity != array[0]->arity) {
      free(array);
      return;
    }
    if (!array[0]->arity)
      /* no more children level, we're ok */
      break;
    /* look at first child of each element now */
    for(i=0; i<root->arity; i++)
      array[i] = array[i]->first_child;
  }
  free(array);

  /* everything went fine, we're symmetric */
  root->symmetric_subtree = 1;
}

/*
 * Initialize handy pointers in the whole topology.
 * The topology only had first_child and next_sibling pointers.
 * When this funtions return, all parent/children pointers are initialized.
 * The remaining fields (levels, cousins, logical_index, depth, ...) will
 * be setup later in hwloc_connect_levels().
 */
void
hwloc_connect_children(hwloc_obj_t parent)
{
  unsigned n;
  hwloc_obj_t child, prev_child = NULL;

  for (n = 0, child = parent->first_child;
       child;
       n++,   prev_child = child, child = child->next_sibling) {
    child->parent = parent;
    child->sibling_rank = n;
    child->prev_sibling = prev_child;
  }
  parent->last_child = prev_child;

  parent->arity = n;
  free(parent->children);
  if (!n) {
    parent->children = NULL;
    return;
  }

  parent->children = malloc(n * sizeof(*parent->children));
  for (n = 0, child = parent->first_child;
       child;
       n++,   child = child->next_sibling) {
    parent->children[n] = child;
    hwloc_connect_children(child);
  }
}

/*
 * Check whether there is an object below ROOT that has the same type as OBJ
 */
static int
find_same_type(hwloc_obj_t root, hwloc_obj_t obj)
{
  hwloc_obj_t child;

  if (hwloc_type_cmp(root, obj) == HWLOC_TYPE_EQUAL)
    return 1;

  for (child = root->first_child; child; child = child->next_sibling)
    if (find_same_type(child, obj))
      return 1;

  return 0;
}

/* traverse the array of current object and compare them with top_obj.
 * if equal, take the object and put its children into the remaining objs.
 * if not equal, put the object into the remaining objs.
 */
static int
hwloc_level_take_objects(hwloc_obj_t top_obj,
			 hwloc_obj_t *current_objs, unsigned n_current_objs,
			 hwloc_obj_t *taken_objs, unsigned n_taken_objs __hwloc_attribute_unused,
			 hwloc_obj_t *remaining_objs, unsigned n_remaining_objs __hwloc_attribute_unused)
{
  unsigned taken_i = 0;
  unsigned new_i = 0;
  unsigned i, j;

  for (i = 0; i < n_current_objs; i++)
    if (hwloc_type_cmp(top_obj, current_objs[i]) == HWLOC_TYPE_EQUAL) {
      /* Take it, add children.  */
      taken_objs[taken_i++] = current_objs[i];
      for (j = 0; j < current_objs[i]->arity; j++)
	remaining_objs[new_i++] = current_objs[i]->children[j];
    } else {
      /* Leave it.  */
      remaining_objs[new_i++] = current_objs[i];
    }

#ifdef HWLOC_DEBUG
  /* Make sure we didn't mess up.  */
  assert(taken_i == n_taken_objs);
  assert(new_i == n_current_objs - n_taken_objs + n_remaining_objs);
#endif

  return new_i;
}

/* Given an input object, copy it or its interesting children into the output array.
 * If new_obj is NULL, we're just counting interesting ohjects.
 */
static unsigned
hwloc_level_filter_object(hwloc_topology_t topology,
			  hwloc_obj_t *new_obj, hwloc_obj_t old)
{
  unsigned i, total;
  if (hwloc_obj_type_is_io(old->type)) {
    if (new_obj)
      append_iodevs(topology, old);
    return 0;
  }
  if (old->type != HWLOC_OBJ_MISC) {
    if (new_obj)
      *new_obj = old;
    return 1;
  }
  for(i=0, total=0; i<old->arity; i++) {
    int nb = hwloc_level_filter_object(topology, new_obj, old->children[i]);
    if (new_obj) {
      new_obj += nb;
    }
    total += nb;
  }
  return total;
}

/* Replace an input array of objects with an input array containing
 * only interesting objects for levels.
 * Misc objects are removed, their interesting children are added.
 * I/O devices are removed and queue to their own lists.
 */
static int
hwloc_level_filter_objects(hwloc_topology_t topology,
			   hwloc_obj_t **objs, unsigned *n_objs)
{
  hwloc_obj_t *old = *objs, *new;
  unsigned nold = *n_objs, nnew, i;

  /* anything to filter? */
  for(i=0; i<nold; i++)
    if (hwloc_obj_type_is_io(old[i]->type)
	|| old[i]->type == HWLOC_OBJ_MISC)
      break;
  if (i==nold)
    return 0;

  /* count interesting objects and allocate the new array */
  for(i=0, nnew=0; i<nold; i++)
    nnew += hwloc_level_filter_object(topology, NULL, old[i]);
  new = malloc(nnew * sizeof(hwloc_obj_t));
  if (!new) {
    free(old);
    errno = ENOMEM;
    return -1;
  }
  /* copy them now */
  for(i=0, nnew=0; i<nold; i++)
    nnew += hwloc_level_filter_object(topology, new+nnew, old[i]);

  /* use the new array */
  *objs = new;
  *n_objs = nnew;
  free(old);
  return 0;
}

static unsigned
hwloc_build_level_from_list(struct hwloc_obj *first, struct hwloc_obj ***levelp)
{
  unsigned i, nb;
  struct hwloc_obj * obj;

  /* count */
  obj = first;
  i = 0;
  while (obj) {
    i++;
    obj = obj->next_cousin;
  }
  nb = i;

  /* allocate and fill level */
  *levelp = malloc(nb * sizeof(struct hwloc_obj *));
  obj = first;
  i = 0;
  while (obj) {
    obj->logical_index = i;
    (*levelp)[i] = obj;
    i++;
    obj = obj->next_cousin;
  }

  return nb;
}

/*
 * Do the remaining work that hwloc_connect_children() did not do earlier.
 */
int
hwloc_connect_levels(hwloc_topology_t topology)
{
  unsigned l, i=0;
  hwloc_obj_t *objs, *taken_objs, *new_objs, top_obj;
  unsigned n_objs, n_taken_objs, n_new_objs;
  int err;

  /* reset non-root levels (root was initialized during init and will not change here) */
  for(l=1; l<HWLOC_DEPTH_MAX; l++)
    free(topology->levels[l]);
  memset(topology->levels+1, 0, (HWLOC_DEPTH_MAX-1)*sizeof(*topology->levels));
  memset(topology->level_nbobjects+1, 0,  (HWLOC_DEPTH_MAX-1)*sizeof(*topology->level_nbobjects));
  topology->nb_levels = 1;
  /* don't touch next_group_depth, the Group objects are still here */

  /* initialize all depth to unknown */
  for (l = HWLOC_OBJ_SYSTEM; l < HWLOC_OBJ_TYPE_MAX; l++)
    topology->type_depth[l] = HWLOC_TYPE_DEPTH_UNKNOWN;
  /* initialize root type depth */
  topology->type_depth[topology->levels[0][0]->type] = 0;

  /* initialize I/O special levels */
  free(topology->bridge_level);
  topology->bridge_level = NULL;
  topology->bridge_nbobjects = 0;
  topology->first_bridge = topology->last_bridge = NULL;
  topology->type_depth[HWLOC_OBJ_BRIDGE] = HWLOC_TYPE_DEPTH_BRIDGE;
  free(topology->pcidev_level);
  topology->pcidev_level = NULL;
  topology->pcidev_nbobjects = 0;
  topology->first_pcidev = topology->last_pcidev = NULL;
  topology->type_depth[HWLOC_OBJ_PCI_DEVICE] = HWLOC_TYPE_DEPTH_PCI_DEVICE;
  free(topology->osdev_level);
  topology->osdev_level = NULL;
  topology->osdev_nbobjects = 0;
  topology->first_osdev = topology->last_osdev = NULL;
  topology->type_depth[HWLOC_OBJ_OS_DEVICE] = HWLOC_TYPE_DEPTH_OS_DEVICE;

  /* Start with children of the whole system.  */
  n_objs = topology->levels[0][0]->arity;
  objs = malloc(n_objs * sizeof(objs[0]));
  if (!objs) {
    errno = ENOMEM;
    return -1;
  }
  memcpy(objs, topology->levels[0][0]->children, n_objs*sizeof(objs[0]));

  /* Filter-out interesting objects */
  err = hwloc_level_filter_objects(topology, &objs, &n_objs);
  if (err < 0)
    return -1;

  /* Keep building levels while there are objects left in OBJS.  */
  while (n_objs) {
    /* At this point, the objs array contains only objects that may go into levels */

    /* First find which type of object is the topmost.
     * Don't use PU if there are other types since we want to keep PU at the bottom.
     */

    /* Look for the first non-PU object, and use the first PU if we really find nothing else */
    for (i = 0; i < n_objs; i++)
      if (objs[i]->type != HWLOC_OBJ_PU)
        break;
    top_obj = i == n_objs ? objs[0] : objs[i];

    /* See if this is actually the topmost object */
    for (i = 0; i < n_objs; i++) {
      if (hwloc_type_cmp(top_obj, objs[i]) != HWLOC_TYPE_EQUAL) {
	if (find_same_type(objs[i], top_obj)) {
	  /* OBJS[i] is strictly above an object of the same type as TOP_OBJ, so it
	   * is above TOP_OBJ.  */
	  top_obj = objs[i];
	}
      }
    }

    /* Now peek all objects of the same type, build a level with that and
     * replace them with their children.  */

    /* First count them.  */
    n_taken_objs = 0;
    n_new_objs = 0;
    for (i = 0; i < n_objs; i++)
      if (hwloc_type_cmp(top_obj, objs[i]) == HWLOC_TYPE_EQUAL) {
	n_taken_objs++;
	n_new_objs += objs[i]->arity;
      }

    /* New level.  */
    taken_objs = malloc((n_taken_objs + 1) * sizeof(taken_objs[0]));
    /* New list of pending objects.  */
    if (n_objs - n_taken_objs + n_new_objs) {
      new_objs = malloc((n_objs - n_taken_objs + n_new_objs) * sizeof(new_objs[0]));
    } else {
#ifdef HWLOC_DEBUG
      assert(!n_new_objs);
      assert(n_objs == n_taken_objs);
#endif
      new_objs = NULL;
    }

    n_new_objs = hwloc_level_take_objects(top_obj,
					  objs, n_objs,
					  taken_objs, n_taken_objs,
					  new_objs, n_new_objs);

    /* Ok, put numbers in the level and link cousins.  */
    for (i = 0; i < n_taken_objs; i++) {
      taken_objs[i]->depth = topology->nb_levels;
      taken_objs[i]->logical_index = i;
      if (i) {
	taken_objs[i]->prev_cousin = taken_objs[i-1];
	taken_objs[i-1]->next_cousin = taken_objs[i];
      }
    }
    taken_objs[0]->prev_cousin = NULL;
    taken_objs[n_taken_objs-1]->next_cousin = NULL;

    /* One more level!  */
    if (top_obj->type == HWLOC_OBJ_CACHE)
      hwloc_debug("--- Cache level depth %u", top_obj->attr->cache.depth);
    else
      hwloc_debug("--- %s level", hwloc_obj_type_string(top_obj->type));
    hwloc_debug(" has number %u\n\n", topology->nb_levels);

    if (topology->type_depth[top_obj->type] == HWLOC_TYPE_DEPTH_UNKNOWN)
      topology->type_depth[top_obj->type] = topology->nb_levels;
    else
      topology->type_depth[top_obj->type] = HWLOC_TYPE_DEPTH_MULTIPLE; /* mark as unknown */

    taken_objs[n_taken_objs] = NULL;

    topology->level_nbobjects[topology->nb_levels] = n_taken_objs;
    topology->levels[topology->nb_levels] = taken_objs;

    topology->nb_levels++;

    free(objs);

    /* Switch to new_objs, after filtering-out interesting objects */
    err = hwloc_level_filter_objects(topology, &new_objs, &n_new_objs);
    if (err < 0)
      return -1;

    objs = new_objs;
    n_objs = n_new_objs;
  }

  /* It's empty now.  */
  if (objs)
    free(objs);

  topology->bridge_nbobjects = hwloc_build_level_from_list(topology->first_bridge, &topology->bridge_level);
  topology->pcidev_nbobjects = hwloc_build_level_from_list(topology->first_pcidev, &topology->pcidev_level);
  topology->osdev_nbobjects = hwloc_build_level_from_list(topology->first_osdev, &topology->osdev_level);

  hwloc_propagate_symmetric_subtree(topology, topology->levels[0][0]);

  return 0;
}

void hwloc_alloc_obj_cpusets(hwloc_obj_t obj)
{
  obj->cpuset = hwloc_bitmap_alloc_full();
  obj->complete_cpuset = hwloc_bitmap_alloc();
  obj->online_cpuset = hwloc_bitmap_alloc_full();
  obj->allowed_cpuset = hwloc_bitmap_alloc_full();
  obj->nodeset = hwloc_bitmap_alloc();
  obj->complete_nodeset = hwloc_bitmap_alloc();
  obj->allowed_nodeset = hwloc_bitmap_alloc_full();
}

/* Main discovery loop */
static int
hwloc_discover(struct hwloc_topology *topology)
{
  struct hwloc_backend *backend;
  int gotsomeio = 0;
  unsigned discoveries = 0;
  unsigned need_reconnect = 0;

  /* discover() callbacks should use hwloc_insert to add objects initialized
   * through hwloc_alloc_setup_object.
   * For node levels, nodeset and memory must be initialized.
   * For cache levels, memory and type/depth must be initialized.
   * For group levels, depth must be initialized.
   */

  /* There must be at least a PU object for each logical processor, at worse
   * produced by hwloc_setup_pu_level()
   */

  /* To be able to just use hwloc_insert_object_by_cpuset to insert the object
   * in the topology according to the cpuset, the cpuset field must be
   * initialized.
   */

  /* A priori, All processors are visible in the topology, online, and allowed
   * for the application.
   *
   * - If some processors exist but topology information is unknown for them
   *   (and thus the backend couldn't create objects for them), they should be
   *   added to the complete_cpuset field of the lowest object where the object
   *   could reside.
   *
   * - If some processors are not online, they should be dropped from the
   *   online_cpuset field.
   *
   * - If some processors are not allowed for the application (e.g. for
   *   administration reasons), they should be dropped from the allowed_cpuset
   *   field.
   *
   * The same applies to the node sets complete_nodeset and allowed_cpuset.
   *
   * If such field doesn't exist yet, it can be allocated, and initialized to
   * zero (for complete), or to full (for online and allowed). The values are
   * automatically propagated to the whole tree after detection.
   */

  /*
   * Discover CPUs first
   */
  backend = topology->backends;
  while (NULL != backend) {
    int err;
    if (backend->component->type != HWLOC_DISC_COMPONENT_TYPE_CPU
	&& backend->component->type != HWLOC_DISC_COMPONENT_TYPE_GLOBAL)
      /* not yet */
      goto next_cpubackend;
    if (!backend->discover)
      goto next_cpubackend;

    if (need_reconnect && (backend->flags & HWLOC_BACKEND_FLAG_NEED_LEVELS)) {
      hwloc_debug("Backend %s forcing a reconnect of levels\n", backend->component->name);
      hwloc_connect_children(topology->levels[0][0]);
      if (hwloc_connect_levels(topology) < 0)
	return -1;
      need_reconnect = 0;
    }

    err = backend->discover(backend);
    if (err >= 0) {
      if (backend->component->type == HWLOC_DISC_COMPONENT_TYPE_GLOBAL)
        gotsomeio += err;
      discoveries++;
      if (err > 0)
	need_reconnect++;
    }
    print_objects(topology, 0, topology->levels[0][0]);

next_cpubackend:
    backend = backend->next;
  }

  if (!discoveries) {
    hwloc_debug("%s", "No CPU backend enabled or no discovery succeeded\n");
    errno = EINVAL;
    return -1;
  }

  /*
   * Group levels by distances
   */
  hwloc_distances_finalize_os(topology);
  hwloc_group_by_distances(topology);

  /* Update objects cpusets and nodesets now that the CPU/GLOBAL backend populated PUs and nodes */

  hwloc_debug("%s", "\nRestrict topology cpusets to existing PU and NODE objects\n");
  collect_proc_cpuset(topology->levels[0][0], NULL);

  hwloc_debug("%s", "\nPropagate offline and disallowed cpus down and up\n");
  propagate_unused_cpuset(topology->levels[0][0], NULL);

  if (topology->levels[0][0]->complete_nodeset && hwloc_bitmap_iszero(topology->levels[0][0]->complete_nodeset)) {
    /* No nodeset, drop all of them */
    hwloc_bitmap_free(topology->levels[0][0]->nodeset);
    topology->levels[0][0]->nodeset = NULL;
    hwloc_bitmap_free(topology->levels[0][0]->complete_nodeset);
    topology->levels[0][0]->complete_nodeset = NULL;
    hwloc_bitmap_free(topology->levels[0][0]->allowed_nodeset);
    topology->levels[0][0]->allowed_nodeset = NULL;
  }
  hwloc_debug("%s", "\nPropagate nodesets\n");
  propagate_nodeset(topology->levels[0][0], NULL);
  propagate_nodesets(topology->levels[0][0]);

  print_objects(topology, 0, topology->levels[0][0]);

  if (!(topology->flags & HWLOC_TOPOLOGY_FLAG_WHOLE_SYSTEM)) {
    hwloc_debug("%s", "\nRemoving unauthorized and offline cpusets from all cpusets\n");
    remove_unused_cpusets(topology->levels[0][0]);

    hwloc_debug("%s", "\nRemoving disallowed memory according to nodesets\n");
    apply_nodeset(topology->levels[0][0], NULL);

    print_objects(topology, 0, topology->levels[0][0]);
  }

  hwloc_debug("%s", "\nAdd default object sets\n");
  add_default_object_sets(topology->levels[0][0], 0);

  /* Now connect handy pointers to make remaining discovery easier. */
  hwloc_debug("%s", "\nOk, finished tweaking, now connect\n");
  hwloc_connect_children(topology->levels[0][0]);
  if (hwloc_connect_levels(topology) < 0)
    return -1;
  print_objects(topology, 0, topology->levels[0][0]);

  /*
   * Additional discovery with other backends
   */

  backend = topology->backends;
  need_reconnect = 0;
  while (NULL != backend) {
    int err;
    if (backend->component->type == HWLOC_DISC_COMPONENT_TYPE_CPU
	|| backend->component->type == HWLOC_DISC_COMPONENT_TYPE_GLOBAL)
      /* already done above */
      goto next_noncpubackend;
    if (!backend->discover)
      goto next_noncpubackend;

    if (need_reconnect && (backend->flags & HWLOC_BACKEND_FLAG_NEED_LEVELS)) {
      hwloc_debug("Backend %s forcing a reconnect of levels\n", backend->component->name);
      hwloc_connect_children(topology->levels[0][0]);
      if (hwloc_connect_levels(topology) < 0)
	return -1;
      need_reconnect = 0;
    }

    err = backend->discover(backend);
    if (err >= 0) {
      gotsomeio += err;
      if (err > 0)
	need_reconnect++;
    }
    print_objects(topology, 0, topology->levels[0][0]);

next_noncpubackend:
    backend = backend->next;
  }

  /* if we got anything, filter interesting objects and update the tree */
  if (gotsomeio) {
    hwloc_drop_useless_io(topology, topology->levels[0][0]);
    hwloc_debug("%s", "\nNow reconnecting\n");
    print_objects(topology, 0, topology->levels[0][0]);
    hwloc_propagate_bridge_depth(topology, topology->levels[0][0], 0);
  }

  /* Removed some stuff */

  hwloc_debug("%s", "\nRemoving ignored objects\n");
  remove_ignored(topology, &topology->levels[0][0]);
  print_objects(topology, 0, topology->levels[0][0]);

  hwloc_debug("%s", "\nRemoving empty objects except numa nodes and PCI devices\n");
  remove_empty(topology, &topology->levels[0][0]);
  if (!topology->levels[0][0]) {
    fprintf(stderr, "Topology became empty, aborting!\n");
    abort();
  }
  print_objects(topology, 0, topology->levels[0][0]);

  hwloc_debug("%s", "\nRemoving objects whose type has HWLOC_IGNORE_TYPE_KEEP_STRUCTURE and have only one child or are the only child\n");
  merge_useless_child(topology, &topology->levels[0][0]);
  print_objects(topology, 0, topology->levels[0][0]);

  /* Reconnect things after all these changes */
  hwloc_connect_children(topology->levels[0][0]);
  if (hwloc_connect_levels(topology) < 0)
    return -1;

  /* accumulate children memory in total_memory fields (only once parent is set) */
  hwloc_debug("%s", "\nPropagate total memory up\n");
  propagate_total_memory(topology->levels[0][0]);

  /*
   * Now that objects are numbered, take distance matrices from backends and put them in the main topology.
   *
   * Some objects may have disappeared (in removed_empty or removed_ignored) since we setup os distances
   * (hwloc_distances_finalize_os()) above. Reset them so as to not point to disappeared objects anymore.
   */
  hwloc_distances_restrict_os(topology);
  hwloc_distances_finalize_os(topology);
  hwloc_distances_finalize_logical(topology);

  /*
   * Now set binding hooks according to topology->is_thissystem
   * what the native OS backend offers.
   */
  hwloc_set_binding_hooks(topology);

  return 0;
}

/* To be before discovery is actually launched,
 * Resets everything in case a previous load initialized some stuff.
 */
void
hwloc_topology_setup_defaults(struct hwloc_topology *topology)
{
  struct hwloc_obj *root_obj;

  /* reset support */
  memset(&topology->binding_hooks, 0, sizeof(topology->binding_hooks));
  memset(topology->support.discovery, 0, sizeof(*topology->support.discovery));
  memset(topology->support.cpubind, 0, sizeof(*topology->support.cpubind));
  memset(topology->support.membind, 0, sizeof(*topology->support.membind));

  /* Only the System object on top by default */
  topology->nb_levels = 1; /* there's at least SYSTEM */
  topology->next_group_depth = 0;
  topology->levels[0] = malloc (sizeof (hwloc_obj_t));
  topology->level_nbobjects[0] = 1;
  /* NULLify other levels so that we can detect and free old ones in hwloc_connect_levels() if needed */
  memset(topology->levels+1, 0, (HWLOC_DEPTH_MAX-1)*sizeof(*topology->levels));
  topology->bridge_level = NULL;
  topology->pcidev_level = NULL;
  topology->osdev_level = NULL;
  topology->first_bridge = topology->last_bridge = NULL;
  topology->first_pcidev = topology->last_pcidev = NULL;
  topology->first_osdev = topology->last_osdev = NULL;

  /* Create the actual machine object, but don't touch its attributes yet
   * since the OS backend may still change the object into something else
   * (for instance System)
   */
  root_obj = hwloc_alloc_setup_object(HWLOC_OBJ_MACHINE, 0);
  root_obj->depth = 0;
  root_obj->logical_index = 0;
  root_obj->sibling_rank = 0;
  topology->levels[0][0] = root_obj;
}

int
hwloc_topology_init (struct hwloc_topology **topologyp)
{
  struct hwloc_topology *topology;
  int i;

  topology = malloc (sizeof (struct hwloc_topology));
  if(!topology)
    return -1;

  hwloc_components_init(topology);

  /* Setup topology context */
  topology->is_loaded = 0;
  topology->flags = 0;
  topology->is_thissystem = 1;
  topology->pid = 0;

  topology->support.discovery = malloc(sizeof(*topology->support.discovery));
  topology->support.cpubind = malloc(sizeof(*topology->support.cpubind));
  topology->support.membind = malloc(sizeof(*topology->support.membind));

  /* Only ignore useless cruft by default */
  for(i = HWLOC_OBJ_SYSTEM; i < HWLOC_OBJ_TYPE_MAX; i++)
    topology->ignored_types[i] = HWLOC_IGNORE_TYPE_NEVER;
  topology->ignored_types[HWLOC_OBJ_GROUP] = HWLOC_IGNORE_TYPE_KEEP_STRUCTURE;

  hwloc_distances_init(topology);

  topology->userdata_export_cb = NULL;
  topology->userdata_import_cb = NULL;

  /* Make the topology look like something coherent but empty */
  hwloc_topology_setup_defaults(topology);

  *topologyp = topology;
  return 0;
}

int
hwloc_topology_set_pid(struct hwloc_topology *topology __hwloc_attribute_unused,
                       hwloc_pid_t pid __hwloc_attribute_unused)
{
  /* this does *not* change the backend */
#ifdef HWLOC_LINUX_SYS
  topology->pid = pid;
  return 0;
#else /* HWLOC_LINUX_SYS */
  errno = ENOSYS;
  return -1;
#endif /* HWLOC_LINUX_SYS */
}

int
hwloc_topology_set_fsroot(struct hwloc_topology *topology, const char *fsroot_path)
{
  return hwloc_disc_component_force_enable(topology,
					   0 /* api */,
					   HWLOC_DISC_COMPONENT_TYPE_CPU, "linux",
					   fsroot_path, NULL, NULL);
}

int
hwloc_topology_set_synthetic(struct hwloc_topology *topology, const char *description)
{
  return hwloc_disc_component_force_enable(topology,
					   0 /* api */,
					   -1, "synthetic",
					   description, NULL, NULL);
}

int
hwloc_topology_set_xml(struct hwloc_topology *topology,
		       const char *xmlpath)
{
  return hwloc_disc_component_force_enable(topology,
					   0 /* api */,
					   -1, "xml",
					   xmlpath, NULL, NULL);
}

int
hwloc_topology_set_xmlbuffer(struct hwloc_topology *topology,
                             const char *xmlbuffer,
                             int size)
{
  return hwloc_disc_component_force_enable(topology,
					   0 /* api */,
					   -1, "xml", NULL,
					   xmlbuffer, (void*) (uintptr_t) size);
}

int
hwloc_topology_set_custom(struct hwloc_topology *topology)
{
  return hwloc_disc_component_force_enable(topology,
					   0 /* api */,
					   -1, "custom",
					   NULL, NULL, NULL);
}

int
hwloc_topology_set_flags (struct hwloc_topology *topology, unsigned long flags)
{
  if (topology->is_loaded) {
    /* actually harmless */
    errno = EBUSY;
    return -1;
  }
  topology->flags = flags;
  return 0;
}

unsigned long
hwloc_topology_get_flags (struct hwloc_topology *topology)
{
  return topology->flags;
}

int
hwloc_topology_ignore_type(struct hwloc_topology *topology, hwloc_obj_type_t type)
{
  if (type >= HWLOC_OBJ_TYPE_MAX) {
    errno = EINVAL;
    return -1;
  }

  if (type == HWLOC_OBJ_PU) {
    /* we need the PU level */
    errno = EINVAL;
    return -1;
  } else if (hwloc_obj_type_is_io(type)) {
    /* I/O devices aren't in any level, use topology flags to ignore them */
    errno = EINVAL;
    return -1;
  }

  topology->ignored_types[type] = HWLOC_IGNORE_TYPE_ALWAYS;
  return 0;
}

int
hwloc_topology_ignore_type_keep_structure(struct hwloc_topology *topology, hwloc_obj_type_t type)
{
  if (type >= HWLOC_OBJ_TYPE_MAX) {
    errno = EINVAL;
    return -1;
  }

  if (type == HWLOC_OBJ_PU) {
    /* we need the PU level */
    errno = EINVAL;
    return -1;
  } else if (hwloc_obj_type_is_io(type)) {
    /* I/O devices aren't in any level, use topology flags to ignore them */
    errno = EINVAL;
    return -1;
  }

  topology->ignored_types[type] = HWLOC_IGNORE_TYPE_KEEP_STRUCTURE;
  return 0;
}

int
hwloc_topology_ignore_all_keep_structure(struct hwloc_topology *topology)
{
  unsigned type;
  for(type = HWLOC_OBJ_SYSTEM; type < HWLOC_OBJ_TYPE_MAX; type++)
    if (type != HWLOC_OBJ_PU
	&& !hwloc_obj_type_is_io((hwloc_obj_type_t) type))
      topology->ignored_types[type] = HWLOC_IGNORE_TYPE_KEEP_STRUCTURE;
  return 0;
}

/* traverse the tree and free everything.
 * only use first_child/next_sibling so that it works before load()
 * and may be used when switching between backend.
 */
static void
hwloc_topology_clear_tree (struct hwloc_topology *topology, struct hwloc_obj *root)
{
  hwloc_obj_t child = root->first_child;
  while (child) {
    hwloc_obj_t nextchild = child->next_sibling;
    hwloc_topology_clear_tree (topology, child);
    child = nextchild;
  }
  hwloc_free_unlinked_object (root);
}

void
hwloc_topology_clear (struct hwloc_topology *topology)
{
  unsigned l;
  hwloc_topology_clear_tree (topology, topology->levels[0][0]);
  for (l=0; l<topology->nb_levels; l++) {
    free(topology->levels[l]);
    topology->levels[l] = NULL;
  }
  free(topology->bridge_level);
  free(topology->pcidev_level);
  free(topology->osdev_level);
}

void
hwloc_topology_destroy (struct hwloc_topology *topology)
{
  hwloc_backends_disable_all(topology);
  hwloc_components_destroy_all(topology);

  hwloc_topology_clear(topology);
  hwloc_distances_destroy(topology);

  free(topology->support.discovery);
  free(topology->support.cpubind);
  free(topology->support.membind);
  free(topology);
}

int
hwloc_topology_load (struct hwloc_topology *topology)
{
  int err;

  if (topology->is_loaded) {
    errno = EBUSY;
    return -1;
  }

  /* enforce backend anyway if a FORCE variable was given */
  {
    char *fsroot_path_env = getenv("HWLOC_FORCE_FSROOT");
    if (fsroot_path_env)
      hwloc_disc_component_force_enable(topology,
					1 /* env force */,
					HWLOC_DISC_COMPONENT_TYPE_CPU, "linux",
					fsroot_path_env, NULL, NULL);
  }
  {
    char *xmlpath_env = getenv("HWLOC_FORCE_XMLFILE");
    if (xmlpath_env)
      hwloc_disc_component_force_enable(topology,
					1 /* env force */,
					-1, "xml",
					xmlpath_env, NULL, NULL);
  }

  /* only apply non-FORCE variables if we have not changed the backend yet */
  if (!topology->backends) {
    char *fsroot_path_env = getenv("HWLOC_FSROOT");
    if (fsroot_path_env)
      hwloc_disc_component_force_enable(topology,
					1 /* env force */,
					HWLOC_DISC_COMPONENT_TYPE_CPU, "linux",
					fsroot_path_env, NULL, NULL);
  }
  if (!topology->backends) {
    char *xmlpath_env = getenv("HWLOC_XMLFILE");
    if (xmlpath_env)
      hwloc_disc_component_force_enable(topology,
					1 /* env force */,
					-1, "xml",
					xmlpath_env, NULL, NULL);
  }

  /* instantiate all possible other backends now */
  hwloc_disc_components_enable_others(topology);
  /* now that backends are enabled, update the thissystem flag */
  hwloc_backends_is_thissystem(topology);

  /* get distance matrix from the environment are store them (as indexes) in the topology.
   * indexes will be converted into objects later once the tree will be filled
   */
  hwloc_distances_set_from_env(topology);

  /* actual topology discovery */
  err = hwloc_discover(topology);
  if (err < 0)
    goto out;

#ifndef HWLOC_DEBUG
  if (getenv("HWLOC_DEBUG_CHECK"))
#endif
    hwloc_topology_check(topology);

  topology->is_loaded = 1;
  return 0;

 out:
  hwloc_topology_clear(topology);
  hwloc_distances_destroy(topology);
  hwloc_topology_setup_defaults(topology);
  hwloc_backends_disable_all(topology);
  return -1;
}

int
hwloc_topology_restrict(struct hwloc_topology *topology, hwloc_const_cpuset_t cpuset, unsigned long flags)
{
  hwloc_bitmap_t droppedcpuset, droppednodeset;

  /* make sure we'll keep something in the topology */
  if (!hwloc_bitmap_intersects(cpuset, topology->levels[0][0]->cpuset)) {
    errno = EINVAL; /* easy failure, just don't touch the topology */
    return -1;
  }

  droppedcpuset = hwloc_bitmap_alloc();
  droppednodeset = hwloc_bitmap_alloc();

  /* drop object based on the reverse of cpuset, and fill the 'dropped' nodeset */
  hwloc_bitmap_not(droppedcpuset, cpuset);
  restrict_object(topology, flags, &topology->levels[0][0], droppedcpuset, droppednodeset, 0 /* root cannot be removed */);
  /* update nodesets according to dropped nodeset */
  restrict_object_nodeset(topology, &topology->levels[0][0], droppednodeset);

  hwloc_bitmap_free(droppedcpuset);
  hwloc_bitmap_free(droppednodeset);

  hwloc_connect_children(topology->levels[0][0]);
  if (hwloc_connect_levels(topology) < 0)
    goto out;

  propagate_total_memory(topology->levels[0][0]);
  hwloc_distances_restrict(topology, flags);
  hwloc_distances_finalize_os(topology);
  hwloc_distances_finalize_logical(topology);
  return 0;

 out:
  /* unrecoverable failure, re-init the topology */
   hwloc_topology_clear(topology);
   hwloc_distances_destroy(topology);
   hwloc_topology_setup_defaults(topology);
   return -1;
}

int
hwloc_topology_is_thissystem(struct hwloc_topology *topology)
{
  return topology->is_thissystem;
}

unsigned
hwloc_topology_get_depth(struct hwloc_topology *topology) 
{
  return topology->nb_levels;
}

/* check children between a parent object */
static void
hwloc__check_children(struct hwloc_obj *parent)
{
  hwloc_bitmap_t remaining_parent_set;
  unsigned j;

  if (!parent->arity) {
    /* check whether that parent has no children for real */
    assert(!parent->children);
    assert(!parent->first_child);
    assert(!parent->last_child);
    return;
  }
  /* check whether that parent has children for real */
  assert(parent->children);
  assert(parent->first_child);
  assert(parent->last_child);

  /* first child specific checks */
  assert(parent->first_child->sibling_rank == 0);
  assert(parent->first_child == parent->children[0]);
  assert(parent->first_child->prev_sibling == NULL);

  /* last child specific checks */
  assert(parent->last_child->sibling_rank == parent->arity-1);
  assert(parent->last_child == parent->children[parent->arity-1]);
  assert(parent->last_child->next_sibling == NULL);

  if (parent->cpuset) {
    remaining_parent_set = hwloc_bitmap_dup(parent->cpuset);
    for(j=0; j<parent->arity; j++) {
      if (!parent->children[j]->cpuset)
	continue;
      /* check that child cpuset is included in the parent */
      assert(hwloc_bitmap_isincluded(parent->children[j]->cpuset, remaining_parent_set));
#if !defined(NDEBUG)
      /* check that children are correctly ordered (see below), empty ones may be anywhere */
      if (!hwloc_bitmap_iszero(parent->children[j]->cpuset)) {
        int firstchild = hwloc_bitmap_first(parent->children[j]->cpuset);
        int firstparent = hwloc_bitmap_first(remaining_parent_set);
        assert(firstchild == firstparent);
      }
#endif
      /* clear previously used parent cpuset bits so that we actually checked above
       * that children cpusets do not intersect and are ordered properly
       */
      hwloc_bitmap_andnot(remaining_parent_set, remaining_parent_set, parent->children[j]->cpuset);
    }
    assert(hwloc_bitmap_iszero(remaining_parent_set));
    hwloc_bitmap_free(remaining_parent_set);
  }

  /* checks for all children */
  for(j=1; j<parent->arity; j++) {
    assert(parent->children[j]->parent == parent);
    assert(parent->children[j]->sibling_rank == j);
    assert(parent->children[j-1]->next_sibling == parent->children[j]);
    assert(parent->children[j]->prev_sibling == parent->children[j-1]);
  }
}

static void
hwloc__check_children_depth(struct hwloc_topology *topology, struct hwloc_obj *parent)
{
  hwloc_obj_t child = NULL;
  while ((child = hwloc_get_next_child(topology, parent, child)) != NULL) {
    if (child->type == HWLOC_OBJ_BRIDGE)
      assert(child->depth == (unsigned) HWLOC_TYPE_DEPTH_BRIDGE);
    else if (child->type == HWLOC_OBJ_PCI_DEVICE)
      assert(child->depth == (unsigned) HWLOC_TYPE_DEPTH_PCI_DEVICE);
    else if (child->type == HWLOC_OBJ_OS_DEVICE)
      assert(child->depth == (unsigned) HWLOC_TYPE_DEPTH_OS_DEVICE);
    else if (child->type == HWLOC_OBJ_MISC)
      assert(child->depth == (unsigned) -1);
    else if (parent->depth != (unsigned) -1)
      assert(child->depth > parent->depth);
    hwloc__check_children_depth(topology, child);
  }
}

/* check a whole topology structure */
void
hwloc_topology_check(struct hwloc_topology *topology)
{
  struct hwloc_obj *obj;
  hwloc_obj_type_t type;
  unsigned i, j, depth;

  /* check type orders */
  for (type = HWLOC_OBJ_SYSTEM; type < HWLOC_OBJ_TYPE_MAX; type++) {
    assert(hwloc_get_order_type(hwloc_get_type_order(type)) == type);
  }
  for (i = hwloc_get_type_order(HWLOC_OBJ_SYSTEM);
       i <= hwloc_get_type_order(HWLOC_OBJ_CORE); i++) {
    assert(i == hwloc_get_type_order(hwloc_get_order_type(i)));
  }

  /* check that last level is PU */
  assert(hwloc_get_depth_type(topology, hwloc_topology_get_depth(topology)-1) == HWLOC_OBJ_PU);
  /* check that other levels are not PU */
  for(i=1; i<hwloc_topology_get_depth(topology)-1; i++)
    assert(hwloc_get_depth_type(topology, i) != HWLOC_OBJ_PU);

  /* top-level specific checks */
  assert(hwloc_get_nbobjs_by_depth(topology, 0) == 1);
  obj = hwloc_get_root_obj(topology);
  assert(obj);
  assert(!obj->parent);

  depth = hwloc_topology_get_depth(topology);

  /* check each level */
  for(i=0; i<depth; i++) {
    unsigned width = hwloc_get_nbobjs_by_depth(topology, i);
    struct hwloc_obj *prev = NULL;

    /* check each object of the level */
    for(j=0; j<width; j++) {
      obj = hwloc_get_obj_by_depth(topology, i, j);
      /* check that the object is corrected placed horizontally and vertically */
      assert(obj);
      assert(obj->depth == i);
      assert(obj->logical_index == j);
      /* check that all objects in the level have the same type */
      if (prev) {
	assert(hwloc_type_cmp(obj, prev) == HWLOC_TYPE_EQUAL);
	assert(prev->next_cousin == obj);
	assert(obj->prev_cousin == prev);
      }
      if (obj->complete_cpuset) {
        if (obj->cpuset)
          assert(hwloc_bitmap_isincluded(obj->cpuset, obj->complete_cpuset));
        if (obj->online_cpuset)
          assert(hwloc_bitmap_isincluded(obj->online_cpuset, obj->complete_cpuset));
        if (obj->allowed_cpuset)
          assert(hwloc_bitmap_isincluded(obj->allowed_cpuset, obj->complete_cpuset));
      }
      if (obj->complete_nodeset) {
        if (obj->nodeset)
          assert(hwloc_bitmap_isincluded(obj->nodeset, obj->complete_nodeset));
        if (obj->allowed_nodeset)
          assert(hwloc_bitmap_isincluded(obj->allowed_nodeset, obj->complete_nodeset));
      }
      /* check children */
      hwloc__check_children(obj);
      prev = obj;
    }

    /* check first object of the level */
    obj = hwloc_get_obj_by_depth(topology, i, 0);
    assert(obj);
    assert(!obj->prev_cousin);

    /* check type */
    assert(hwloc_get_depth_type(topology, i) == obj->type);
    assert(i == (unsigned) hwloc_get_type_depth(topology, obj->type) ||
           HWLOC_TYPE_DEPTH_MULTIPLE == hwloc_get_type_depth(topology, obj->type));

    /* check last object of the level */
    obj = hwloc_get_obj_by_depth(topology, i, width-1);
    assert(obj);
    assert(!obj->next_cousin);

    /* check last+1 object of the level */
    obj = hwloc_get_obj_by_depth(topology, i, width);
    assert(!obj);
  }

  /* check bottom objects */
  assert(hwloc_get_nbobjs_by_depth(topology, depth-1) > 0);
  for(j=0; j<hwloc_get_nbobjs_by_depth(topology, depth-1); j++) {
    obj = hwloc_get_obj_by_depth(topology, depth-1, j);
    assert(obj);
    /* bottom-level object must always be PU */
    assert(obj->type == HWLOC_OBJ_PU);
  }

  /* check relative depths */
  obj = hwloc_get_root_obj(topology);
  assert(obj->depth == 0);
  hwloc__check_children_depth(topology, obj);
}

const struct hwloc_topology_support *
hwloc_topology_get_support(struct hwloc_topology * topology)
{
  return &topology->support;
}
