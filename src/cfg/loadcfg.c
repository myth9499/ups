#include "ups.h"
#include <libxml/tree.h>
#include <libxml/parser.h>

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
	char	cfgpath[100];
	int 	ret;

	memset(buffer,0,sizeof(buffer));
	memset(xmlname,0,sizeof(xmlname));
	memset(cfgfile,0,sizeof(cfgfile));
	memset(xmlns,0,sizeof(xmlns));
	memset(cfgpath,0,sizeof(cfgpath));

	sprintf(cfgpath,"%s%s",upshome,"/src/cfg/channel/chnl.cfg");
	if(load_commmsg_cfg(cfgpath)==0)
	{
		printf("FILE [%s] LINE [%d]:加载渠道配置文件[%s]成功\n",__FILE__,__LINE__,cfgpath);
	}else
	{
		printf("FILE [%s] LINE [%d]:加载渠道配置文件[%s]成功\n",__FILE__,__LINE__,cfgpath);
		return -1;
	}
	memset(cfgpath,0,sizeof(cfgpath));
	sprintf(cfgpath,"%s%s",upshome,"/src/cfg/flow/flow.cfg");
	if(load_flow_cfg(cfgpath)==0)
	{
		printf("FILE [%s] LINE [%d]:加载渠道配置文件[%s]成功\n",__FILE__,__LINE__,cfgpath);
	}else
	{
		printf("FILE [%s] LINE [%d]:加载渠道配置文件[%s]成功\n",__FILE__,__LINE__,cfgpath);
		return -1;
	}

	/**装载XML配置 **/
	memset(cfgpath,0,sizeof(cfgpath));
	sprintf(cfgpath,"%s%s",upshome,"/src/cfg/xmlcfg/loadxml.list");
	fp = fopen(cfgpath,"r");
	if(fp == NULL)
	{
		printf("FILE [%s] LINE [%d]:加载XML列表文件[%s]失败\n",__FILE__,__LINE__,cfgpath);
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
	memset(cfgpath,0,sizeof(cfgpath));
	sprintf(cfgpath,"%s%s",upshome,"/src/cfg/trancode/tran.cfg");
	if(load_tranmap_cfg(cfgpath)==0)
	{
		printf("FILE [%s] LINE [%d]:加载交易映射配置文件[%s]成功\n",__FILE__,__LINE__,cfgpath);
	}else
	{
		printf("FILE [%s] LINE [%d]:加载交易映射配置文件[%s]失败\n",__FILE__,__LINE__,cfgpath);
		return -1;
	}
	memset(cfgpath,0,sizeof(cfgpath));
	sprintf(cfgpath,"%s%s",upshome,"/src/cfg/vardef/vardef.cfg");
	if(load_vardef_cfg(cfgpath)==0)
	{
		printf("FILE [%s] LINE [%d]:加载变量映射配置文件[%s]成功\n",__FILE__,__LINE__,cfgpath);
	}else
	{
		printf("FILE [%s] LINE [%d]:加载变量映射配置文件[%s]失败\n",__FILE__,__LINE__,cfgpath);
		return -1;
	}
	if((ret=setsysparam())==-1)
	{
		printf("FILE [%s] LINE [%d]:设置LOGLEVEL失败\n",__FILE__,__LINE__);
	}
	printf("FILE [%s] LINE [%d]:设置LOGLEVEL成功,当前日志级别为[%d]\n",__FILE__,__LINE__,ret);
	return 0;
}
int load_commmsg_cfg(char *filename)
{
	key_t	key;
	int shmid,i=0;
	char buff[1200];
	char cname[60];
	_commmsg *cmsg=NULL;
	char *tmpbuf = NULL;
	size_t shmsize;
	FILE *fp=NULL;


	memset(buff,0,sizeof(buff));
	memset(cname,0,sizeof(cname));
	/** init commmsg **/
	shmsize=MAXCOMMMSG*sizeof(_commmsg);
	if((shmid=getshmid(9,shmsize))==-1)
	{
		SysLog(LOG_SYS_ERR,"get shm error\n");
		return -1;
	}
	SysLog(LOG_SYS_ERR,"start loadcfg \n");
	cmsg = (_commmsg *)shmat(shmid,NULL,0);
	if(cmsg == NULL)
	{
		SysLog(LOG_SYS_ERR,"cmsg shmat error\n");
		return -1;
	}
	fp = fopen(filename,"r");
	if(fp==NULL)
	{
		perror("file open error");
		shmdt(cmsg);
		return -1;
	}
	while(fgets(buff,sizeof(buff),fp)!=NULL)
	{
		buff[strlen(buff)-1]='\0';
		if(buff[0]=='*')
		{
			strcpy(cname,buff+1);
			strcpy(cmsg->commname,cname);
			//cmsg++;
			//memset(buff,0,sizeof(buff));
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
			strcpy(cmsg->commname,"over");
			cmsg->len=0;
			strcpy(cmsg->commvar," ");
			strcpy(cmsg->commmark," ");
			cmsg++;
			continue;
		}
		/** need if else **/
		strcpy(cmsg->commname,cname);
		strcpy(cmsg->commmark,strtok(buff,"^"));
		cmsg->len=atoi(strtok(NULL,"^"));
		strcpy(cmsg->commvar,strtok(NULL,"^"));
		cmsg++;
	}
	shmdt(cmsg);
	fclose(fp);
	SysLog(LOG_SYS_ERR,"load ok\n");
	return 0;
}
int load_flow_cfg(char *filename)
{
	key_t	key;
	int shmid,i=0;
	char buff[2000];
	char flowname[60];
	_flow *flow=NULL;
	char *tmpbuf = NULL;
	size_t shmsize;
	FILE *fp=NULL;


	memset(buff,0,sizeof(buff));
	memset(flowname,0,sizeof(flowname));
	/** init commmsg **/
	shmsize=MAXFLOW*sizeof(_flow);
	if((shmid=getshmid(8,shmsize))==-1)
	{
		SysLog(LOG_SYS_ERR,"get shm error\n");
		return -1;
	}
	SysLog(LOG_SYS_ERR,"start loadcfg \n");
	flow = (_flow *)shmat(shmid,NULL,0);
	if(flow == NULL)
	{
		SysLog(LOG_SYS_ERR,"cmsg shmat error\n");
		return -1;
	}
	fp = fopen(filename,"r");
	if(fp==NULL)
	{
		perror("file open error");
		shmdt(flow);
		return -1;
	}
	while(fgets(buff,sizeof(buff),fp)!=NULL)
	{
		buff[strlen(buff)-1]='\0';
		if(buff[0]=='*')
		{
			strcpy(flowname,buff+1);
			strcpy(flow->flowname,flowname);
			flow++;
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
			strcpy(flow->flowname,"over");
			strcpy(flow->flowmark," ");
			strcpy(flow->flowso," ");
			strcpy(flow->flowfunc," ");
			strcpy(flow->funcpar1," ");
			flow++;
			continue;
		}
		/** need if else **/
		strcpy(flow->flowname,flowname);
		strcpy(flow->flowmark,strtok(buff,"^"));
		strcpy(flow->flowso,strtok(NULL,"^"));
		strcpy(flow->flowfunc,strtok(NULL,"^"));
		strcpy(flow->funcpar1,strtok(NULL,"^"));
		strcpy(flow->errflow,strtok(NULL,"^"));
		flow++;
		memset(buff,0,sizeof(buff));
	}
	shmdt(flow);
	fclose(fp);
	SysLog(LOG_SYS_ERR,"load ok\n");
	return 0;
}

int load_xmlcfg(char *xmltype,char *filename)
{
	_xmlcfg *xmlcfg = NULL;
	xmlDocPtr doc;
	xmlNodePtr	curNode;

	int shmid = 0;
	size_t shmsize = MAXXMLCFG*sizeof(_xmlcfg);

	if((shmid = getshmid(6,shmsize))==-1)
	{
		SysLog(LOG_SYS_ERR,"get xml cfg error\n");
		return -1;
	}
	if((xmlcfg = shmat(shmid,NULL,0))==(void *)-1)
	{
		SysLog(LOG_SYS_ERR,"shmat xml cfg error\n");
		return -1;
	}

	xmlKeepBlanksDefault(0);
	doc = xmlReadFile(filename,"UTF-8",XML_PARSE_RECOVER);
	if(doc == NULL)
	{
		SysLog(LOG_SYS_ERR,"parse file error\n");
		return -1;
	}

	curNode = xmlDocGetRootElement(doc);
	if(curNode == NULL)
	{
		SysLog(LOG_SYS_ERR,"get root elemenet error\n");
		xmlFreeDoc(doc);
		return -1;
	}
	insertcfg(curNode,xmlcfg,xmltype);
	xmlFreeDoc(doc);
	shmdt(xmlcfg);
}
int insertcfg(xmlNodePtr cur,_xmlcfg *xmlcfg,char *xmltype)
{
	xmlChar	*szKey;
	xmlNodePtr curNode ;
	xmlAttr	*attr;
	curNode = cur;
	int  i = 0,iret = 0;
	char	path[256];
	memset(path,0,sizeof(path));

	while(curNode!=NULL)
	{
		if(curNode->type == XML_ELEMENT_NODE)
		{
			SysLog(LOG_SYS_ERR,"ELement name [%s]\n",curNode->name);
		}else if(curNode->type == XML_TEXT_NODE)
		{
			szKey = xmlNodeGetContent(curNode);
			SysLog(LOG_SYS_ERR,"[%s]---%s---\n",curNode->name,szKey);
			iret = getNodePath(path,curNode);
			SysLog(LOG_SYS_ERR,"path is [%s]\n",path);
			attr = curNode->parent->properties;
			for(i=0;i<MAXXMLCFG;i++)
			{
				if(!strcmp((xmlcfg+i)->xmlname,""))
				{
					memset(xmlcfg+i,0,sizeof(xmlcfg));
					strcpy((xmlcfg+i)->xmlname,xmltype);
					strcpy((xmlcfg+i)->mark,curNode->name);
					strcpy((xmlcfg+i)->fullpath,path);
					strcpy((xmlcfg+i)->varname,szKey);
					SysLog(LOG_SYS_ERR,"FILE[%s] LINE[%d] 放入变量[%s]\n",__FILE__,__LINE__,(xmlcfg+i)->varname);
					if(attr==NULL)
					{
						(xmlcfg+i)->depth = -1;
						break;
					} else
					{
						while(attr)
						{
							SysLog(LOG_SYS_ERR,"FILE[%s] LINE[%d] attr name[%s] attr value[%s]\n",__FILE__,__LINE__,attr->name,attr->children->content);
							if(!strcmp(attr->name,"loop"))
							{
								strcpy((xmlcfg+i)->loop,attr->children->content);
								break;
							}
							if(!strcmp(attr->name,"seq"))
							{
								(xmlcfg+i)->depth = atoi(attr->children->content);
								break;
							}
							else
							{
								(xmlcfg+i)->depth = -1;
								break;
							} 
							attr = attr->next;
						}
					}
					break;
				}
			}
			xmlFree(szKey);
		}
		insertcfg(curNode->xmlChildrenNode,xmlcfg,xmltype);
		curNode = curNode->next;
	}
	return  0;
}
int load_tranmap_cfg(char *filename)
{
	int shmid,i=0;
	char buff[1200];
	_tranmap *tmap=NULL;
	char *tmpbuf = NULL;
	size_t shmsize;
	FILE *fp=NULL;


	memset(buff,0,sizeof(buff));
	/** init commmsg **/
	shmsize=MAXTRANMAP*sizeof(_tranmap);
	if((shmid=getshmid(5,shmsize))==-1)
	{
		SysLog(LOG_SYS_ERR,"get shm error\n");
		return -1;
	}
	SysLog(LOG_SYS_ERR,"start load tranmap  cfg \n");
	tmap = (_tranmap *)shmat(shmid,NULL,0);
	if(tmap == NULL)
	{
		SysLog(LOG_SYS_ERR,"tranmap shmat error\n");
		return -1;
	}
	fp = fopen(filename,"r");
	if(fp==NULL)
	{
		perror("file open error");
		shmdt(tmap);
		return -1;
	}
	while(fgets(buff,sizeof(buff),fp)!=NULL)
	{
		buff[strlen(buff)-1]='\0';
		if(buff[0]=='#')
		{
			//cmsg++;
			continue;
		}
		if(buff[0]=='*')
		{
			strcpy(tmap->trancode,buff+1);
			tmap++;
			continue;
		}
		/** need if else **/
		strcpy(tmap->trancode,strtok(buff,"^"));
		strcpy(tmap->tranname,strtok(NULL,"^"));
		strcpy(tmap->tranflow,strtok(NULL,"^"));
		tmap->timeout=atoi(strtok(NULL,"^"));
		tmap++;
	}
	shmdt(tmap);
	fclose(fp);
	SysLog(LOG_SYS_ERR,"load ok\n");
	return 0;
}
int load_vardef_cfg(char *filename)
{
	int shmid,i=0;
	char buff[1200];
	_vardef *vardef=NULL;
	char *tmpbuf = NULL;
	size_t shmsize;
	FILE *fp=NULL;


	memset(buff,0,sizeof(buff));
	/** init vardef **/
	shmsize=MAXVARDEF*sizeof(_vardef);
	if((shmid=getshmid(4,shmsize))==-1)
	{
		SysLog(LOG_SYS_ERR,"get shm error\n");
		return -1;
	}
	SysLog(LOG_SYS_ERR,"start load vardef  cfg \n");
	vardef = (_vardef *)shmat(shmid,NULL,0);
	if(vardef == NULL)
	{
		SysLog(LOG_SYS_ERR,"vardef shmat error\n");
		return -1;
	}
	fp = fopen(filename,"r");
	if(fp==NULL)
	{
		perror("file open error");
		shmdt(vardef);
		return -1;
	}
	while(fgets(buff,sizeof(buff),fp)!=NULL)
	{
		buff[strlen(buff)-1]='\0';
		if(buff[0]=='#')
		{
			//cmsg++;
			continue;
		}
		/** need if else **/
		strcpy(vardef->varname,strtok(buff,"^"));
		strcpy(vardef->varmark,strtok(NULL,"^"));
		strcpy(vardef->vartype,strtok(NULL,"^"));
		vardef->varlen=atoi(strtok(NULL,"^"));
		vardef++;
	}
	shmdt(vardef);
	fclose(fp);
	SysLog(LOG_SYS_ERR,"load ok\n");
	return 0;
}

int	setsysparam(void)
{
	_sys_param  *sp = NULL;
	int shmsize,ret=-1;
	FILE	*fp = NULL;
	char	filepath[100];
	char	tmpbuf[100];
	memset(filepath,0x00,sizeof(filepath));
	memset(tmpbuf,0x00,sizeof(tmpbuf));

	shmsize =   sizeof(_sys_param);
	int shmid,isfound = 0;
	if((shmid=getshmid(3,shmsize))==-1)
	{
		printf("FILE [%s] LINE[%d] 获取系统公用共享内存失败:%s\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	sp = (_sys_param *)shmat(shmid,NULL,0);
	if(sp == NULL)
	{
		printf("FILE [%s] LINE[%d] 链接系统公用共享内存失败:%s\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	sprintf(filepath,"%s/%s",upshome,"/src/cfg/sys.cfg");
	fp = fopen(filepath,"r");
	if(fp == NULL)
	{
		printf("FILE [%s] LINE[%d] 打开[%s]配置失败:%s\n",__FILE__,__LINE__,filepath,strerror(errno));
		shmdt(sp);
		return -1;
	}
	while(fgets(tmpbuf,sizeof(tmpbuf),fp)!=NULL)
	{
		if(tmpbuf[0]=='#')
			continue;
		if(!memcmp(tmpbuf,"LOGLEVEL",8))
		{
			isfound = 1;
			sp->curloglvl=atoi(tmpbuf+9);
			ret = sp->curloglvl;
		}
	}
	if(isfound ==0)
	{
		printf("FILE [%s] LINE[%d] sys.cfg中未配置LOGLEVEL:%s\n",__FILE__,__LINE__,strerror(errno));
	}
	fclose(fp);
	shmdt(sp);
	return ret;
}
