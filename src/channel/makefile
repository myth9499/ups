all:sync_chnl	sync_chnl_r	mqsend	mqrcv_ibps	mqrcv_cnaps
.PHONY:all
objects=sync_chnl.o	sync_chnl_r.o mqsend.o mqrcv_ibps.o mqrcv_cnaps.o
upshome=$(UPSHOME)
upsincl=$(UPSHOME)/incl
upslib=$(UPSHOME)/src/lib
mqincl=/opt/mqm/inc/
mqlib=/opt/mqm/lib64
CFLAGS=-I $(upsincl) -I $(mqincl)
LDFLAGS=-L $(upslib) -L $(mqlib)
LIBS=-lpack	-lhash	-lbase -lpthread -lmqm
sync_chnl:sync_chnl.o
	gcc  -g -o sync_chnl	sync_chnl.o $(CFLAGS) $(LDFLAGS) $(LIBS) 
	@mv	sync_chnl	$(upshome)/bin	
sync_chnl_r:sync_chnl_r.o
	gcc  -g -o sync_chnl_r	sync_chnl_r.o $(CFLAGS) $(LDFLAGS) $(LIBS)
	@mv	sync_chnl_r	$(upshome)/bin	
mqsend:mqsend.o
	gcc -g -o mqsend mqsend.o  $(CFLAGS) $(LDFLAGS) $(LIBS) 
	@mv	mqsend	$(upshome)/bin	
mqrcv_ibps:mqrcv_ibps.o
	gcc -o mqrcv_ibps mqrcv_ibps.o $(CFLAGS) $(LDFLAGS) $(LIBS)
	@mv	mqrcv_ibps	$(upshome)/bin	
mqrcv_cnaps:mqrcv_cnaps.o
	gcc -o mqrcv_cnaps mqrcv_cnaps.o $(CFLAGS) $(LDFLAGS) $(LIBS)
	@mv	mqrcv_cnaps	$(upshome)/bin	
