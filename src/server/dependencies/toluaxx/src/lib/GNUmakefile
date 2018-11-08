#===================================================
# GNUmakefile for GCC 4.01							
# S.R.K. Branavan									
# Massachusetts Institute of Technology				
#===================================================


include ../../../../GNUmakefile_options


INC_PATH = -I. -I..  -I../.. -I../../.. -I../../../.. -I../../../../utility -I../../include -I../../../lua-5.1/src/
LIB_PATH = $(group)/tools/lib
DEFS = -DHAVE_CONFIG_H 

FLAGS = $(CPU) $(DEFS) $(INC_PATH) -Wall -Wpointer-arith -Wcast-align -fsigned-char $(OPT_FLAGS)
CCFLAGS = $(FLAGS) -Wmissing-prototypes -Wmissing-declarations



#								
TARGET = libtoluaxx.a

OBJS = tolua_event.o \
	tolua_is.o \
	tolua_map.o \
	tolua_push.o \
	tolua_to.o


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



