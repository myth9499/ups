static char sqla_program_id[292] = 
{
 172,0,65,69,65,78,65,73,90,65,115,79,84,66,70,101,48,49,49,49,
 49,32,50,32,32,32,32,32,32,32,32,32,8,0,68,69,86,32,32,32,
 32,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,8,0,68,66,50,68,69,77,79,32,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0
};

#include "sqladef.h"

static struct sqla_runtime_info sqla_rtinfo = 
{{'S','Q','L','A','R','T','I','N'}, sizeof(wchar_t), 0, {' ',' ',' ',' '}};


static const short sqlIsLiteral   = SQL_IS_LITERAL;
static const short sqlIsInputHvar = SQL_IS_INPUT_HVAR;


#line 1 "db2demo.sqc"
#include <stdio.h>
#include <unistd.h>
#include <string.h>


/*
EXEC SQL INCLUDE sqlca;
*/

/* SQL Communication Area - SQLCA - structures and constants */
#include "sqlca.h"
struct sqlca sqlca;


#line 5 "db2demo.sqc"


int dbtest1(char *p1,char *p2,char *p3)
{
	
/*
EXEC SQL connect to apsdb;
*/

{
#line 9 "db2demo.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 9 "db2demo.sqc"
  sqlaaloc(2,1,1,0L);
    {
      struct sqla_setdata_list sql_setdlist[1];
#line 9 "db2demo.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 6;
#line 9 "db2demo.sqc"
      sql_setdlist[0].sqldata = (void*)"apsdb";
#line 9 "db2demo.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 9 "db2demo.sqc"
      sqlasetdata(2,0,1,sql_setdlist,0L,0L);
    }
#line 9 "db2demo.sqc"
  sqlacall((unsigned short)29,4,2,0,0L);
#line 9 "db2demo.sqc"
  sqlastop(0L);
}

#line 9 "db2demo.sqc"

	if(SQLCODE!=0)
		return -1;
	else
		return 0;
}
int dbtest2(char *par1,char *par2,char *par3)
{
	
/*
EXEC SQL begin declare section;
*/

#line 17 "db2demo.sqc"

	char	apsno[13];
	
/*
EXEC SQL end declare section;
*/

#line 19 "db2demo.sqc"


	if(SQLCODE!=0)
	{
		printf("connect to apsdb error[%d]",SQLCODE);
		return -1;
	}
	printf("connect to apsdb ok\n");
	
/*
EXEC SQL select apsno into :apsno from t_ibps_txreg where apsno ='000002707701';
*/

{
#line 27 "db2demo.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 27 "db2demo.sqc"
  sqlaaloc(3,1,2,0L);
    {
      struct sqla_setdata_list sql_setdlist[1];
#line 27 "db2demo.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 13;
#line 27 "db2demo.sqc"
      sql_setdlist[0].sqldata = (void*)apsno;
#line 27 "db2demo.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 27 "db2demo.sqc"
      sqlasetdata(3,0,1,sql_setdlist,0L,0L);
    }
#line 27 "db2demo.sqc"
  sqlacall((unsigned short)24,1,0,3,0L);
#line 27 "db2demo.sqc"
  sqlastop(0L);
}

#line 27 "db2demo.sqc"
 
	if(SQLCODE!=0)
	{
		printf("select from apsdb error[%d]",SQLCODE);
		return -1;
	}
	printf("the apsno is [%s]\n",apsno);
	char	msgid[20];
	memset(msgid,0,sizeof(msgid));
	get_var_value("V_A01",20,1,msgid);
	printf("msgid is [%s]\n",msgid);
	char	msgid2[20];
	memset(msgid2,0,sizeof(msgid2));
	get_var_value("V_A02",20,1,msgid2);
	put_var_value("V_A02",20,1,"test upate value");
	printf("msgid2 is [%s]\n",msgid2);
	return  0;
}
int dbtest3(char *par1,char *par2,char *par3)
{
	
/*
EXEC SQL begin declare section;
*/

#line 47 "db2demo.sqc"

	char	apsno[13];
	
/*
EXEC SQL end declare section;
*/

#line 49 "db2demo.sqc"


	
/*
EXEC SQL update t_ibps_txreg  set msgid  = '99999999' where apsno ='000002707701';
*/

{
#line 51 "db2demo.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 51 "db2demo.sqc"
  sqlacall((unsigned short)24,2,0,0,0L);
#line 51 "db2demo.sqc"
  sqlastop(0L);
}

#line 51 "db2demo.sqc"
 
	if(SQLCODE!=0)
	{
		printf("select from apsdb error[%d]",SQLCODE);
		return -1;
	}
	seterr("EEEEEEEE","DB2 OPEN ERROR");
	return 0;
}
int dbtest4(char *par1,char *par2,char *par3)
{
	
/*
EXEC SQL connect reset;
*/

{
#line 62 "db2demo.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 62 "db2demo.sqc"
  sqlacall((unsigned short)29,3,0,0,0L);
#line 62 "db2demo.sqc"
  sqlastop(0L);
}

#line 62 "db2demo.sqc"

}
