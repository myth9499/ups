/** XML 打包函数 **/
#include "ups.h"
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

_xmlcfg *tmpcfg = NULL;
_xmlcfg *dtcfg = NULL;

int	regNs(xmlXPathContextPtr context,char	*xmltype)
{
	FILE    *fp=NULL;
	char    buffer[1024];
	char    xmlname[256];
	char    cfgfile[256];
	char    xmlns[256];
	char    cfgpath[100];

	memset(buffer,0,sizeof(buffer));
	memset(xmlname,0,sizeof(xmlname));
	memset(cfgfile,0,sizeof(cfgfile));
	memset(xmlns,0,sizeof(xmlns));
	memset(cfgpath,0,sizeof(cfgpath));

	sprintf(cfgpath,"%s%s",upshome,"/src/cfg/xmlcfg/loadxml.list");
	fp = fopen(cfgpath,"r");
	if(fp == NULL)
	{
		SysLog(LOG_APP_ERR,"FILE [%s] LINE [%d]:加载XML列表文件[%s]失败\n",__FILE__,__LINE__,cfgpath);
		return -1;
	}
	while(fgets(buffer,sizeof(buffer),fp)!=NULL)
	{
		if(buffer[0]=='#')
			continue;
		buffer[strlen(buffer)-1]='\0';
		strcpy(xmlname,strtok(buffer,"^"));
		strcpy(cfgfile,strtok(NULL,"^"));
		strcpy(xmlns,strtok(NULL,"^"));
		if(!strcmp(xmltype,xmlname))
		{
			xmlXPathRegisterNs(context,BAD_CAST"lilei",BAD_CAST(xmlns));  
			if(!context)  
			{  
				SysLog(LOG_APP_ERR,"Error: 无法创建新的xmlpath\n");  
				fclose(fp);
				return -1;  
			}
			fclose(fp);
			return 0;
		}
		memset(buffer,0,sizeof(buffer));
		memset(xmlname,0,sizeof(xmlname));
		memset(cfgfile,0,sizeof(cfgfile));
		memset(xmlns,0,sizeof(xmlns));
	}
	fclose(fp);
	return -1;
}
/** 生成最新的文件名 **/
int	GetNewFileName(char	*ffile)
{
	char	nfilename[L_tmpnam+1];
	if(ffile == NULL)
	{
		SysLog(LOG_APP_ERR,"传入生成文件名存放空间为NULL\n");
		return -1;
	}
	memset(nfilename,0,L_tmpnam+1);
	//strcpy(nfilename,tmpnam(nfilename));
	if(tmpnam(nfilename)==NULL)
	{
		SysLog(LOG_APP_ERR,"生成临时文件失败[%s]\n",strerror(errno));
		return -1;
	}
	sprintf(ffile,"%s%s%s",upshome,"/msg",nfilename);
	SysLog(LOG_APP_SHOW,"临时文件名[%s]\n",ffile);
	return 0;
}
xmlDocPtr  getdoc (char *docname) 
{                                                  
	xmlDocPtr doc;                                                                     
	doc = xmlParseFile(docname);                                                       

	if (doc == NULL ) {                                                                
		SysLog(LOG_APP_ERR,"FILE [%s] LINE[%d] 获取XML文件指针失败. \n",__FILE__,__LINE__);                          
		return NULL;                                                                     
	}                                                                                  

	return doc;                                                                        
}                                                                                    

xmlXPathObjectPtr  getnodeset (xmlDocPtr doc, xmlChar *xpath,char *xmltype)
{                       

	if(doc == NULL || strlen(xpath)==0)
	{
		SysLog(LOG_APP_ERR,"FILE [%s] LINE[%d] 传入变量地址不合法. \n",__FILE__,__LINE__);                          
		return NULL;
	}
	xmlXPathContextPtr context;                                                        
	xmlXPathObjectPtr result;                                                          

	context = xmlXPathNewContext(doc);                                                 
	if (context == NULL) {                                                             
		SysLog(LOG_APP_ERR,"FILE [%s] LINE[%d] 创建XMLNEWPATH失败. \n",__FILE__,__LINE__);                          
		return NULL;                                                                     
	}
	/** 根据报文类型，设置xpath命名空间 **/
	if(regNs(context,xmltype)!=0)
	{
		xmlXPathFreeContext(context);                                                      
		xmlFreeDoc(doc);   
		return NULL;                                                                     
	}
	result = xmlXPathEvalExpression(xpath, context);                                   
	xmlXPathFreeContext(context);                                                      
	if (result == NULL) {                                                              
		SysLog(LOG_APP_ERR,"FILE [%s] LINE[%d] 使用xmlpath失败. \n",__FILE__,__LINE__);                          
		return NULL;                                                                     
	}                                                                                  
	if(xmlXPathNodeSetIsEmpty(result->nodesetval)){                                    
		xmlXPathFreeObject(result);                                                      
		SysLog(LOG_APP_ERR,"FILE [%s] LINE[%d] 无该XPATH对应结果. \n",__FILE__,__LINE__);                          
		return NULL;                                                                     
	}                                                                                  
	return result;                                                                     
}  
int	pack_xml(char	*xmltype)
{
	/** 获取V_LOOP 值，若V_LOOP==0或者未获取到，走单独打包XML **/
	char	loop[5];
	memset(loop,0x00,sizeof(loop));
	int	ret;
	ret = get_var_value("V_LOOP",sizeof(loop),1,loop);
	if(ret ==-1||atoi(loop)<=0)
	{
		SysLog(LOG_APP_SHOW,"走单独XML打包函数\n");
		return (pack_xml_single(xmltype));
	}else
	{
		SysLog(LOG_APP_SHOW,"走循环XML打包函数\n");
		return (pack_xml_loop(xmltype));
	}
	return 0;
}
int pack_xml_single(char *xmltype)
{
	int shmid = 0,flag = 0;
	xmlDocPtr doc = NULL;
	xmlChar	xpath[1024];
	xmlNodeSetPtr	nodeset;
	xmlXPathObjectPtr	result;
	int i ;
	xmlChar	*keyword;
	char	keyvalue[4096];
	char	tmppath[256];
	char	lastpath[256];
	char	*tmp;
	size_t shmsize = MAXXMLCFG*sizeof(_xmlcfg);
	char	xmlcfgpath[60];
	char	msgtype[30];

	char	ffile[60];


	memset(tmppath,0,sizeof(tmppath));
	memset(lastpath,0,sizeof(lastpath));
	memset(xmlcfgpath,0,sizeof(xmlcfgpath));
	memset(msgtype,0,sizeof(msgtype));
	memset(ffile,0,sizeof(ffile));

	xmlInitParser();

	if((shmid = getshmid(6,shmsize))==-1)
	{
		SysLog(LOG_APP_ERR,"FILE [%s] LINE [%d] 获取XML配置失败\n",__FILE__,__LINE__);
		return -1;
	}
	if((tmpcfg = shmat(shmid,NULL,0))==(void *)-1)
	{
		SysLog(LOG_APP_ERR,"FILE [%s] LINE [%d] 获取XML配置失败\n",__FILE__,__LINE__);
		return -1;
	}
	dtcfg = tmpcfg;
	xmlKeepBlanksDefault(0);
	xmlIndentTreeOutput = 1;

	/** 获取变量对应配置 **/
	if(!strcmp(xmltype,"V")||strlen(xmltype)==0)
	{
		SysLog(LOG_APP_SHOW,"从变量V_MSGTYPE取报文类型进行处理");
		if(get_var_value("V_MSGTYPE",sizeof(msgtype),1,msgtype)!=0)
		{
			SysLog(LOG_APP_ERR,"从变量V_MSGTYPE取报文类型进行处理失败");
			return  -1;
		}
		SysLog(LOG_APP_SHOW,"获取到待解包报文类型[%s]\n",msgtype);
		trim(msgtype);
		sprintf(xmlcfgpath,"%s%s/%s.xml",upshome,"/src/cfg/xmlcfg",msgtype);
	}else
	{
		SysLog(LOG_APP_SHOW,"直接从参数读取\n");
		sprintf(xmlcfgpath,"%s%s/%s.xml",upshome,"/src/cfg/xmlcfg",xmltype);
		strcpy(msgtype,xmltype);
	}


	doc = getdoc(xmlcfgpath);
	if(doc == NULL)
	{
		SysLog(LOG_APP_ERR,"FILE [%s] LINE [%d] 获取doc指针\n",__FILE__,__LINE__);
		return -1;
	}


	while(strcmp(tmpcfg->xmlname,""))
	{
		if(!strcmp(tmpcfg->xmlname,xmltype)&&strcmp(lastpath,tmpcfg->fullpath))
		{
			memset(tmppath,0,sizeof(tmppath));
			strcpy(tmppath,tmpcfg->fullpath);
			SysLog(LOG_APP_DEBUG,"FILE [%s] LINE [%d] 1:\n",__FILE__,__LINE__);
			tmp = strtok(tmppath,"/");
			sprintf(xpath,"/lilei:%s",tmp);
			while((tmp = strtok(NULL,"/"))!=NULL)
			{
				sprintf(xpath,"%s/lilei:%s",xpath,tmp);
			}
			SysLog(LOG_APP_DEBUG,"FILE[%s]LINE[%d]xpath is [%s]\n",__FILE__,__LINE__,xpath);
			result = getnodeset(doc,xpath,xmltype);
			if(result)
			{
				nodeset = result->nodesetval;
				for(i=0;i<nodeset->nodeNr;i++)
				{
					keyword = xmlNodeListGetString(doc,nodeset->nodeTab[i]->xmlChildrenNode,1);
					if(keyword == NULL)
					{
						memset(xpath,0,sizeof(xpath));
						SysLog(LOG_APP_ERR,"FILE [%s] LINE [%d] 获取XPATH失败:\n",__FILE__,__LINE__,strerror(errno));
						break;
					}
					/** 根据值获取到对应变量信息 **/
					memset(keyvalue,0,sizeof(keyvalue));
					if(get_var_value(keyword,sizeof(keyvalue),1,keyvalue)==-1)
					{
						SysLog(LOG_APP_ERR,"FILE [%s] LINE [%d] 获取[%s]变量值失败\n",__FILE__,__LINE__,keyword);
						/**
						nodeset->nodeTab[i]->xmlChildrenNode->parent->prev->next=nodeset->nodeTab[i]->xmlChildrenNode->parent->next;
						if(nodeset->nodeTab[i]->xmlChildrenNode->parent->next!=NULL)
						{
							nodeset->nodeTab[i]->xmlChildrenNode->parent->next->prev=nodeset->nodeTab[i]->xmlChildrenNode->parent->prev;
						}else
						{
							nodeset->nodeTab[i]->xmlChildrenNode->parent=NULL;
						}
						**/
						/** test not use  **/
						xmlNodePtr	tempNode;
						tempNode=nodeset->nodeTab[i]->next;
						xmlUnlinkNode(nodeset->nodeTab[i]);
						xmlFreeNode(nodeset->nodeTab[i]);
						nodeset->nodeTab[i]=tempNode;
						free(keyword);
						//xmlFreeNode(nodeset->nodeTab[i]->xmlChildrenNode->parent);
						//xmlXPathFreeObject(result);
						continue;
					}else
					{
						SysLog(LOG_APP_SHOW,"FILE [%s] LINE [%d] 变量:%s 变量值[%s]\n",__FILE__,__LINE__,keyword,keyvalue);
						xmlNodeSetContent(nodeset->nodeTab[i],keyvalue);
						free(keyword);
					}
				}
				xmlXPathFreeObject(result);
			}
			memset(xpath,0,sizeof(xpath));
		}
		memset(lastpath,0,sizeof(lastpath));
		strcpy(lastpath,tmpcfg->fullpath);
		tmpcfg++;
	}
	/** 生成新的文件名,放置到变量中 **/
	memset(ffile,0,sizeof(ffile));
	if(GetNewFileName(ffile)==0)
	{
		xmlSaveFormatFile(ffile,doc,1);
		/** 放置到变量中**/
		if(put_var_value("V_XMLFILE",strlen(ffile),1,ffile)!=0)
		{
			SysLog(LOG_APP_ERR,"打包文件失败\n");
			xmlFreeDoc(doc);
			xmlCleanupParser();
			//xmlXPathFreeObject(result);
			xmlMemoryDump();
			shmdt(dtcfg);
			return -1;
		}else
		{
			SysLog(LOG_APP_SHOW,"打包文件成功[%s]\n",ffile);
			xmlFreeDoc(doc);
			xmlCleanupParser();
			//xmlXPathFreeObject(result);
			xmlMemoryDump();
			shmdt(dtcfg);
			return 0;
		}
	}else
	{
		SysLog(LOG_APP_SHOW,"打包文件失败\n");
		xmlFreeDoc(doc);
		xmlCleanupParser();
		//xmlXPathFreeObject(result);
		xmlMemoryDump();
		shmdt(dtcfg);
		return -1;
	}
	xmlFreeDoc(doc);
	xmlCleanupParser();
	//xmlXPathFreeObject(result);
	xmlMemoryDump();
	shmdt(dtcfg);
	return  0;
}
