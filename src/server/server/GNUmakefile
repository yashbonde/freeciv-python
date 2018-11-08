#===================================================
# GNUmakefile for GCC 4.01							
# S.R.K. Branavan									
# Massachusetts Institute of Technology				
#===================================================


include ../GNUmakefile_options


INC_PATH = -I. -I..  -I../utility -I./aicore -I../common -I../common/aicore -I../ai -I./scripting -I./generator -I$(group)/tools/include
LIB_PATH = -L$(group)/tools/lib -L$(group)/tools/lib/i7
DEFS = -DHAVE_CONFIG_H 

FLAGS = $(CPU) $(DEFS) $(INC_PATH) -Wall -Wpointer-arith -Wcast-align -fsigned-char $(OPT_FLAGS)
CCFLAGS = $(FLAGS) -Wmissing-prototypes -Wmissing-declarations
CXXFLAGS = $(FLAGS)


#								
TARGET = freeciv-server

OBJS = aiiface.o \
	auth.o \
	barbarian.o \
	cityhand.o \
	citytools.o \
	cityturn.o \
	civserver.o \
	commands.o \
	connecthand.o \
	console.o \
	diplhand.o \
	diplomats.o \
	edithand.o \
	gamehand.o \
	ggzserver.o \
	gotohand.o \
	handchat.o \
	hand_gen.o \
	maphand.o \
	meta.o \
	notify.o \
	plrhand.o \
	report.o \
	ruleset.o \
	sanitycheck.o \
	savegame.o \
	score.o \
	sernet.o \
	settings.o \
	settlers.o \
	spacerace.o \
	srv_main.o \
	stdinhand.o \
	techtools.o \
	unithand.o \
	unittools.o \
	voting.o \
	timer.o

LIBS = -lbz2 -lz generator/libgenerator.a scripting/libscripting.a ../ai/libai.a ../common/aicore/libaicore.a ../common/libcommon.a ../dependencies/lua-5.1/src/libdeplua.a ../dependencies/toluaxx/src/lib/libtoluaxx.a ../utility/libutility.a -lreadline -lnlp_string_intel64 -lnlp_filesystem_intel64 -lnlp_time_intel64

#								
all: deps $(TARGET)

$(TARGET): $(OBJS) 
	g++ -o $@ $^ $(LIB_PATH) $(LIBS) $(CXXFLAGS)

%.o: %.c
	gcc -c -o $@ $< $(CCFLAGS)

%.o: %.cpp
	g++ -c -o $@ $< $(CCFLAGS)

deps:
	cd generator; $(MAKE)
	cd scripting; $(MAKE)


#								
.PHONY: clean

clean: 
	cd generator; $(MAKE) clean
	cd scripting; $(MAKE) clean
	rm -f *.o $(TARGET)



