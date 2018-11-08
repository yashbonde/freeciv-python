#===================================================
# GNUmakefile for GCC 4.01							
# S.R.K. Branavan									
# Massachusetts Institute of Technology				
#===================================================


include ../../GNUmakefile_options


INC_PATH = -I. -I.. -I../..
LIB_PATH = $(group)/tools/lib
DEFS = -DHAVE_CONFIG_H 

CCFLAGS = $(DEFS) $(INC_PATH) -Wall -Wpointer-arith -Wcast-align -Wmissing-prototypes -Wmissing-declarations $(OPT_FLAGS)


#								
TARGET = libftwl.a

OBJS = be_common_cairo_32.o \
	be_common_pixels.o \
	be_sdl.o \
	be_sdl_pixels_cairo_32.o \
	be_x11_cairo_32.o \
	common_types.o \
	text_renderer.o \
	theme_engine.o \
	widget_button.o \
	widget.o \
	widget_core.o \
	widget_edit.o \
	widget_label.o \
	widget_list.o \
	widget_slider.o \
	widget_timeout.o \
	widget_window.o


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
	$(MAKE) clean
	rm -f *.o $(TARGET)





