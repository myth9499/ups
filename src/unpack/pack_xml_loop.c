/** XML 打包函数 **/
#include "ups.h"
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

_xmlcfg *tmpcfgloop = NULL;
_xmlcfg *dtcfgloop = NULL;

typedef struct	NODELIST
{
	char	nodepath[1024];/** 节点路径 **/
	xmlNodePtr	node;	   /** 节点对应的node指针 **/
	int	loop;			   /** 该节点循环次数 **/
}_nodelist;
_nodelist	nl[1024];

xmlNodePtr	getNodePtr(char	*fullpath,int loop)
{
	int	i=0;
	char	tmp[1024];
	char	tmpnodepath[256];
	memset(tmpnodepath,0x00,sizeof(tmpnodepath));
	xmlNodePtr	curNode;
	memset(tmp,0x00,sizeof(tmp));
	strcpy(tmp,fullpath);
	for(i=0;i<1024;i++)
	{
		if(!strcmp(fullpath,"/Document")&&nl[0].node!=NULL)
			return nl[0].node;
		if(!strcmp(nl[i].nodepath,fullpath)&&(nl[i].loop==loop))
		{
			return nl[i].node;
		}
		if(!strcmp(nl[i].nodepath," ")||(strlen(nl[i].nodepath)==0))
		{
			if(i==0)
			{
				curNode = xmlNewNode(NULL,BAD_CAST(tmp+1));
				nl[i].node = curNode;
				nl[i].loop = 0;
				strcpy(nl[i].nodepath,fullpath);
				return curNode;
			}
			memset(tmpnodepath,0x00,sizeof(tmpnodepath));
			strcpy(tmpnodepath,strtok(tmp,"/"));
			curNode = xmlNewNode(NULL,BAD_CAST(tmpnodepath));
			nl[i].node = curNode;
			strcpy(nl[i].nodepath,fullpath);
			nl[i].loop = loop;
			memset(tmpnodepath,0x00,sizeof(tmpnodepath));
			strcpy(tmpnodepath,strstr(fullpath+1,"/"));
			if(!strcmp(tmpnodepath,"/"))
				return ;
			xmlAddChild(getNodePtr(tmpnodepath,loop),curNode);
			return curNode;
		}
	}
	return NULL;
}
int pack_xml_loop(char *xmltype)
{
	int shmid = 0,flag = 0;
	xmlDocPtr doc = NULL;
	int i ,lastlen;
	char	keyvalue[4096];
	char	tmppath[256];
	char	docpath[256];
	char	fullpath[1024];
	char	tmpfullpath[1024];
	char	ltpath[256];
	char	*tpath=NULL;
	char	lastpath[256];
	char	*tmp;
	size_t shmsize = MAXXMLCFG*sizeof(_xmlcfg);
	char	xmlcfgpath[60];
	char	msgtype[30];
	char	ffile[60];
	xmlNodePtr	root,curnode,lastnode;


	memset(tmppath,0,sizeof(tmppath));
	memset(docpath,0,sizeof(docpath));
	memset(fullpath,0,sizeof(fullpath));
	memset(tmpfullpath,0,sizeof(tmpfullpath));
	memset(ltpath,0,sizeof(ltpath));
	memset(nl,0,sizeof(nl));
	memset(lastpath,0,sizeof(lastpath));
	memset(xmlcfgpath,0,sizeof(xmlcfgpath));
	memset(msgtype,0,sizeof(msgtype));
	memset(ffile,0,sizeof(ffile));

	xmlInitParser();

	if((shmid = getshmid(6,shmsize))==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d] 获取XML配置失败\n",__FILE__,__LINE__);
		return -1;
	}
	if((tmpcfgloop = shmat(shmid,NULL,0))==(void *)-1)
	{
		SysLog(1,"FILE [%s] LINE [%d] 获取XML配置失败\n",__FILE__,__LINE__);
		return -1;
	}
	dtcfgloop = tmpcfgloop;
	xmlKeepBlanksDefault(0);
	xmlIndentTreeOutput = 1;

	/** 获取变量对应配置 **/
	if(!strcmp(xmltype,"V")||strlen(xmltype)==0)
	{
		SysLog(1,"从变量V_MSGTYPE取报文类型进行处理");
		if(get_var_value("V_MSGTYPE",sizeof(msgtype),1,msgtype)!=0)
		{
			SysLog(1,"从变量V_MSGTYPE取报文类型进行处理失败");
			return  -1;
		}
		SysLog(1,"获取到待解包报文类型[%s]\n",msgtype);
		trim(msgtype);
		sprintf(xmlcfgpath,"%s%s/%s.xml",upshome,"/src/cfg/xmlcfg",msgtype);
	}else
	{
		SysLog(1,"直接从参数读取\n");
		sprintf(xmlcfgpath,"%s%s/%s.xml",upshome,"/src/cfg/xmlcfg",xmltype);
		strcpy(msgtype,xmltype);
	}


	doc = xmlNewDoc(BAD_CAST"1.0");
	if(doc == NULL)
	{
		SysLog(1,"FILE [%s] LINE [%d] 获取doc指针\n",__FILE__,__LINE__);
		return -1;
	}

	/** 根据全路径打包 **/

	while(strcmp(tmpcfgloop->xmlname,""))
	{
		if(!strcmp(tmpcfgloop->xmlname,xmltype)&&strcmp(lastpath,tmpcfgloop->fullpath))
		{
			if(!strcmp(tmpcfgloop->loop,"Y"))
			{
				int l=0;
				/** 获取循环次数**/
				char	loopdepth[5];
				memset(loopdepth,0x00,sizeof(loopdepth));
				get_var_value("V_LOOP",sizeof(loopdepth),1,loopdepth);
				for(l=0;l<atoi(loopdepth);l++)
				{
					memset(tmppath,0,sizeof(tmppath));
					strcpy(tmppath,tmpcfgloop->fullpath);
				//	SysLog(1,"FILE [%s] LINE [%d] path is  [%s] flag is [%d]loop[%s]:\n",__FILE__,__LINE__,tmppath,flag,tmpcfgloop->loop);
					/** 判断全路径倒数第二个节点是否已经创建，若已经创建，判断是否与当前循环次数相符**/
					if(flag==0)
					{
						tpath = strtok(tmppath,"/");
						sprintf(docpath,"/%s",tpath);
						root=getNodePtr(docpath,l);
						xmlDocSetRootElement(doc,root);
						SysLog(1,"FILE [%s] LINE [%d] set the root element flag is [%d]:\n",__FILE__,__LINE__,flag);
					}
					if(flag ==1)
					{
						/** 为了方便递归调用，此处存放时倒序存放 **/
						strcpy(docpath,strtok(tmppath,"/"));
					}
					memset(fullpath,0x00,sizeof(fullpath));
					while((tpath = strtok(NULL,"/"))!=NULL)
					{
						memset(tmpfullpath,0x00,sizeof(tmpfullpath));
						strcpy(tmpfullpath,fullpath);
						sprintf(fullpath,"%s/%s",tpath,tmpfullpath);
						lastlen = strlen(tpath)+1;
						strcpy(ltpath,tpath);
					}
					if(flag ==0)
					{
						strcat(fullpath,docpath+1);
					}else
					{
						strcat(fullpath,docpath);
					}
					strcpy(tmpfullpath,strstr(fullpath,"/"));
					memset(keyvalue,0,sizeof(keyvalue));
					if(get_var_value(tmpcfgloop->varname,sizeof(keyvalue),l+1,keyvalue)==-1)
					{
						SysLog(1,"get keyvalue error\n");
						flag = 1;
						continue;
					}else
					{
						lastnode = getNodePtr(tmpfullpath,l);
						curnode = xmlNewChild(lastnode,NULL,BAD_CAST(ltpath),BAD_CAST(keyvalue));
						flag = 1;
					}
				}
			}else
			{
				memset(tmppath,0,sizeof(tmppath));
				strcpy(tmppath,tmpcfgloop->fullpath);
				//SysLog(1,"FILE [%s] LINE [%d] path is  [%s] flag is [%d]:\n",__FILE__,__LINE__,tmppath,flag);
				/** 判断全路径倒数第二个节点是否已经创建，若已经创建，判断是否与当前循环次数相符**/
				if(flag==0)
				{
					tpath = strtok(tmppath,"/");
					sprintf(docpath,"/%s",tpath);
					root=getNodePtr(docpath,0);
					xmlDocSetRootElement(doc,root);
					SysLog(1,"FILE [%s] LINE [%d] set the root element flag is [%d]:\n",__FILE__,__LINE__,flag);
				}
				if(flag ==1)
				{
					/** 为了方便递归调用，此处存放时倒序存放 **/
					strcpy(docpath,strtok(tmppath,"/"));
				}
				memset(fullpath,0x00,sizeof(fullpath));
				while((tpath = strtok(NULL,"/"))!=NULL)
				{
					memset(tmpfullpath,0x00,sizeof(tmpfullpath));
					strcpy(tmpfullpath,fullpath);
					sprintf(fullpath,"%s/%s",tpath,tmpfullpath);
					lastlen = strlen(tpath)+1;
					strcpy(ltpath,tpath);
				}
				if(flag ==0)
				{
					strcat(fullpath,docpath+1);
				}else
				{
					strcat(fullpath,docpath);
				}
				strcpy(tmpfullpath,strstr(fullpath,"/"));
				if(get_var_value(tmpcfgloop->varname,sizeof(keyvalue),1,keyvalue)==-1)
				{
					SysLog(1,"get keyvalue error\n");
					flag = 1;
				}else
				{
					lastnode = getNodePtr(tmpfullpath,0);
					curnode = xmlNewChild(lastnode,NULL,BAD_CAST(ltpath),BAD_CAST(keyvalue));
					flag = 1;
				}
			}
		}
		memset(lastpath,0,sizeof(lastpath));
		strcpy(lastpath,tmpcfgloop->fullpath);
		tmpcfgloop++;
	}
	/** 生成新的文件名,放置到变量中 **/
	memset(ffile,0,sizeof(ffile));
	if(GetNewFileName(ffile)==0)
	{
		xmlSaveFormatFile(ffile,doc,1);
		/** 放置到变量中**/
		if(put_var_value("V_XMLFILE",strlen(ffile),1,ffile)!=0)
		{
			SysLog(1,"打包文件失败\n");
			xmlFreeDoc(doc);
			xmlCleanupParser();
			//xmlXPathFreeObject(result);
			xmlMemoryDump();
			shmdt(dtcfgloop);
			return -1;
		}else
		{
			SysLog(1,"打包文件成功[%s]\n",ffile);
			xmlFreeDoc(doc);
			xmlCleanupParser();
			//xmlXPathFreeObject(result);
			xmlMemoryDump();
			shmdt(dtcfgloop);
			return 0;
		}
	}else
	{
		SysLog(1,"打包文件失败\n");
		xmlFreeDoc(doc);
		xmlCleanupParser();
		//xmlXPathFreeObject(result);
		xmlMemoryDump();
		shmdt(dtcfgloop);
		return -1;
	}
	xmlFreeDoc(doc);
	xmlCleanupParser();
	//xmlXPathFreeObject(result);
	xmlMemoryDump();
	shmdt(dtcfgloop);
	return  0;
}
