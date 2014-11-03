all:libhash.so
.PHONY:all
objects=hash.o
upsincl=/item/ups/incl
upslib=/item/ups/src/lib
CFLAGS=-I $(upsincl) -shared	-fPIC
LDFLAGS=-L $(upslib)
LIBS=-lbase
libhash.so:$(objects)
	    gcc -g -o libhash.so $(objects) $(CFLAGS)	$(LDFLAGS)	$(LIBS)
	    @mv libhash.so  ../lib
