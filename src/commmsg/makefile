all:libpack.so
.PHONY:all
objects=unpack.o	chnlcom.o	unpack_define.o
upsincl=/item/ups/incl
upslib=/item/ups/src/lib
CFLAGS=-I $(upsincl) -shared	-fPIC
LDFLAGS=-L $(upslib)
LIBS=-lhash	-lbase
libpack.so:$(objects)
	    gcc -g -o libpack.so $(objects)  $(CFLAGS)	$(LDFLAGS)	$(LIBS)
	    @mv libpack.so  ../lib
