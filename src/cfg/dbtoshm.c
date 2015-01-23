#include "ups.h"
#include "sqlite3.h"

#include <libxml/tree.h>
#include <libxml/parser.h>

sqlite3 *db=NULL;
char    sql[1024];
sqlite3_stmt    *stmt;
int rc;
int ncolumn = 0;  
const char *column_name;  
int vtype , i; 

int main(int argc,char *argv[])
{
	/** 初始化全局共享内存前，先获取ups根路径 **/
	if(setupshome()==-1)
	{
		printf("设置全局变量upshome错误,请检查UPSHOME环境变量是否设置\n");
		return -1;
	}

	FILE	*fp=NULL;
	char	buffer[1024];
	char	xmlname[256];
	char	cfgfile[256];
	char	xmlns[256];
	char	dbpath[100];

	memset(buffer,0,sizeof(buffer));
	memset(xmlname,0,sizeof(xmlname));
	memset(cfgfile,0,sizeof(cfgfile));
	memset(xmlns,0,sizeof(xmlns));
	memset(dbpath,0,sizeof(dbpath));

	/* connect to sqlite3 **/
	sprintf(dbpath,"%s%s",upshome,"/cfg/db/ups.sqlite");
	rc  =   sqlite3_open(dbpath,&db);
	if(rc)
	{
		fprintf(stderr,"Cannot open db :%s\n",sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}

	printf("sqlite open db ok\n");

	if(load_commmsg_cfg("commmsg")==0)
	{
		printf("FILE [%s] LINE [%d]:加载渠道配置文件[%s]成功\n",__FILE__,__LINE__,"/cfg/channel/chnl.cfg");
	}else
	{
		printf("FILE [%s] LINE [%d]:加载渠道配置文件[%s]失败\n",__FILE__,__LINE__,"/cfg/channel/chnl.cfg");
		return -1;
	}
	if(load_flow_cfg("flow")==0)
	{
		printf("FILE [%s] LINE [%d]:加载流程配置文件[%s]成功\n",__FILE__,__LINE__,"/cfg/flow/flow.cfg");
	}else
	{
		printf("FILE [%s] LINE [%d]:加载流程配置文件[%s]失败\n",__FILE__,__LINE__,"/cfg/flow/flow.cfg");
		return -1;
	}

	/**装载XML配置 **/
	memset(dbpath,0,sizeof(dbpath));
	sprintf(dbpath,"%s%s",upshome,"/cfg/xmlcfg/loadxml.list");
	fp = fopen(dbpath,"r");
	if(fp == NULL)
	{
		printf("FILE [%s] LINE [%d]:加载XML列表文件[%s]失败\n",__FILE__,__LINE__,"/cfg/xmlcfg/loadxml.list");
		return -1;
	}
	while(fgets(buffer,sizeof(buffer),fp)!=NULL)
	{
		if(buffer[0]=='#')
			continue;
		buffer[strlen(buffer)-1]='\0';
		/** 需要增加检查，防止配置错误 **/
		strcpy(xmlname,strtok(buffer,"^"));
		strcpy(cfgfile,strtok(NULL,"^"));
		strcpy(xmlns,strtok(NULL,"^"));
		if(load_xmlcfg(xmlname,"xmlcfg")==0)
		{
			printf("FILE [%s] LINE [%d]:加载报文配置文件[%s]成功\n",__FILE__,__LINE__,cfgfile);
		}else
		{
			printf("FILE [%s] LINE [%d]:加载报文配置文件[%s]失败\n",__FILE__,__LINE__,cfgfile);
			fclose(fp);
			return -1;
		}
		memset(buffer,0,sizeof(buffer));
		memset(xmlname,0,sizeof(xmlname));
		memset(cfgfile,0,sizeof(cfgfile));
		memset(xmlns,0,sizeof(xmlns));
	}
	fclose(fp);
	if(load_tranmap_cfg("tranmap")==0)
	{
		printf("FILE [%s] LINE [%d]:加载交易映射配置文件[%s]成功\n",__FILE__,__LINE__,"/cfg/trancode/tran.cfg");
	}else
	{
		printf("FILE [%s] LINE [%d]:加载交易映射配置文件[%s]失败\n",__FILE__,__LINE__,"/cfg/trancode/tran.cfg");
		return -1;
	}
	if(load_vardef_cfg("vardef")==0)
	{
		printf("FILE [%s] LINE [%d]:加载变量映射配置文件[%s]成功\n",__FILE__,__LINE__,"/cfg/vardef/vardef.cfg");
	}else
	{
		printf("FILE [%s] LINE [%d]:加载变量映射配置文件[%s]失败\n",__FILE__,__LINE__,"/cfg/vardef/vardef.cfg");
		return -1;
	}
	return 0;
}
int load_commmsg_cfg(char *table)
{
	key_t	key;
	int shmid,i=0;
	_commmsg *cmsg=NULL;
	size_t shmsize;


	/** init commmsg **/
	shmsize=MAXCOMMMSG*sizeof(_commmsg);
	if((shmid=getshmid(9,shmsize))==-1)
	{
		SysLog(LOG_SYS_ERR,"获取共享内存失败\n");
		return -1;
	}
	SysLog(LOG_SYS_SHOW,"开始加载配置文件\n");
	cmsg = (_commmsg *)shmat(shmid,NULL,0);
	if(cmsg == NULL)
	{
		SysLog(LOG_SYS_ERR,"链接共享内存失败\n");
		return -1;
	}

	sprintf(sql,"%s %s","select * from ",table);
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);  
	if (rc != SQLITE_OK)  
	{  
		fprintf(stderr, "%s : %s\n", sql, sqlite3_errmsg(db));    
		sqlite3_close(db);    
		return -1;    
	}  

	ncolumn = sqlite3_column_count(stmt);
	while (sqlite3_step(stmt) == SQLITE_ROW)  
	{     
		for(i = 0 ; i < ncolumn ; i++ )  
		{  
			vtype = sqlite3_column_type(stmt , i);  
			column_name = sqlite3_column_name(stmt , i);  
			switch( vtype )  
			{  
				case SQLITE_NULL:  
					fprintf(stdout, "%s: null\n", column_name);     
					break;  
				case SQLITE_INTEGER:  
					fprintf(stdout, "%s: %d\n", column_name,sqlite3_column_int(stmt,i));    
					break;  
				case SQLITE_FLOAT:  
					fprintf(stdout, "%s: %f\n", column_name,sqlite3_column_double(stmt,i));     
					break;  
				case SQLITE_BLOB: /* arguably fall through... */  
					fprintf(stdout, "%s: BLOB\n", column_name);     
					break;  
				case SQLITE_TEXT:   
					fprintf(stdout, "%s: %s\n", column_name,sqlite3_column_text(stmt,i));   
					break;  
				default:  
					fprintf(stdout, "%s: ERROR [%s]\n", column_name, sqlite3_errmsg(db));   
					break;  
			}
			if(i==1)
			{	
				strcpy(cmsg->commname,sqlite3_column_text(stmt,i));
			}
			if(i==2)
			{
				cmsg->len=sqlite3_column_int(stmt,i);
			}	
			if(i==3)
			{
				strcpy(cmsg->commvar,sqlite3_column_text(stmt,i));
			}	
			if(i==4)
			{
				strcpy(cmsg->commmark,sqlite3_column_text(stmt,i));
			}	
		}  
		cmsg++;
	}
	SysLog(LOG_SYS_SHOW,"加载配置文件成功\n");
	shmdt(cmsg);
	return 0;
}

int load_flow_cfg(char *table)
{
	key_t	key;
	int i=0,shmid;
	_flow *flow=NULL;
	size_t shmsize;


	/** init commmsg **/
	shmsize=MAXFLOW*sizeof(_flow);
	if((shmid=getshmid(8,shmsize))==-1)
	{
		SysLog(LOG_SYS_ERR,"获取流程配置共享内存失败\n");
		return -1;
	}
	SysLog(LOG_SYS_SHOW,"开始装载流程配置表[%s]\n",table);
	flow = (_flow *)shmat(shmid,NULL,0);
	if(flow == NULL)
	{
		SysLog(LOG_SYS_ERR,"链接流程配置共享内存失败\n");
		return -1;
	}
	sprintf(sql,"%s %s","select * from ",table);
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);  
	if (rc != SQLITE_OK)  
	{  
		fprintf(stderr, "%s : %s\n", sql, sqlite3_errmsg(db));    
		sqlite3_close(db);    
		return -1;    
	}  

	ncolumn = sqlite3_column_count(stmt);
	while (sqlite3_step(stmt) == SQLITE_ROW)  
	{     
		for(i = 0 ; i < ncolumn ; i++ )  
		{  
			vtype = sqlite3_column_type(stmt , i);  
			column_name = sqlite3_column_name(stmt , i);  
			switch( vtype )  
			{  
				case SQLITE_NULL:  
					fprintf(stdout, "%s: null\n", column_name);     
					break;  
				case SQLITE_INTEGER:  
					fprintf(stdout, "%s: %d\n", column_name,sqlite3_column_int(stmt,i));    
					break;  
				case SQLITE_FLOAT:  
					fprintf(stdout, "%s: %f\n", column_name,sqlite3_column_double(stmt,i));     
					break;  
				case SQLITE_BLOB: /* arguably fall through... */  
					fprintf(stdout, "%s: BLOB\n", column_name);     
					break;  
				case SQLITE_TEXT:   
					fprintf(stdout, "%s: %s\n", column_name,sqlite3_column_text(stmt,i));   
					break;  
				default:  
					fprintf(stdout, "%s: ERROR [%s]\n", column_name, sqlite3_errmsg(db));   
					break;  
			}
			if(i==1)
				strcpy(flow->flowname,sqlite3_column_text(stmt,i));
			if(i==2)
				strcpy(flow->flowmark,sqlite3_column_text(stmt,i));
			if(i==3)
				strcpy(flow->flowso,sqlite3_column_text(stmt,i));
			if(i==4)
				strcpy(flow->flowfunc,sqlite3_column_text(stmt,i));
			if(i==5)
				strcpy(flow->funcpar1,sqlite3_column_text(stmt,i));
			if(i==6)
				strcpy(flow->errflow,sqlite3_column_text(stmt,i));
		}
		flow++;
	}
	shmdt(flow);
	SysLog(LOG_SYS_SHOW,"装载流程配置表[%s]成功\n",table);
	return 0;
}

int load_xmlcfg(char *xmltype,char *table)
{
	int  i = 0;
	_xmlcfg *xmlcfg = NULL;

	int shmid = 0;
	size_t shmsize = MAXXMLCFG*sizeof(_xmlcfg);

	SysLog(LOG_SYS_SHOW,"开始装载XML配置表[%s]\n",table);
	if((shmid = getshmid(6,shmsize))==-1)
	{
		SysLog(LOG_SYS_ERR,"获取XML配置共享内存失败\n");
		return -1;
	}
	if((xmlcfg = shmat(shmid,NULL,0))==(void *)-1)
	{
		SysLog(LOG_SYS_ERR,"链接XML配置共享内存失败\n");
		return -1;
	}
	sprintf(sql,"%s %s","select * from ",table);
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);  
	if (rc != SQLITE_OK)  
	{  
		fprintf(stderr, "%s : %s\n", sql, sqlite3_errmsg(db));    
		sqlite3_close(db);    
		return -1;    
	}  

	ncolumn = sqlite3_column_count(stmt);
	while (sqlite3_step(stmt) == SQLITE_ROW)  
	{     
		for(i = 0 ; i < ncolumn ; i++ )  
		{  
			vtype = sqlite3_column_type(stmt , i);  
			column_name = sqlite3_column_name(stmt , i);  
			switch( vtype )  
			{  
				case SQLITE_NULL:  
					fprintf(stdout, "%s: null\n", column_name);     
					break;  
				case SQLITE_INTEGER:  
					fprintf(stdout, "%s: %d\n", column_name,sqlite3_column_int(stmt,i));    
					break;  
				case SQLITE_FLOAT:  
					fprintf(stdout, "%s: %f\n", column_name,sqlite3_column_double(stmt,i));     
					break;  
				case SQLITE_BLOB: /* arguably fall through... */  
					fprintf(stdout, "%s: BLOB\n", column_name);     
					break;  
				case SQLITE_TEXT:   
					fprintf(stdout, "%s: %s\n", column_name,sqlite3_column_text(stmt,i));   
					break;  
				default:  
					fprintf(stdout, "%s: ERROR [%s]\n", column_name, sqlite3_errmsg(db));   
					break;  
			}
			if(i==1)
				strcpy(xmlcfg->xmlname,sqlite3_column_text(stmt,i));
			if(i==2)
				strcpy(xmlcfg->mark,sqlite3_column_text(stmt,i));
			if(i==3)
				strcpy(xmlcfg->fullpath,sqlite3_column_text(stmt,i));
			if(i==4)
				strcpy(xmlcfg->varname,sqlite3_column_text(stmt,i));
			if(i==5)
				strcpy(xmlcfg->isneed,sqlite3_column_text(stmt,i));
			if(i==6)
				strcpy(xmlcfg->sign,sqlite3_column_text(stmt,i));
			if(i==7)
				strcpy(xmlcfg->loop,sqlite3_column_text(stmt,i));
			if(i==7)
				xmlcfg->depth=sqlite3_column_int(stmt,i);
		}
		xmlcfg++;
	}
	shmdt(xmlcfg);
	SysLog(LOG_SYS_SHOW,"装载XML配置表[%s]成功\n",table);
	return  0;
}
int load_tranmap_cfg(char *table)
{
	int i=0;
	_tranmap *tmap=NULL;
	size_t shmsize;
	int	shmid=0;


	SysLog(LOG_SYS_SHOW,"开始装载交易映射配置表[%s]\n",table);
	/** init commmsg **/
	shmsize=MAXTRANMAP*sizeof(_tranmap);
	if((shmid=getshmid(5,shmsize))==-1)
	{
		SysLog(LOG_SYS_ERR,"获取交易映射配置共享内存失败\n");
		return -1;
	}
	tmap = (_tranmap *)shmat(shmid,NULL,0);
	if(tmap == NULL)
	{
		SysLog(LOG_SYS_ERR,"鏈接交易映射配置共享内存失败\n");
		return -1;
	}
	sprintf(sql,"%s %s","select * from ",table);
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);  
	if (rc != SQLITE_OK)  
	{  
		fprintf(stderr, "%s : %s\n", sql, sqlite3_errmsg(db));    
		sqlite3_close(db);    
		return -1;    
	}  

	ncolumn = sqlite3_column_count(stmt);
	while (sqlite3_step(stmt) == SQLITE_ROW)  
	{     
		for(i = 0 ; i < ncolumn ; i++ )  
		{  
			vtype = sqlite3_column_type(stmt , i);  
			column_name = sqlite3_column_name(stmt , i);  
			switch( vtype )  
			{  
				case SQLITE_NULL:  
					fprintf(stdout, "%s: null\n", column_name);     
					break;  
				case SQLITE_INTEGER:  
					fprintf(stdout, "%s: %d\n", column_name,sqlite3_column_int(stmt,i));    
					break;  
				case SQLITE_FLOAT:  
					fprintf(stdout, "%s: %f\n", column_name,sqlite3_column_double(stmt,i));     
					break;  
				case SQLITE_BLOB: /* arguably fall through... */  
					fprintf(stdout, "%s: BLOB\n", column_name);     
					break;  
				case SQLITE_TEXT:   
					fprintf(stdout, "%s: %s\n", column_name,sqlite3_column_text(stmt,i));   
					break;  
				default:  
					fprintf(stdout, "%s: ERROR [%s]\n", column_name, sqlite3_errmsg(db));   
					break;  
			}
			if(i==1)
			{	
				strcpy(tmap->trancode,sqlite3_column_text(stmt,i));
			}
			if(i==2)
			{
				strcpy(tmap->tranname,sqlite3_column_text(stmt,i));
			}	
			if(i==3)
			{
				strcpy(tmap->tranflow,sqlite3_column_text(stmt,i));
			}	
			if(i==4)
			{
				tmap->timeout=sqlite3_column_int(stmt,i);
			}
		}
		tmap++;
	}
	shmdt(tmap);
	SysLog(LOG_SYS_SHOW,"装载交易映射配置表[%s]成功\n",table);
	return 0;
}
int load_vardef_cfg(char *table)
{
	int i=0,shmid = 0;
	_vardef *vardef=NULL;
	size_t shmsize;


	SysLog(LOG_SYS_SHOW,"开始装载变量定义配置表[%s]\n",table);
	/** init vardef **/
	shmsize=MAXVARDEF*sizeof(_vardef);
	if((shmid=getshmid(4,shmsize))==-1)
	{
		SysLog(LOG_SYS_ERR,"获取交易定义配置共享内存失败\n");
		return -1;
	}
	vardef = (_vardef *)shmat(shmid,NULL,0);
	if(vardef == NULL)
	{
		SysLog(LOG_SYS_ERR,"链接交易定义配置共享内存失败\n");
		return -1;
	}
	sprintf(sql,"%s %s","select * from ",table);
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);  
	if (rc != SQLITE_OK)  
	{  
		fprintf(stderr, "%s : %s\n", sql, sqlite3_errmsg(db));    
		sqlite3_close(db);    
		return -1;    
	}  

	ncolumn = sqlite3_column_count(stmt);
	while (sqlite3_step(stmt) == SQLITE_ROW)  
	{     
		for(i = 0 ; i < ncolumn ; i++ )  
		{  
			vtype = sqlite3_column_type(stmt , i);  
			column_name = sqlite3_column_name(stmt , i);  
			switch( vtype )  
			{  
				case SQLITE_NULL:  
					fprintf(stdout, "%s: null\n", column_name);     
					break;  
				case SQLITE_INTEGER:  
					fprintf(stdout, "%s: %d\n", column_name,sqlite3_column_int(stmt,i));    
					break;  
				case SQLITE_FLOAT:  
					fprintf(stdout, "%s: %f\n", column_name,sqlite3_column_double(stmt,i));     
					break;  
				case SQLITE_BLOB: /* arguably fall through... */  
					fprintf(stdout, "%s: BLOB\n", column_name);     
					break;  
				case SQLITE_TEXT:   
					fprintf(stdout, "%s: %s\n", column_name,sqlite3_column_text(stmt,i));   
					break;  
				default:  
					fprintf(stdout, "%s: ERROR [%s]\n", column_name, sqlite3_errmsg(db));   
					break;  
			}
			if(i==1)
			{	
				strcpy(vardef->varname,sqlite3_column_text(stmt,i));
			}
			if(i==2)
			{
				strcpy(vardef->varmark,sqlite3_column_text(stmt,i));
			}	
			if(i==3)
			{
				strcpy(vardef->vartype,sqlite3_column_text(stmt,i));
			}	
			if(i==4)
			{
				vardef->varlen=sqlite3_column_int(stmt,i);
			}
		}
		vardef++;
	}
	shmdt(vardef);
	SysLog(LOG_SYS_SHOW,"装载变量定义配置表[%s]成功\n",table);
	return 0;
}
