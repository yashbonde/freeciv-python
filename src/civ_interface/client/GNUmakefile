#===================================================
# GNUmakefile for GCC 4.01							
# S.R.K. Branavan									
# Massachusetts Institute of Technology				
#===================================================


include ../GNUmakefile_options


INC_PATH = -I./ -I../ -I./include -I./agents -I../common -I../common/aicore -I../utility -I$(group)/tools/include -I../../server/server -I../../server/ai
LIB_PATH = $(group)/tools/lib
DEFS = -DHAVE_CONFIG_H 

GTK_CFLAGS = -D_REENTRANT -pthread -I/usr/include/gtk-2.0 -I/usr/lib/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/gio-unix-2.0/ -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/directfb -I/usr/include/libpng12 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include  
GTK_LIBS = -pthread -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lpangoft2-1.0 -lgdk_pixbuf-2.0 -lm -lpangocairo-1.0 -lcairo -lgio-2.0 -lpango-1.0 -lfreetype -lfontconfig -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lrt -lglib-2.0  

FLAGS = $(DEFS) $(INC_PATH) $(GTK_CFLAGS) -Wall -Wpointer-arith -Wcast-align -fsigned-char -fno-rtti $(OPT_FLAGS)
CCFLAGS = $(FLAGS) -Wmissing-prototypes -Wmissing-declarations
CXXFLAGS = $(FLAGS)



#								
TARGET_GTK = freeciv-gtk2
TARGET_CMD = freeciv-cmdline

OBJS = attribute.o \
	audio.o \
	audio_none.o \
	chatline_common.o \
	citydlg_common.o \
	cityrepdata.o \
	client_main.o \
	climap.o \
	climisc.o \
	clinet.o \
	colors_common.o \
	connectdlg_common.o \
	control.o \
	dummy.o \
	editor.o \
	ggzclient.o \
	global_worklist.o \
	goto.o \
	helpdata.o \
	mapctrl_common.o \
	mapview_common.o \
	messagewin_common.o \
	options.o \
	overview_common.o \
	packhand.o \
	packhand_gen.o \
	plrdlg_common.o \
	repodlgs_common.o \
	reqtree.o \
	servers.o \
	text.o \
	themes_common.o \
	tilespec.o \
	voteinfo.o \
	message.o \
	process_human_cmds.o \
	process_machine_cmds.o \
	callbacks.o \
	main.o \
	#savegame.o 


#LIB_DEPS = gui-cmdline/libgui-cmdline.a ../common/libcommon.a ../common/aicore/libaicore.a ../utility/libutility.a agents/libagents.a -lbz2 -lz 
#LIB_DEPS = gui-cmdline/libgui-cmdline.a gui-gtk-2.0/libgui-gtk2.a ../common/libcommon.a ../common/aicore/libaicore.a ../utility/libutility.a agents/libagents.a -lbz2 -lz 

LIBS_COMMON = -lbz2 -lz -lnlp_socket_intel64 -lnlp_string_intel64 -lnlp_filesystem_intel64 -lnlp_time_intel64 -lpthread
LIBS_CMD = gui-cmdline/libgui-cmdline.a ../common/libcommon.a ../common/aicore/libaicore.a ../utility/libutility.a agents/libagents.a
LIBS_GTK = gui-gtk-2.0/libgui-gtk2.a ../common/libcommon.a ../common/aicore/libaicore.a ../utility/libutility.a agents/libagents.a


#								
all: deps $(TARGET_CMD)

$(TARGET_GTK): $(OBJS) $(LIBS_GTK)
	g++ -o $@ $^ -L$(LIB_PATH) $(LIBS_GTK) $(GTK_LIBS) $(LIBS_COMMON) $(CXXFLAGS)

$(TARGET_CMD): $(OBJS) net_callback.o $(LIBS_CMD)
	g++ -o $@ $^ -L$(LIB_PATH) $(LIBS_CMD) $(LIBS_COMMON) $(CXXFLAGS)

%.o: %.c
	gcc -c -o $@ $< $(CCFLAGS)

%.o: %.cpp
	g++ -c -o $@ $< $(CXXFLAGS)

deps:
	cd agents; $(MAKE)
	cd gui-cmdline; $(MAKE)
	# cd gui-gtk-2.0; $(MAKE)


#								
.PHONY: clean

clean: 
	cd agents; $(MAKE) clean
	cd gui-cmdline; $(MAKE) clean
	cd gui-gtk-2.0; $(MAKE) clean
	rm -f *.o $(TARGET)


