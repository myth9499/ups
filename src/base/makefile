libbase.so:base.c pool.c SysLog.c
	gcc -g -o libbase.so base.c pool.c SysLog.c -shared -fPIC  -lpack -lxml2  -I ../../incl/ -I ../../incl/libxml2/include/libxml2  -lpthread -ldl  -L  ../lib  -L ../../incl/libxml2/lib
	mv libbase.so ../lib
