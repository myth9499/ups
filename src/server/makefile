all:server
.PHONY:all
objects=server.o
upshome=$(UPSHOME)
upsincl=$(UPSHOME)/incl
upslib=$(UPSHOME)/src/lib
CFLAGS=-I $(upsincl)
LDFLAGS=-L $(upslib)
LIBS=-lpack	-lhash	-lbase -lpthread
server:server.o
	gcc  -g -o server	server.o $(CFLAGS) $(LDFLAGS) $(LIBS) 
	@mv	server	$(upshome)/bin
