all:libhash.so
.PHONY:all
objects=hash.o
upsincl=$(UPSHOME)/incl
upslib=$(UPSHOME)/src/lib
CFLAGS=-I $(upsincl) -shared	-fPIC
LDFLAGS=-L $(upslib)
LIBS=-lbase
libhash.so:$(objects)
	    gcc -g -o libhash.so $(objects) $(CFLAGS)	$(LDFLAGS)	$(LIBS)
	    @mv libhash.so  ../lib
