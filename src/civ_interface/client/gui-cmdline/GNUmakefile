#===================================================
# GNUmakefile for GCC 4.01							
# S.R.K. Branavan									
# Massachusetts Institute of Technology				
#===================================================


include ../../GNUmakefile_options


INC_PATH = -I../ -I../include -I../../common -I../../utility -I$(group)/tools/include
LIB_PATH = $(group)/tools/lib
DEFS = 

FLAGS = $(DEFS) $(INC_PATH) -Wall -Wpointer-arith -Wcast-align -fsigned-char $(OPT_FLAGS)
CCFLAGS = $(FLAGS) -Wmissing-prototypes -Wmissing-declarations
CXXFLAGS = $(FLAGS) -fno-rtti




#								
TARGET = libgui-cmdline.a

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
		wldlg.o 

LIBS = 


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


