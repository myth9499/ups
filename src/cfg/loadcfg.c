#include "ups.h"
#include <libxml/tree.h>
#include <libxml/parser.h>


int main(int argc,char *argv[])
{
	if(load_commmsg_cfg("/item/ups/src/cfg/channel/chnl.cfg")==0)
	{
		printf("FILE [%s] LINE [%d]:加载渠道配置文件[%s]成功\n",__FILE__,__LINE__,"/item/ups/src/cfg/channel/chnl.cfg");
	}else
	{
		printf("FILE [%s] LINE [%d]:加载渠道配置文件[%s]失败\n",__FILE__,__LINE__,"/item/ups/src/cfg/channel/chnl.cfg");
		return -1;
	}
	if(load_flow_cfg("/item/ups/src/cfg/flow/flow.cfg")==0)
	{
		printf("FILE [%s] LINE [%d]:加载流程配置文件[%s]成功\n",__FILE__,__LINE__,"/item/ups/src/cfg/flow/flow.cfg");
	}else
	{
		printf("FILE [%s] LINE [%d]:加载流程配置文件[%s]失败\n",__FILE__,__LINE__,"/item/ups/src/cfg/flow/flow.cfg");
		return -1;
	}
	if(load_xmlcfg("hvps.111.001.01","/item/ups/src/cfg/xmlcfg/hvps.111.001.01.xml")==0)
	{
		printf("FILE [%s] LINE [%d]:加载报文配置文件[%s]成功\n",__FILE__,__LINE__,"/item/ups/src/cfg/xmlcfg/hvps.111.001.01.xml");
	}else
	{
		printf("FILE [%s] LINE [%d]:加载报文配置文件[%s]失败\n",__FILE__,__LINE__,"/item/ups/src/cfg/xmlcfg/hvps.111.001.01.xml");
		return -1;
	}
	if(load_xmlcfg("ccms.990.001.02","/item/ups/src/cfg/xmlcfg/ccms.990.001.02.xml")==0)
	{
		printf("FILE [%s] LINE [%d]:加载报文配置文件[%s]成功\n",__FILE__,__LINE__,"/item/ups/src/cfg/xmlcfg/ccms.990.001.02.xml");
	}else
	{
		printf("FILE [%s] LINE [%d]:加载报文配置文件[%s]失败\n",__FILE__,__LINE__,"/item/ups/src/cfg/xmlcfg/ccms.990.001.02.xml");
		return -1;
	}
	if(load_tranmap_cfg("/item/ups/src/cfg/trancode/tran.cfg")==0)
	{
		printf("FILE [%s] LINE [%d]:加载交易映射配置文件[%s]成功\n",__FILE__,__LINE__,"/item/ups/src/cfg/trancode/tran.cfg");
	}else
	{
		printf("FILE [%s] LINE [%d]:加载交易映射配置文件[%s]失败\n",__FILE__,__LINE__,"/item/ups/src/cfg/trancode/tran.cfg");
		return -1;
	}
	if(load_vardef_cfg("/item/ups/src/cfg/vardef/vardef.cfg")==0)
	{
		printf("FILE [%s] LINE [%d]:加载变量映射配置文件[%s]成功\n",__FILE__,__LINE__,"/item/ups/src/cfg/vardef/vardef.cfg");
	}else
	{
		printf("FILE [%s] LINE [%d]:加载变量映射配置文件[%s]失败\n",__FILE__,__LINE__,"/item/ups/src/cfg/vardef/vardef.cfg");
		return -1;
	}
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
		SysLog(1,"get shm error\n");
		return -1;
	}
	SysLog(1,"start loadcfg \n");
	cmsg = (_commmsg *)shmat(shmid,NULL,0);
	if(cmsg == NULL)
	{
		SysLog(1,"cmsg shmat error\n");
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
	SysLog(1,"load ok\n");
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
		SysLog(1,"get shm error\n");
		return -1;
	}
	SysLog(1,"start loadcfg \n");
	flow = (_flow *)shmat(shmid,NULL,0);
	if(flow == NULL)
	{
		SysLog(1,"cmsg shmat error\n");
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
	SysLog(1,"load ok\n");
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
		SysLog(1,"get xml cfg error\n");
		return -1;
	}
	if((xmlcfg = shmat(shmid,NULL,0))==(void *)-1)
	{
		SysLog(1,"shmat xml cfg error\n");
		return -1;
	}

	xmlKeepBlanksDefault(0);
	doc = xmlReadFile(filename,"UTF-8",XML_PARSE_RECOVER);
	if(doc == NULL)
	{
		SysLog(1,"parse file error\n");
		return -1;
	}

	curNode = xmlDocGetRootElement(doc);
	if(curNode == NULL)
	{
		SysLog(1,"get root elemenet error\n");
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
			SysLog(1,"ELement name [%s]\n",curNode->name);
		}else if(curNode->type == XML_TEXT_NODE)
		{
			szKey = xmlNodeGetContent(curNode);
			SysLog(1,"[%s]---%s---\n",curNode->name,szKey);
			iret = getNodePath(path,curNode);
			SysLog(1,"path is [%s]\n",path);
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
					SysLog(1,"FILE[%s] LINE[%d] 放入变量[%s]\n",__FILE__,__LINE__,(xmlcfg+i)->varname);
					if(attr==NULL)
					{
						(xmlcfg+i)->depth = -1;
						break;
					} else
					{
						while(attr)
						{
							SysLog(1,"FILE[%s] LINE[%d] attr name[%s] attr value[%s]\n",__FILE__,__LINE__,attr->name,attr->children->content);
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
		SysLog(1,"get shm error\n");
		return -1;
	}
	SysLog(1,"start load tranmap  cfg \n");
	tmap = (_tranmap *)shmat(shmid,NULL,0);
	if(tmap == NULL)
	{
		SysLog(1,"tranmap shmat error\n");
		return -1;
	}
	//fp = fopen("/item/ups/src/cfg/channel/chnl.cfg","r");
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
	SysLog(1,"load ok\n");
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
		SysLog(1,"get shm error\n");
		return -1;
	}
	SysLog(1,"start load vardef  cfg \n");
	vardef = (_vardef *)shmat(shmid,NULL,0);
	if(vardef == NULL)
	{
		SysLog(1,"vardef shmat error\n");
		return -1;
	}
	//fp = fopen("/item/ups/src/cfg/channel/chnl.cfg","r");
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
	SysLog(1,"load ok\n");
	return 0;
}
