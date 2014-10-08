#include"ups.h"
#include<mysql/mysql.h>


int xml2table(char	*xmltype)
{
	MYSQL	*mysql;
	MYSQL_RES	*res;
	MYSQL_ROW	row;

	char	query[4096];
	char	sqlstr1[1300];
	char	sqlstr2[1300];
	char	sqlstr3[1300];
	char	tablename[31];

	memset(sqlstr1,0,sizeof(sqlstr1));
	memset(sqlstr2,0,sizeof(sqlstr2));
	memset(sqlstr3,0,sizeof(sqlstr3));
	memset(tablename,0,sizeof(tablename));

	/** 获取是登记往还是来 **/
	char	*value;
	strcpy(query,"select * from xml2tablemap where inoutflag ='1' ");
	int	t,r;

	mysql = mysql_init(NULL);

	if(mysql==NULL)
	{
		SysLog(1,"连接数据库失败\n");
		return -1;
	}

	if(!mysql_real_connect(mysql,"localhost","root","","ibpsdb",0,NULL,0))
	{
		SysLog(1,"连接数据库失败:%s\n",mysql_error(mysql));
		return  -1;
	}
	t = mysql_query(mysql,query);  
	if (t)  
	{  
		SysLog(1,"执行查询[%s]失败:%s\n",query,mysql_error(mysql));  
	}else
	{
		res = mysql_store_result(mysql);	
		r = mysql_num_fields(res);
		while(row = mysql_fetch_row(res))
		{
			if(strlen(tablename)==0)
				strcpy(tablename,row[5]);
			/**报文类型 往来标志 变量名称 变量长度 变量值获取 映射表名 映射字段名 **/
			strcat(sqlstr1,row[6]);
			strcat(sqlstr1,",");

			value = (char	*)malloc(atoi(row[3]));
			if(value==NULL)
			{
				SysLog(1,"申请内存失败[%s]\n",strerror(errno));
				return -1;
			}
			if(row[4][0]=='B')
			{
				SysLog(1,"从变量获取\n");
				memset(value,0,sizeof(atoi(row[3])));
				get_var_value(row[2],atoi(row[3]),1,value);
			}else if(row[4][0]=='S')
			{
				SysLog(1,"从常量获取\n");
				memset(value,0,sizeof(atoi(row[3])));
				strcpy(value,row[4]+1);
			}
			strcat(sqlstr2,"'");
			strcat(sqlstr2,value);
			strcat(sqlstr2,"'");
			strcat(sqlstr2,",");
			free(value);
		}
		sqlstr1[strlen(sqlstr1)-1]='\0';
		sqlstr2[strlen(sqlstr2)-1]='\0';
		sprintf(query,"insert into %s ( %s ) values ( %s ) ",tablename,sqlstr1,sqlstr2);
		SysLog(1,"query is [%s]\n",query);
		t=mysql_query(mysql,query);
		if(t!=0)
		{
			SysLog(1,"执行语句[%s]失败:%s\n",query,mysql_error(mysql));  
		}
		SysLog(1,"执行语句[%s]成功\n",query);  
		mysql_free_result(res);
	}
	mysql_close(mysql);
	mysql_library_end();
}

int	init_var(char *a)
{
	put_var_value("V_NETGRND",1,1,"1");
	put_var_value("V_NETDATE",10,1,"2014-09-20");
	put_var_value("V_BOOK",4,1,"BOOK");
	put_var_value("V_PMNT",4,1,"PMNT");
	put_var_value("V_OTHR",4,1,"OTHR");
	put_var_value("V_IBPS",4,1,"IBPS");
	put_var_value("V_AT01",4,1,"AT01");
	put_var_value("V_STAT",4,1,"0000");
	put_var_value("V_CRDT",4,1,"CRDT");
	return 0;
}
