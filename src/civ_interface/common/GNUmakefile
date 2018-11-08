#===================================================
# GNUmakefile for GCC 4.01							
# S.R.K. Branavan									
# Massachusetts Institute of Technology				
#===================================================


include ../GNUmakefile_options


INC_PATH = -I. -I..  -I../utility -I./aicore 
LIB_PATH = $(group)/tools/lib
DEFS = -DHAVE_CONFIG_H 

FLAGS = $(DEFS) $(INC_PATH) -Wall -Wpointer-arith -Wcast-align -fsigned-char $(OPT_FLAGS)
CCFLAGS = $(FLAGS) -Wmissing-prototypes -Wmissing-declarations



#								
TARGET = libcommon.a

OBJS = ai.o \
	base.o \
	borders.o \
	capstr.o \
	city.o \
	combat.o \
	connection.o \
	dataio.o \
	diptreaty.o \
	effects.o \
	events.o \
	featured_text.o \
	game.o \
	government.o \
	idex.o \
	improvement.o \
	map.o \
	movement.o \
	nation.o \
	packets.o \
	packets_gen.o \
	player.o \
	requirements.o \
	spaceship.o \
	specialist.o \
	team.o \
	tech.o \
	terrain.o \
	tile.o \
	unit.o \
	unitlist.o \
	unittype.o \
	version.o \
	vision.o \
	worklist.o


#								
all: deps $(TARGET)

$(TARGET): $(OBJS)
	ar -cq $@ $^

%.o: %.c
	gcc -c -o $@ $< $(CCFLAGS)

%.o: %.cpp
	g++ -c -o $@ $< $(CCFLAGS)

deps:
	cd aicore; $(MAKE)


#								
.PHONY: clean

clean: 
	cd aicore; $(MAKE) clean
	rm -f *.o $(TARGET)



