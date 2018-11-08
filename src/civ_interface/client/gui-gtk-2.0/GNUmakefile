#===================================================
# GNUmakefile for GCC 4.01							
# S.R.K. Branavan									
# Massachusetts Institute of Technology				
#===================================================

#include $(MAKEFILE_HEADER)



INC_PATH = -I../ -I../../ -I../include -I../agents -I../../common -I../../common/aicore -I../../utility -I$(group)/tools/include
LIB_PATH = $(group)/tools/lib
DEFS = -DHAVE_CONFIG_H -DNDEBUG
CPU = -mtune=core2 -march=core2

GTK_CFLAGS = -D_REENTRANT -pthread -I/usr/include/gtk-2.0 -I/usr/lib/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/gio-unix-2.0/ -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/directfb -I/usr/include/libpng12 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include  
GTK_LIBS = -pthread -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lpangoft2-1.0 -lgdk_pixbuf-2.0 -lm -lpangocairo-1.0 -lcairo -lgio-2.0 -lpango-1.0 -lfreetype -lfontconfig -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lrt -lglib-2.0  

FLAGS = $(CPU) $(DEFS) $(INC_PATH) $(GTK_CFLAGS) -Wall -Wpointer-arith -Wcast-align -fsigned-char -O3 -pg # -ggdb
CCFLAGS = $(FLAGS) -Wmissing-prototypes -Wmissing-declarations
CXXFLAGS = $(FLAGS)



#								
TARGET = libgui-gtk2.a

OBJS = canvas.o \
	caravan_dialog.o \
	chatline.o \
	choice_dialog.o \
	citydlg.o \
	cityrep.o \
	cma_fe.o \
	colors.o \
	connectdlg.o \
	dialogs.o \
	diplodlg.o \
	diplomat_dialog.o \
	editgui.o \
	editprop.o \
	embedggz.o \
	finddlg.o \
	gamedlgs.o \
	gotodlg.o \
	graphics.o \
	gtkpixcomm.o \
	gui_main.o \
	gui_stuff.o \
	happiness.o \
	helpdlg.o \
	inputdlg.o \
	inteldlg.o \
	mapctrl.o \
	mapview.o \
	menu.o \
	messagedlg.o \
	messagewin.o \
	pages.o \
	plrdlg.o \
	repodlgs.o \
	resources.o \
	spaceshipdlg.o \
	sprite.o \
	theme_dlg.o \
	themes.o \
	tileset_dlg.o \
	voteinfo_bar.o \
	wldlg.o

LIBS = $(GTK_LIBS)


#								
$(TARGET): $(OBJS)
	rm -f $@
	ar -cq $@ $^

%.o: %.c
	gcc -c -o $@ $< $(CCFLAGS)

%.o: %.cpp
	g++ -c -o $@ $< $(CXXFLAGS)



#								
.PHONY: clean

clean: 
	rm -f *.o $(TARGET)


