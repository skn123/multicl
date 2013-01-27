# === Basics ===
CC       = gcc
CXX      = g++
LD       = g++
AR       = ar
RANLIB   = ranlib

CPPFLAGS += -I$(SHOC_ROOT)/src/common -I${SHOC_ROOT}/config -I/opt/AMDAPP/include/
CFLAGS   += -g -O2
CXXFLAGS += -g -O2
NVCXXFLAGS += -g -O2
ARFLAGS  = rcv
LDFLAGS  = 
LIBS     = -L$(SHOC_ROOT)/lib  -lrt

USE_MPI         = no
MPICXX          = 

OCL_CPPFLAGS    += -I${SHOC_ROOT}/src/opencl/common
OCL_LIBS        = -lOpenCL

NVCC            = 
CUDA_CXX        = 
CUDA_INC        = -I
CUDA_CPPFLAGS   +=  -I${SHOC_ROOT}/src/cuda/include

USE_CUDA        = no
ifeq ($(USE_CUDA),yes)
CUDA_LIBS       := $(shell  -dryrun bogus.cu 2>&1 | grep LIBRARIES | sed 's/^.*LIBRARIES=//')
else
CUDA_LIBS       =
endif



