db2 connect to ibpsdb
db2 prep xml2table.sqc bindfile
gcc -o libtestdb2.so -fPIC -shared xml2table.c -I /home/dev/sqllib/include -I /item/ups/incl -L /home/dev/sqllib/lib -L /item/ups/src/lib  -ldb2  -lbase -lhash
#gcc -o testxml  xml2table.c -I /home/dev/sqllib/include -I /item/ups/incl -L /home/dev/sqllib/lib -L /item/ups/src/lib  -ldb2  -lbase -lhash
db2 bind xml2table.bnd
mv libtestdb2.so ../src/lib
