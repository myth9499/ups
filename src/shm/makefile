all:initshm
.PHONY:all
objects=initshm.o
upshome=/item/ups/
upsincl=/item/ups/incl
upslib=/item/ups/src/lib
CFLAGS=-I $(upsincl)
LDFLAGS=-L $(upslib)
LIBS=-lbase -lhash -lpack
initshm:initshm.o
	gcc  -g -o initshm	initshm.o $(CFLAGS) $(LDFLAGS) $(LIBS) 
	@mv	initshm	$(upshome)/bin
