#===================================================
# GNUmakefile for GCC 4.01							
# S.R.K. Branavan									
# Massachusetts Institute of Technology				
#===================================================


include ../GNUmakefile_options


INC_PATH = -I. -I..  -I../utility -I./aicore  -I$(group)/tools/include
LIB_PATH = $(group)/tools/lib
DEFS = -DHAVE_CONFIG_H -DLOCALEDIR="\"/usr/local/share/locale\"" -DDEFAULT_DATA_PATH="\".:data:~/.freeciv:/usr/local/share/freeciv\""   

FLAGS = $(CPU) $(DEFS) $(INC_PATH) -Wall -Wpointer-arith -Wcast-align -fsigned-char $(OPT_FLAGS)
CCFLAGS = $(FLAGS) -Wmissing-prototypes -Wmissing-declarations


#								
TARGET = libutility.a

OBJS = astring.o \
	capability.o \
	distribute.o \
	fciconv.o \
	fcintl.o \
	genlist.o \
	hash.o \
	inputfile.o \
	ioz.o \
	iterator.o \
	log.o \
	md5.o \
	mem.o \
	netintf.o \
	pqueue.o \
	rand.o \
	registry.o \
	sbuffer.o \
	shared.o \
	string_vector.o \
	support.o \
	timing.o \
	memory_file_cache.o 


#								
$(TARGET): $(OBJS)
	ar -cq $@ $^

%.o: %.c
	gcc -c -o $@ $< $(CCFLAGS)

%.o: %.cpp
	g++ -c -o $@ $< $(FLAGS)


#								
.PHONY: clean

clean: 
	rm -f *.o $(TARGET)




