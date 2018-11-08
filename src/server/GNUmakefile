#===================================================
# GNUmakefile for GCC 4.01							
# S.R.K. Branavan									
# Massachusetts Institute of Technology				
#===================================================


.PHONY: clean

auto: build install

build:
	cd utility; $(MAKE)
	cd common; $(MAKE)
	cd ai; $(MAKE)
	cd dependencies/toluaxx/src/lib; $(MAKE)
	cd dependencies/lua-5.1/src; $(MAKE)
	cd server; $(MAKE)


clean: 
	cd utility; $(MAKE) clean
	cd common; $(MAKE) clean
	cd ai; $(MAKE) clean
	cd dependencies/toluaxx/src/lib; $(MAKE) clean
	cd dependencies/lua-5.1/src; $(MAKE) clean
	cd server; $(MAKE) clean


install:
	cp server/freeciv-server ../../bin
