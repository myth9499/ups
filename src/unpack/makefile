libunpack.so:unpack_xml.c pack_xml.c
	gcc	-g -o libunpack.so -fPIC -shared  unpack_xml.c pack_xml.c -I  ../../incl/ -I ../../incl/libxml2/include/libxml2/ -L  ../../incl/libxml2/lib/ -L ../lib/  -lxml2 -lbase
	mv libunpack.so  ../lib
