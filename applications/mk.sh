db2 connect to apsdb
db2 prep db2demo.sqc bindfile
gcc -o libtestdb.so -fPIC -shared db2demo.c -I /home/dev/sqllib/include -I /item/ups/incl -L /home/dev/sqllib/lib -L /item/ups/src/lib  -ldb2  -lbase -lhash
db2 bind db2demo.bnd
