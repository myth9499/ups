#include "ups.h"
#include "sqlite3.h"
#include <libxml/tree.h>
#include <libxml/parser.h>

sqlite3	*db=NULL;
char	sql[1024];
sqlite3_stmt	*stmt;
int	rc;

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
	rc	=	sqlite3_open(dbpath,&db);
	if(rc)
	{
		fprintf(stderr,"Cannot open db :%s\n",sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}

	printf("sqlite open db ok\n");
	memset(dbpath,0,sizeof(dbpath));
	sprintf(dbpath,"%s%s",upshome,"/cfg/channel/chnl.cfg");
	if(load_commmsg_cfg(dbpath)==0)
	{
		printf("FILE [%s] LINE [%d]:加载渠道配置文件[%s]成功\n",__FILE__,__LINE__,dbpath);
	}else
	{
		printf("FILE [%s] LINE [%d]:加载渠道配置文件[%s]失败\n",__FILE__,__LINE__,dbpath);
		return -1;
	}

	memset(dbpath,0,sizeof(dbpath));
	sprintf(dbpath,"%s%s",upshome,"/cfg/flow/flow.cfg");
	if(load_flow_cfg(dbpath)==0)
	{
		printf("FILE [%s] LINE [%d]:加载流程配置文件[%s]成功\n",__FILE__,__LINE__,dbpath);
	}else
	{
		printf("FILE [%s] LINE [%d]:加载流程配置文件[%s]失败\n",__FILE__,__LINE__,dbpath);
		return -1;
	}

	/**装载XML配置 **/
	memset(sql,0,sizeof(sql));
	/** 先删除 **/
	strcpy(sql,"delete from xmlcfg ");

	rc = sqlite3_exec(db,sql,NULL,NULL,NULL);
	if(rc!=SQLITE_OK)
	{
		fprintf(stderr,"Cannot prepare sql\n");
		sqlite3_close(db);
		exit(1);
	}

	memset(dbpath,0,sizeof(dbpath));
	sprintf(dbpath,"%s%s",upshome,"/cfg/xmlcfg/loadxml.list");
	fp = fopen(dbpath,"r");
	if(fp == NULL)
	{
		printf("FILE [%s] LINE [%d]:加载XML列表文件[%s]失败\n",__FILE__,__LINE__,dbpath);
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
		if(load_xmlcfg(xmlname,cfgfile)==0)
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

	memset(sql,0,sizeof(sql));
	/** 先删除 **/
	strcpy(sql,"delete from tranmap ");

	rc = sqlite3_exec(db,sql,NULL,NULL,NULL);
	if(rc!=SQLITE_OK)
	{
		fprintf(stderr,"Cannot prepare sql\n");
		sqlite3_close(db);
		exit(1);
	}

	memset(dbpath,0,sizeof(dbpath));
	sprintf(dbpath,"%s%s",upshome,"/cfg/trancode/tran.cfg");
	if(load_tranmap_cfg(dbpath)==0)
	{
		printf("FILE [%s] LINE [%d]:加载交易映射配置文件[%s]成功\n",__FILE__,__LINE__,dbpath);
	}else
	{
		printf("FILE [%s] LINE [%d]:加载交易映射配置文件[%s]失败\n",__FILE__,__LINE__,dbpath);
		return -1;
	}
	memset(sql,0,sizeof(sql));
	/** 先删除 **/
	strcpy(sql,"delete from vardef ");

	rc = sqlite3_exec(db,sql,NULL,NULL,NULL);
	if(rc!=SQLITE_OK)
	{
		fprintf(stderr,"Cannot prepare sql\n");
		sqlite3_close(db);
		exit(1);
	}
	memset(dbpath,0,sizeof(dbpath));
	sprintf(dbpath,"%s%s",upshome,"/cfg/vardef/vardef.cfg");
	if(load_vardef_cfg(dbpath)==0)
	{
		printf("FILE [%s] LINE [%d]:加载变量映射配置文件[%s]成功\n",__FILE__,__LINE__,dbpath);
	}else
	{
		printf("FILE [%s] LINE [%d]:加载变量映射配置文件[%s]失败\n",__FILE__,__LINE__,dbpath);
		return -1;
	}

	sqlite3_close(db);
	return 0;
}

int load_commmsg_cfg(char *filename)
{

	char	*zErrMsg = 0;

	memset(sql,0,sizeof(sql));

	/** 先删除 **/

	strcpy(sql,"delete from commmsg ");

	rc = sqlite3_exec(db,sql,NULL,NULL,NULL);
	if(rc!=SQLITE_OK)
	{
		fprintf(stderr,"Cannot prepare sql \n");
		sqlite3_close(db);
		exit(1);
	}
	memset(sql,0,sizeof(sql));
	strcpy(sql,"insert into commmsg values (NULL,?,?,?,?);");

	rc = sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,NULL);
	if(rc!=SQLITE_OK)
	{
		fprintf(stderr,"Cannot prepare sql :%s\n",sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}

	key_t	key;
	char buff[1200];
	char cname[60];
	_commmsg	cmsg;
	char *tmpbuf = NULL;
	FILE *fp=NULL;


	memset(buff,0,sizeof(buff));
	memset(cname,0,sizeof(cname));

	fp = fopen(filename,"r");
	if(fp==NULL)
	{
		perror("file open error");
		return -1;
	}
	while(fgets(buff,sizeof(buff),fp)!=NULL)
	{
		buff[strlen(buff)-1]='\0';
		if(buff[0]=='*')
		{
			strcpy(cname,buff+1);
			strcpy(cmsg.commname,cname);
			continue;
		}
		if(buff[0]=='#')
		{
			//cmsg++;
			continue;
		}
		if(buff[0]=='@')
		{
			memset(cname,0,sizeof(cname));
			strcpy(cmsg.commname,"over");
			cmsg.len=0;
			strcpy(cmsg.commvar," ");
			strcpy(cmsg.commmark," ");

			sqlite3_bind_text(stmt,1,cmsg.commname,strlen(cmsg.commname),NULL);
			sqlite3_bind_int(stmt,2,cmsg.len);
			sqlite3_bind_text(stmt,3,cmsg.commvar,strlen(cmsg.commvar),NULL);
			sqlite3_bind_text(stmt,4,cmsg.commmark,strlen(cmsg.commmark),NULL);

			rc = sqlite3_step(stmt);
			if(rc!=SQLITE_DONE)
			{
				fprintf(stderr,"Cannot setp sql :%s\n",sqlite3_errmsg(db));
				sqlite3_finalize(stmt);
				fclose(fp);
				exit(1);
			}
			sqlite3_reset(stmt);
			continue;
		}
		/** need if else **/

		strcpy(cmsg.commname,cname);
		strcpy(cmsg.commmark,strtok(buff,"^"));
		cmsg.len=atoi(strtok(NULL,"^"));
		strcpy(cmsg.commvar,strtok(NULL,"^"));

        sqlite3_bind_text(stmt,1,cmsg.commname,strlen(cmsg.commname),NULL);
        sqlite3_bind_int(stmt,2,cmsg.len);
        sqlite3_bind_text(stmt,3,cmsg.commvar,strlen(cmsg.commvar),NULL);
        sqlite3_bind_text(stmt,4,cmsg.commmark,strlen(cmsg.commmark),NULL);

		rc = sqlite3_step(stmt);
		if(rc!=SQLITE_DONE)
		{
			fprintf(stderr,"Cannot setp sql :%s\n",sqlite3_errmsg(db));
			sqlite3_finalize(stmt);
			fclose(fp);
			exit(1);
		}
		sqlite3_reset(stmt);
	}
	fclose(fp);
	sqlite3_finalize(stmt);
	printf("load ok\n");
	return 0;
}

int load_flow_cfg(char *filename)
{

	int i=0;
	char buff[2000];
	char flowname[60];
	_flow flow;
	char *tmpbuf = NULL;

	FILE *fp=NULL;
	char	*zErrMsg = 0;


	memset(sql,0,sizeof(sql));
	/** 先删除 **/

	strcpy(sql,"delete from flow ");

	rc = sqlite3_exec(db,sql,NULL,NULL,NULL);
	if(rc!=SQLITE_OK)
	{
		fprintf(stderr,"Cannot prepare sql\n");
		sqlite3_close(db);
		exit(1);
	}

	memset(sql,0,sizeof(sql));
	strcpy(sql,"insert into flow values (NULL,?,?,?,?,?,?);");

	rc = sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,NULL);
	if(rc!=SQLITE_OK)
	{
		fprintf(stderr,"Cannot prepare sql :%s\n",sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}

	memset(buff,0,sizeof(buff));
	memset(flowname,0,sizeof(flowname));
	/** init commmsg **/
	fp = fopen(filename,"r");
	if(fp==NULL)
	{
		perror("file open error");
		return -1;
	}
	while(fgets(buff,sizeof(buff),fp)!=NULL)
	{
		buff[strlen(buff)-1]='\0';
		if(buff[0]=='*')
		{
			strcpy(flowname,buff+1);
			strcpy(flow.flowname,flowname);
			memset(buff,0,sizeof(buff));
			continue;
		}
		if(buff[0]=='#')
		{
		//	flow++;
			memset(buff,0,sizeof(buff));
			continue;
		}
		if(buff[0]=='@')
		{
			memset(flowname,0,sizeof(flowname));
			strcpy(flow.flowname,"over");
			strcpy(flow.flowmark," ");
			strcpy(flow.flowso," ");
			strcpy(flow.flowfunc," ");
			strcpy(flow.funcpar1," ");
			sqlite3_bind_text(stmt,1,flow.flowname,strlen(flow.flowname),NULL);
			sqlite3_bind_text(stmt,2,flow.flowmark,strlen(flow.flowmark),NULL);
			sqlite3_bind_text(stmt,3,flow.flowso,strlen(flow.flowso),NULL);
			sqlite3_bind_text(stmt,4,flow.flowfunc,strlen(flow.flowfunc),NULL);
			sqlite3_bind_text(stmt,5,flow.funcpar1,strlen(flow.funcpar1),NULL);
			sqlite3_bind_text(stmt,6,flow.errflow,strlen(flow.errflow),NULL);

			rc = sqlite3_step(stmt);
			if(rc!=SQLITE_DONE)
			{
				fprintf(stderr,"Cannot setp sql :%s\n",sqlite3_errmsg(db));
				sqlite3_finalize(stmt);
				fclose(fp);
				exit(1);
			}
			sqlite3_reset(stmt);
			continue;
		}
		/** need if else **/
		strcpy(flow.flowname,flowname);
		strcpy(flow.flowmark,strtok(buff,"^"));
		strcpy(flow.flowso,strtok(NULL,"^"));
		strcpy(flow.flowfunc,strtok(NULL,"^"));
		strcpy(flow.funcpar1,strtok(NULL,"^"));
		strcpy(flow.errflow,strtok(NULL,"^"));
		sqlite3_bind_text(stmt,1,flow.flowname,strlen(flow.flowname),NULL);
		sqlite3_bind_text(stmt,2,flow.flowmark,strlen(flow.flowmark),NULL);
		sqlite3_bind_text(stmt,3,flow.flowso,strlen(flow.flowso),NULL);
		sqlite3_bind_text(stmt,4,flow.flowfunc,strlen(flow.flowfunc),NULL);
		sqlite3_bind_text(stmt,5,flow.funcpar1,strlen(flow.funcpar1),NULL);
		sqlite3_bind_text(stmt,6,flow.errflow,strlen(flow.errflow),NULL);

		rc = sqlite3_step(stmt);
		if(rc!=SQLITE_DONE)
		{
			fprintf(stderr,"Cannot setp sql :%s\n",sqlite3_errmsg(db));
			sqlite3_finalize(stmt);
			fclose(fp);
			exit(1);
		}
		sqlite3_reset(stmt);
		memset(buff,0,sizeof(buff));
	}
	fclose(fp);
	printf("load ok\n");
	return 0;
}

int load_xmlcfg(char *xmltype,char *filename)
{
	xmlDocPtr doc;
	xmlNodePtr	curNode;

	xmlKeepBlanksDefault(0);
	doc = xmlReadFile(filename,"UTF-8",XML_PARSE_RECOVER);
	if(doc == NULL)
	{
		printf("parse file error\n");
		return -1;
	}

	curNode = xmlDocGetRootElement(doc);
	if(curNode == NULL)
	{
		printf("get root elemenet error\n");
		xmlFreeDoc(doc);
		return -1;
	}
	insertcfg(curNode,xmltype);
	xmlFreeDoc(doc);
	return 0;
}
int insertcfg(xmlNodePtr cur,char *xmltype)
{
	xmlChar	*szKey;
	xmlNodePtr curNode ;
	xmlAttr	*attr;
	curNode = cur;
	int  i = 0,iret = 0;
	char	path[256];
	_xmlcfg	xmlcfg;
	memset(path,0,sizeof(path));

	memset(sql,0,sizeof(sql));
	strcpy(sql,"insert into xmlcfg values (NULL,?,?,?,?,?,?,?,?);");

	rc = sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,NULL);
	if(rc!=SQLITE_OK)
	{
		fprintf(stderr,"Cannot prepare sql :%s\n",sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}


	while(curNode!=NULL)
	{
		if(curNode->type == XML_ELEMENT_NODE)
		{
			printf("ELement name [%s]\n",curNode->name);
		}else if(curNode->type == XML_TEXT_NODE)
		{
			szKey = xmlNodeGetContent(curNode);
			printf("[%s]---%s---\n",curNode->name,szKey);
			iret = getNodePath(path,curNode);
			printf("path is [%s]\n",path);
			attr = curNode->parent->properties;
					strcpy(xmlcfg.xmlname,xmltype);
					strcpy(xmlcfg.mark,curNode->parent->name);
					strcpy(xmlcfg.fullpath,path);
					strcpy(xmlcfg.varname,szKey);
					printf("FILE[%s] LINE[%d] 放入变量[%s]\n",__FILE__,__LINE__,xmlcfg.varname);
					if(attr==NULL)
					{
						xmlcfg.depth = -1;
						//break;
					} else
					{
						while(attr)
						{
							printf("FILE[%s] LINE[%d] attr name[%s] attr value[%s]\n",__FILE__,__LINE__,attr->name,attr->children->content);
							if(!strcmp(attr->name,"seq"))
							{
								xmlcfg.depth = atoi(attr->children->content);
							//	break;
							}
							else
							{
								xmlcfg.depth = -1;
							//	break;
							}
							attr = attr->next;
						}
					}
					sqlite3_bind_text(stmt,1,xmlcfg.xmlname,strlen(xmlcfg.xmlname),NULL);
					sqlite3_bind_text(stmt,2,xmlcfg.mark,strlen(xmlcfg.mark),NULL);
					sqlite3_bind_text(stmt,3,xmlcfg.fullpath,strlen(xmlcfg.fullpath),NULL);
					sqlite3_bind_text(stmt,4,xmlcfg.varname,strlen(xmlcfg.varname),NULL);
					sqlite3_bind_text(stmt,5,xmlcfg.isneed,strlen(xmlcfg.isneed),NULL);
					sqlite3_bind_text(stmt,6,xmlcfg.sign,strlen(xmlcfg.sign),NULL);
					sqlite3_bind_text(stmt,7,xmlcfg.loop,strlen(xmlcfg.loop),NULL);
					sqlite3_bind_int(stmt,8,xmlcfg.depth);
					printf("FILE[%s] LINE[%d] 绑定成功，准备提交\n",__FILE__,__LINE__);
					rc = sqlite3_step(stmt);
					if(rc!=SQLITE_DONE)
					{
						fprintf(stderr,"Cannot setp sql :%s\n",sqlite3_errmsg(db));
						sqlite3_finalize(stmt);
						exit(1);
					}
					sqlite3_reset(stmt);
				xmlFree(szKey);
			}
		insertcfg(curNode->xmlChildrenNode,xmltype);
		curNode = curNode->next;
		}
	return  0;
}
int load_tranmap_cfg(char *filename)
{
	char buff[1200];
	_tranmap	tmap;
	FILE *fp=NULL;


	memset(sql,0,sizeof(sql));
	strcpy(sql,"insert into tranmap values (NULL,?,?,?,?);");

	rc = sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,NULL);
	if(rc!=SQLITE_OK)
	{
		fprintf(stderr,"Cannot prepare sql :%s\n",sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}

	memset(buff,0,sizeof(buff));
	fp = fopen(filename,"r");
	if(fp==NULL)
	{
		perror("file open error");
		return -1;
	}
	while(fgets(buff,sizeof(buff),fp)!=NULL)
	{
		buff[strlen(buff)-1]='\0';
		if(buff[0]=='#')
		{
			continue;
		}
		if(buff[0]=='*')
		{
			strcpy(tmap.trancode,buff+1);
			continue;
		}
		/** need if else **/
		strcpy(tmap.trancode,strtok(buff,"^"));
		strcpy(tmap.tranname,strtok(NULL,"^"));
		strcpy(tmap.tranflow,strtok(NULL,"^"));
		tmap.timeout=atoi(strtok(NULL,"^"));

		sqlite3_bind_text(stmt,1,tmap.trancode,strlen(tmap.trancode),NULL);
		sqlite3_bind_text(stmt,2,tmap.tranname,strlen(tmap.tranname),NULL);
		sqlite3_bind_text(stmt,3,tmap.tranflow,strlen(tmap.tranflow),NULL);
		sqlite3_bind_int(stmt,4,tmap.timeout);
		rc = sqlite3_step(stmt);
		if(rc!=SQLITE_DONE)
		{
			fprintf(stderr,"Cannot setp sql :%s\n",sqlite3_errmsg(db));
			sqlite3_finalize(stmt);
			exit(1);
		}
		sqlite3_reset(stmt);
	}
	fclose(fp);
	printf("load ok\n");
	return 0;
}
int load_vardef_cfg(char *filename)
{
	char buff[1200];
	_vardef vardef;
	FILE *fp=NULL;


	memset(sql,0,sizeof(sql));
	strcpy(sql,"insert into vardef values (NULL,?,?,?,?);");

	rc = sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,NULL);
	if(rc!=SQLITE_OK)
	{
		fprintf(stderr,"Cannot prepare sql :%s\n",sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}

	memset(buff,0,sizeof(buff));
	fp = fopen(filename,"r");
	if(fp==NULL)
	{
		perror("file open error");
		return -1;
	}
	while(fgets(buff,sizeof(buff),fp)!=NULL)
	{
		buff[strlen(buff)-1]='\0';
		if(buff[0]=='#')
		{
			continue;
		}
		/** need if else **/
		strcpy(vardef.varname,strtok(buff,"^"));
		strcpy(vardef.varmark,strtok(NULL,"^"));
		strcpy(vardef.vartype,strtok(NULL,"^"));
		vardef.varlen=atoi(strtok(NULL,"^"));

		sqlite3_bind_text(stmt,1,vardef.varname,strlen(vardef.varname),NULL);
		sqlite3_bind_text(stmt,2,vardef.varmark,strlen(vardef.varmark),NULL);
		sqlite3_bind_text(stmt,3,vardef.vartype,strlen(vardef.vartype),NULL);
		sqlite3_bind_int(stmt,4,vardef.varlen);
		rc = sqlite3_step(stmt);
		if(rc!=SQLITE_DONE)
		{
			fprintf(stderr,"Cannot setp sql :%s\n",sqlite3_errmsg(db));
			sqlite3_finalize(stmt);
			exit(1);
		}
		sqlite3_reset(stmt);
	}
	fclose(fp);
	printf("load ok\n");
	return 0;
}
