all:stopups	startups	monitor	monitor_timeout
.PHONY:all
objects=stopups.o	startups.o monitor.o monitor_timeout.o
upshome=/item/ups/
upsincl=/item/ups/incl
upslib=/item/ups/src/lib
CFLAGS=-I $(upsincl)
LDFLAGS=-L $(upslib)
LIBS=-lpack	-lhash	-lbase -lpthread
stopups:stopups.o
	gcc  -g -o stopups	stopups.o $(CFLAGS) $(LDFLAGS) $(LIBS) 
	@mv	stopups	$(upshome)/bin
startups:startups.o
	gcc  -g -o startups	startups.o $(CFLAGS) $(LDFLAGS) $(LIBS)
	@mv	startups	$(upshome)/bin
monitor:monitor.o
	gcc -g -o monitor monitor.o  $(CFLAGS) $(LDFLAGS) $(LIBS) 
	@mv	monitor	$(upshome)/bin
monitor_timeout:monitor_timeout.o
	gcc -o monitor_timeout monitor_timeout.o $(CFLAGS) $(LDFLAGS) $(LIBS)
	@mv	monitor_timeout	$(upshome)/bin
