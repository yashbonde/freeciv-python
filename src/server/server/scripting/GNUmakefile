#===================================================
# GNUmakefile for GCC 4.01							
# S.R.K. Branavan									
# Massachusetts Institute of Technology				
#===================================================


include ../../GNUmakefile_options


INC_PATH = -I../ -I../.. -I../include -I../../common -I../../utility -I../../dependencies/toluaxx/include/ -I../../dependencies/lua-5.1/src/
LIB_PATH = $(group)/tools/lib
DEFS = -DHAVE_CONFIG_H 

FLAGS = $(CPU) $(DEFS) $(INC_PATH) -Wall -Wpointer-arith -Wcast-align -fsigned-char $(OPT_FLAGS)
CCFLAGS = $(FLAGS) -Wmissing-prototypes -Wmissing-declarations



#								
TARGET = libscripting.a

OBJS = api_actions.o \
	api_find.o \
	api_gen.o \
	api_intl.o \
	api_methods.o \
	api_notify.o \
	api_utilities.o \
	script.o \
	script_signal.o 


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



