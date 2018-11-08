#===================================================
# GNUmakefile for GCC 4.01							
# S.R.K. Branavan									
# Massachusetts Institute of Technology				
#===================================================

#include $(MAKEFILE_HEADER)

include ../GNUmakefile_options

INC_PATH = -I. -I..  -I../utility -I../common -I../common/aicore -I../server -I../server/generator
LIB_PATH = $(group)/tools/lib
DEFS = -DHAVE_CONFIG_H -DLOCALEDIR="\"/usr/local/share/locale\"" -DDEFAULT_DATA_PATH="\".:data:~/.freeciv:/usr/local/share/freeciv\""  

FLAGS = $(DEFS) $(INC_PATH) -Wall -Wpointer-arith -Wcast-align -fsigned-char $(OPT_FLAGS)
CCFLAGS = $(FLAGS) -Wmissing-prototypes -Wmissing-declarations


#								
TARGET = libai.a

OBJS = advdiplomacy.o \
	advdomestic.o \
	advmilitary.o \
	advspace.o \
	aiair.o \
	aicity.o \
	aidata.o \
	aidiplomat.o \
	aiexplorer.o \
	aiferry.o \
	aiguard.o \
	aihand.o \
	aihunt.o \
	ailog.o \
	aiparatrooper.o \
	aisettler.o \
	aitech.o \
	aitools.o \
	aiunit.o


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




