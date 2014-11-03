all:loadcfg	filetodb	dbtoshm
.PHONY:all
objects=loadcfg.o	filetodb.o dbtoshm.o 
upshome=/item/ups/
upsincl=/item/ups/incl
upslib=/item/ups/src/lib
sqlite3incl=$(upsincl)/sqlite3/include
sqlite3lib=$(upsincl)/sqlite3/lib
xmlincl=$(upsincl)/libxml2/include/libxml2
xmllib=$(upsincl)/libxml2/lib
CFLAGS=-I $(upsincl) -I $(xmlincl) -I $(sqlite3incl)
LDFLAGS=-L $(upslib) -L $(xmllib)  -L $(sqlite3lib)
LIBS=-lbase	-lhash	-lpack -lsqlite3	-lxml2
loadcfg:loadcfg.o
	gcc  -g -o loadcfg	loadcfg.o $(CFLAGS) $(LDFLAGS) $(LIBS) 
	@mv	loadcfg	$(upshome)/bin
filetodb:filetodb.o
	gcc  -g -o filetodb	filetodb.o $(CFLAGS) $(LDFLAGS) $(LIBS)
	@mv	filetodb	$(upshome)/bin
dbtoshm:dbtoshm.o
	gcc -g -o dbtoshm dbtoshm.o  $(CFLAGS) $(LDFLAGS) $(LIBS) 
	@mv	dbtoshm	$(upshome)/bin
