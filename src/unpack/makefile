all:libunpack.so
.PHONY:all
objects=unpack_xml.o	pack_xml.o
upsincl=/item/ups/incl
upslib=/item/ups/src/lib
xmlincl=$(upsincl)/libxml2/include/libxml2
xmllib= $(upsincl)/libxml2/lib
CFLAGS=-I $(upsincl) -I $(xmlincl) -shared	-fPIC
LDFLAGS=-L $(upslib)  -L $(xmllib)
LIBS=-lxml2 	-lbase
libunpack.so:$(objects)
	    gcc -g -o libunpack.so $(objects) $(CFLAGS)	$(LDFLAGS)	$(LIBS)
	    @mv libunpack.so  ../lib
