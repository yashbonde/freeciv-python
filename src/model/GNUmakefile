#===================================================
# GNUmakefile for GCC 4.01							
# S.R.K. Branavan									
# Massachusetts Institute of Technology				
#===================================================


#OPT_FLAGS=
include $(MAKEFILE_HEADER)

ENABLE_SOURCE_DUMP = 1

# build targets  -----------------------------------
test: clean build

ARCH_FLAGS = -mfpmath=sse -msse -msse2 -msse3 -mssse3 -msse4 -msse4.1 -msse4.2

PERF_FLAGS = ${ARCH_FLAGS} \
			 -DHAVE_INLINE \
			 -fsee \
			 -fgcse-lm -fgcse-sm -fgcse-las \
			 -ftree-loop-linear \
			 -ftree-loop-im \
			 -funswitch-loops \
			 -ftracer \
			 -fomit-frame-pointer \
			 #-ffast-math

CUSTOM_FLAGS = -Wno-deprecated \
			   -fopenmp \
			   ${PERF_FLAGS} \
			   -DNDEBUG \
			   #-ggdb \
			   -pg 

TARGET = montecarlo_v7.6
OBJS = Text.o \
	   FeatureSpecialization.o \
	   Feature.o \
	   Probability.o \
	   OperatorWords.o \
	   HiddenStateVar.o \
	   SentenceRelevance.o \
	   LogLinearModel.o \
	   LinearApprox.o \
	   Interface.o \
	   SearchTree.o \
	   Play.o \
	   Montecarlo.o \
	   Game.o \
	   SourceDump.o \
	   main.o 

LIBS = -lgsl_intel64 \
	   -lgslcblas_intel64 \
	   -lnlp_string_* \
	   -lnlp_config_* \
	   -lnlp_time_* \
	   -lnlp_socket_* \
	   -lnlp_filesystem_* \
	   -lnlp_distr_* \
	   -lgomp \
	   -lpthread


# OBJECTS_TO_CLEAN = $(OBJS) $(TARGET_NAME)

include $(MAKEFILE_TRAILER)

install:
	cp $(EXE_DIR)/$(RSLVD_TARGET) ~/projects/planning/playciv/bin
