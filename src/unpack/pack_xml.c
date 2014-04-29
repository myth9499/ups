/** XML 打包函数 **/
#include "ups.h"
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

_xmlcfg *tmpcfg = NULL;
_xmlcfg *dtcfg = NULL;
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

xmlXPathObjectPtr  getnodeset (xmlDocPtr doc, xmlChar *xpath)
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
	xmlXPathRegisterNs(context,BAD_CAST"lilei",BAD_CAST"urn:iso:std:iso:20022:tech:xsd:pacs.008.001.02");  
	if(!context)  
	{  
		SysLog(1,"Error: unable to create new XPath context\n");  
		xmlFreeDoc(doc);   
		return(NULL);  
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
	xmlChar	*xpath;
	xmlNodeSetPtr	nodeset;
	xmlXPathObjectPtr	result;
	int i ;
	xmlChar	*keyword;
	char	keyvalue[1024];
	char	tmppath[256];
	char	lastpath[256];
	char	*tmp;
	size_t shmsize = MAXXMLCFG*sizeof(_xmlcfg);

	memset(tmppath,0,sizeof(tmppath));
	memset(lastpath,0,sizeof(lastpath));


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

	doc = getdoc("/item/ups/src/cfg/xmlcfg/111.xml");
	if(doc == NULL)
	{
		SysLog(1,"FILE [%s] LINE [%d] 获取doc指针\n",__FILE__,__LINE__);
		return -1;
	}


	while(strcmp(tmpcfg->xmlname,""))
	{
		SysLog(1,"FILE [%s] LINE [%d] xmlnames[%s]fullpath[%s] last path[%s]xmltype[%s]\n",__FILE__,__LINE__,tmpcfg->xmlname,tmpcfg->fullpath,lastpath,xmltype);
		//if(!strcmp(tmpcfg->xmlname,xmltype))
		if(!strcmp(tmpcfg->xmlname,xmltype)&&strcmp(lastpath,tmpcfg->fullpath))
		{
			//SysLog(1,"FILE[%s] LINE[%d] 路径[%s]变量名[%s]变量值[%s]深度[%d]\n",__FILE__,__LINE__,tmpcfg->fullpath,tmpcfg->varname,"123",tmpcfg->depth);
			xpath  = (xmlChar *)malloc(sizeof(xmlChar)*1024);
			if(xpath == NULL)
			{
				SysLog(1,"FILE [%s] LINE [%d] malloc xpath error:\n",__FILE__,__LINE__,strerror(errno));
				return -1;
			}
			memset(tmppath,0,sizeof(tmppath));
			strcpy(tmppath,tmpcfg->fullpath);
						SysLog(1,"FILE [%s] LINE [%d] 1:\n",__FILE__,__LINE__);
			tmp = strtok(tmppath,"/");
			sprintf(xpath,"/lilei:%s",tmp);
			while((tmp = strtok(NULL,"/"))!=NULL)
			{
				sprintf(xpath,"%s/lilei:%s",xpath,tmp);
			}
			result = getnodeset(doc,xpath);
			if(result)
			{
				nodeset = result->nodesetval;
				for(i=0;i<nodeset->nodeNr;i++)
				{
					keyword  = (xmlChar *)malloc(sizeof(xmlChar)*1024);
					if(keyword == NULL)
					{
						free(xpath);
						SysLog(1,"FILE [%s] LINE [%d] malloc xpath error:\n",__FILE__,__LINE__,strerror(errno));
						return -1;
					}
					keyword = xmlNodeListGetString(doc,nodeset->nodeTab[i]->xmlChildrenNode,1);
					/** 根据值获取到对应变量信息 **/
					memset(keyvalue,0,sizeof(keyvalue));
					if(get_var_value(keyword,sizeof(keyvalue),1,keyvalue)==-1)
					{
						SysLog(1,"FILE [%s] LINE [%d] 获取[%s]变量值失败\n",__FILE__,__LINE__,keyword);
						free(keyword);
						free(xpath);
						return -1;
					}
					SysLog(1,"FILE [%s] LINE [%d] keyword:%s keyvalue[%s]\n",__FILE__,__LINE__,keyword,keyvalue);
					xmlNodeSetContent(nodeset->nodeTab[i],keyvalue);
					memset(keyword,0,sizeof(keyword));
					free(keyword);
				}
				xmlXPathFreeObject(result);
			}
			free(xpath);
		}
		memset(lastpath,0,sizeof(lastpath));
		strcpy(lastpath,tmpcfg->fullpath);
		tmpcfg++;
	}
	xmlSaveFormatFile("/item/ups/log/111.xml",doc,1);
	xmlFreeDoc(doc);
	xmlCleanupParser();
	shmdt(dtcfg);
	return  0;
}
