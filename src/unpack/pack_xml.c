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

	memset(buffer,0,sizeof(buffer));
	memset(xmlname,0,sizeof(xmlname));
	memset(cfgfile,0,sizeof(cfgfile));
	memset(xmlns,0,sizeof(xmlns));

	fp = fopen("/item/ups/src/cfg/xmlcfg/loadxml.list","r");
	if(fp == NULL)
	{
		SysLog(1,"FILE [%s] LINE [%d]:加载XML列表文件[%s]失败\n",__FILE__,__LINE__,"/item/ups/src/cfg/xmlcfg/loadxml.list");
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
				SysLog(1,"Error: unable to create new XPath context\n");  
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
		SysLog(1,"传入生成文件名存放空间为NULL\n");
		return -1;
	}
	memset(nfilename,0,L_tmpnam+1);
	//strcpy(nfilename,tmpnam(nfilename));
	if(tmpnam(nfilename)==NULL)
	{
		SysLog(1,"生成临时文件失败[%s]\n",strerror(errno));
		return -1;
	}
	sprintf(ffile,"%s%s","/item/ups/log",nfilename);
	SysLog(1,"临时文件名[%s]\n",ffile);
	return 0;
}
xmlDocPtr  getdoc (char *docname) 
{                                                  
	xmlDocPtr doc;                                                                     
	doc = xmlParseFile(docname);                                                       

	if (doc == NULL ) {                                                                
		SysLog(1,"FILE [%s] LINE[%d] Document not parsed successfully. \n",__FILE__,__LINE__);                          
		return NULL;                                                                     
	}                                                                                  

	return doc;                                                                        
}                                                                                    

xmlXPathObjectPtr  getnodeset (xmlDocPtr doc, xmlChar *xpath,char *xmltype)
{                       

	if(doc == NULL || strlen(xpath)==0)
	{
		SysLog(1,"FILE [%s] LINE[%d] doc == NULL of xpath  == NULL. \n",__FILE__,__LINE__);                          
		return NULL;
	}
	xmlXPathContextPtr context;                                                        
	xmlXPathObjectPtr result;                                                          

	context = xmlXPathNewContext(doc);                                                 
	if (context == NULL) {                                                             
		SysLog(1,"FILE [%s] LINE[%d] Error in xmlXPathNewContext. \n",__FILE__,__LINE__);                          
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
		SysLog(1,"FILE [%s] LINE[%d] Error in xmlXPathEvalExpression. \n",__FILE__,__LINE__);                          
		return NULL;                                                                     
	}                                                                                  
	if(xmlXPathNodeSetIsEmpty(result->nodesetval)){                                    
		xmlXPathFreeObject(result);                                                      
		SysLog(1,"FILE [%s] LINE[%d] no result. \n",__FILE__,__LINE__);                          
		return NULL;                                                                     
	}                                                                                  
	return result;                                                                     
}  
int pack_xml(char *xmltype)
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
		SysLog(1,"FILE [%s] LINE [%d] 获取XML配置失败\n",__FILE__,__LINE__);
		return -1;
	}
	if((tmpcfg = shmat(shmid,NULL,0))==(void *)-1)
	{
		SysLog(1,"FILE [%s] LINE [%d] 获取XML配置失败\n",__FILE__,__LINE__);
		return -1;
	}
	dtcfg = tmpcfg;
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
		SysLog(1,"获取到待解包报文类型[%s]",msgtype);
		trim(msgtype);
		sprintf(xmlcfgpath,"%s/%s.xml","/item/ups/src/cfg/xmlcfg",msgtype);
	}else
	{
		SysLog(1,"直接从参数读取\n");
		sprintf(xmlcfgpath,"%s/%s.xml","/item/ups/src/cfg/xmlcfg",xmltype);
		strcpy(msgtype,xmltype);
	}


	doc = getdoc(xmlcfgpath);
	if(doc == NULL)
	{
		SysLog(1,"FILE [%s] LINE [%d] 获取doc指针\n",__FILE__,__LINE__);
		return -1;
	}


	while(strcmp(tmpcfg->xmlname,""))
	{
		if(!strcmp(tmpcfg->xmlname,xmltype)&&strcmp(lastpath,tmpcfg->fullpath))
		{
			memset(tmppath,0,sizeof(tmppath));
			strcpy(tmppath,tmpcfg->fullpath);
			SysLog(1,"FILE [%s] LINE [%d] 1:\n",__FILE__,__LINE__);
			tmp = strtok(tmppath,"/");
			sprintf(xpath,"/lilei:%s",tmp);
			while((tmp = strtok(NULL,"/"))!=NULL)
			{
				sprintf(xpath,"%s/lilei:%s",xpath,tmp);
			}
			SysLog(1,"FILE[%s]LINE[%d]xpath is [%s]\n",__FILE__,__LINE__,xpath);
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
						SysLog(1,"FILE [%s] LINE [%d] malloc xpath error:\n",__FILE__,__LINE__,strerror(errno));
						break;
					}
					/** 根据值获取到对应变量信息 **/
					memset(keyvalue,0,sizeof(keyvalue));
					if(get_var_value(keyword,sizeof(keyvalue),1,keyvalue)==-1)
					{
						SysLog(1,"FILE [%s] LINE [%d] 获取[%s]变量值失败\n",__FILE__,__LINE__,keyword);
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
						SysLog(1,"FILE [%s] LINE [%d] keyword:%s keyvalue[%s]\n",__FILE__,__LINE__,keyword,keyvalue);
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
			SysLog(1,"打包文件失败\n");
			xmlFreeDoc(doc);
			xmlCleanupParser();
			//xmlXPathFreeObject(result);
			xmlMemoryDump();
			shmdt(dtcfg);
			return -1;
		}else
		{
			SysLog(1,"打包文件成功[%s]\n",ffile);
			xmlFreeDoc(doc);
			xmlCleanupParser();
			//xmlXPathFreeObject(result);
			xmlMemoryDump();
			shmdt(dtcfg);
			return 0;
		}
	}else
	{
		SysLog(1,"打包文件失败\n");
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
