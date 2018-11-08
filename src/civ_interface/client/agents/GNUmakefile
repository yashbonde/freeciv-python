#===================================================
# GNUmakefile for GCC 4.01							
# S.R.K. Branavan									
# Massachusetts Institute of Technology				
#===================================================


include ../../GNUmakefile_options


INC_PATH = -I../ -I../.. -I../include -I../../common -I../../common/aicore -I../../utility -I$(group)/tools/include
LIB_PATH = $(group)/tools/lib
DEFS = -DHAVE_CONFIG_H 

FLAGS = $(DEFS) $(INC_PATH) $(GTK_CFLAGS) -Wall -Wpointer-arith -Wcast-align -fsigned-char $(OPT_FLAGS)
CCFLAGS = $(FLAGS) -Wmissing-prototypes -Wmissing-declarations
CXXFLAGS = $(FLAGS)



#								
TARGET = libagents.a

OBJS = agents.o \
	cma_core.o \
	cma_fec.o \
	sha.o

LIBS = 


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


