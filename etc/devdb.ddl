CREATE TABLE  XML2TABLEMAP  (
          MSGTYPE  CHAR(32) ,
          INOUTFLAG  CHAR(2) ,
          VARNAME  CHAR(25) ,
          VARLEN  CHAR(5) ,
          VARVALUE  CHAR(25) ,
          TABLENAME  CHAR(25) ,
          COLNAME  CHAR(25) )



create table t_hvps111(
msgid                       char(36),
credttm                     char(21),
nboftxs                     char(16),
sttlmmtd                    char(5),
endtoendid                  char(36),
txid                        char(36),
prtry                    char(5),
intrbksttlmamt           char(21),
sttlmprty                   char(5),
chrgbr                      char(5),
fmmbid                   char(15),
fid                         char(15),
smmbid                      char(15),
sid                      char(15),
fnm                         char(141),
faddr                       char(71),
payno                       char(36),
pmmbid                      char(36),
pmmnm                       char(141),
rmmbid                      char(36),
rmmnm                       char(141),
rnm                         char(141),
raddr                       char(71),
ywzlbm                      char(6)
)
