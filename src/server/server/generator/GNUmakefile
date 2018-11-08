#===================================================
# GNUmakefile for GCC 4.01							
# S.R.K. Branavan									
# Massachusetts Institute of Technology				
#===================================================


include ../../GNUmakefile_options


INC_PATH = -I../ -I../.. -I../include -I../../common -I../../utility -I$(group)/tools/include
LIB_PATH = $(group)/tools/lib
DEFS = -DHAVE_CONFIG_H 

FLAGS = $(CPU) $(DEFS) $(INC_PATH) -Wall -Wpointer-arith -Wcast-align -fsigned-char $(OPT_FLAGS)
CCFLAGS = $(FLAGS) -Wmissing-prototypes -Wmissing-declarations



#								
TARGET = libgenerator.a

OBJS = height_map.o \
	mapgen.o \
	mapgen_topology.o \
	startpos.o \
	temperature_map.o \
	utilities.o


#								
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



