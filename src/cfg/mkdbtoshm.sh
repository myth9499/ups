gcc -o filetodb filetodb.c  -I ../../incl/libxml2/include/libxml2/ -I ../../incl/ -L ../../src/lib/ -lbase -lhash -lsqlite3 -lxml2 -lpack
gcc -o dbtoshm dbtoshm.c  -I ../../incl/libxml2/include/libxml2/ -I ../../incl/ -L ../../src/lib/ -lbase -lhash -lsqlite3 -lxml2 -lpack
