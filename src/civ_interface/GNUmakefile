#===================================================
# GNUmakefile for GCC 4.01							
# S.R.K. Branavan									
# Massachusetts Institute of Technology				
#===================================================


.PHONY: clean build install

auto: build install

build:
	cd utility; $(MAKE)
	cd common; $(MAKE)
	cd client; $(MAKE)


clean: 
	cd utility; $(MAKE) clean
	cd common; $(MAKE) clean
	cd client; $(MAKE) clean


install:
	cp client/freeciv-cmdline ../../bin
