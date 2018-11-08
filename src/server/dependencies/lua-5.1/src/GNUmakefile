#===================================================
# GNUmakefile for GCC 4.01							
# S.R.K. Branavan									
# Massachusetts Institute of Technology				
#===================================================


include ../../../GNUmakefile_options


INC_PATH = -I. -I..  -I../.. -I../../.. -I../../../utility #-I./../aicore 
LIB_PATH = $(group)/tools/lib
DEFS = -DHAVE_CONFIG_H 

FLAGS = $(CPU) $(DEFS) $(INC_PATH) -Wall -Wpointer-arith -Wcast-align -fsigned-char $(OPT_FLAGS)
CCFLAGS = $(FLAGS) -Wmissing-prototypes -Wmissing-declarations



#								
TARGET = libdeplua.a

OBJS = lapi.o \
	lauxlib.o \
	lbaselib.o \
	lcode.o \
	ldblib.o \
	ldebug.o \
	ldo.o \
	ldump.o \
	lfunc.o \
	lgc.o \
	linit.o \
	liolib.o \
	llex.o \
	lmathlib.o \
	lmem.o \
	loadlib.o \
	lobject.o \
	lopcodes.o \
	loslib.o \
	lparser.o \
	lstate.o \
	lstring.o \
	lstrlib.o \
	ltable.o \
	ltablib.o \
	ltm.o \
	luac.o \
	lundump.o \
	lvm.o \
	lzio.o \
	print.o


#								
all: $(TARGET)

$(TARGET): $(OBJS)
	ar -cq $@ $^

%.o: %.c
	gcc -c -o $@ $< $(CCFLAGS)

%.o: %.cpp
	g++ -c -o $@ $< $(CCFLAGS)


#								
.PHONY: clean

clean: 
	rm -f *.o $(TARGET)



