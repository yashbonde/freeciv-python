#===================================================
# GNUmakefile for GCC 4.01							
# S.R.K. Branavan									
# Massachusetts Institute of Technology				
#===================================================

INC_PATH = ../../../include

# build commands -----------------------------------
.PHONY : auto install

auto: install

install:
	install -d $(INC_PATH)
	install -m0644 hash_map $(INC_PATH)
	install -m0644 nlp_macros.h $(INC_PATH)

