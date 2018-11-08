#===================================================
# GNUmakefile for GCC 4.01							
# S.R.K. Branavan									
# Massachusetts Institute of Technology				
#===================================================

#include $(MAKEFILE_HEADER)



INC_PATH = -I../ -I../include -I../../common -I../../utility -I$(group)/tools/include
LIB_PATH = $(group)/tools/lib
CCFLAGS = $(INC_PATH) -Wall -Wpointer-arith -Wcast-align -Wmissing-prototypes -Wmissing-declarations -fsigned-char #-O2 -DNDEBUG



#								
TARGET = libgui-stub.a

OBJS = canvas.o \
		chatline.o \
		citydlg.o \
		cityrep.o \
		colors.o \
		connectdlg.o \
		dialogs.o \
		diplodlg.o \
		finddlg.o \
		gotodlg.o \
		graphics.o \
		gui_main.o \
		helpdlg.o \
		inteldlg.o \
		mapctrl.o \
		mapview.o \
		menu.o \
		messagedlg.o \
		messagewin.o \
		pages.o \
		plrdlg.o \
		ratesdlg.o \
		repodlgs.o \
		spaceshipdlg.o \
		sprite.o \
		themes.o \
		voteinfo_bar.o \
		wldlg.o \
		main.o

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


