all:libbase.so
.PHONY:all
objects=base.o	pool.o	SysLog.o	getinnerseq.o
upsincl=$(UPSHOME)/incl
upslib=$(UPSHOME)/src/lib
xmlincl=$(upsincl)/libxml2/include/libxml2
xmllib= $(upsincl)/libxml2/lib
CFLAGS=-I $(upsincl) -I $(xmlincl) -shared	-fPIC
LDFLAGS=-L $(upslib)  -L $(xmllib)
LIBS=-lxml2 -lpthread	-ldl
libbase.so:$(objects)
	    gcc -g -o libbase.so $(objects) $(CFLAGS)	$(LDFLAGS)	$(LIBS)
	    @mv libbase.so  ../lib
