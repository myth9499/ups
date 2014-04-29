/** XML 解包函数 **/
#include "ups.h"
#include <libxml/tree.h>
#include <libxml/parser.h>

int loop=0;
_xmlcfg *xmlcfg = NULL;
int upnew(char *a)
{
	if(unpack_xml("hvps.111.001.01","/item/ups/src/unpack/hvps.111.001.01_1.xml")==-1)
	{
		SysLog(1,"unpack xml error");
		return -1;
	}
		SysLog(1,"unpack xml ok");
return 0;
}
int unpack_xml(char *xmltype,char *filename)
{
	int shmid = 0;
	xmlDocPtr doc;
	xmlNodePtr	curNode;
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
	if(prtvalue(curNode,xmltype)!=-1)
	{
		SysLog(1,"解包到变量成功\n");
	}else
	{
		SysLog(1,"解包到变量失败\n");
	}
	xmlFreeDoc(doc);
	shmdt(xmlcfg);
}
int prtvalue(xmlNodePtr cur,char *xmltype)
{
	xmlChar	*szKey;
	xmlNodePtr curNode ;
	curNode = cur;
	char	path[256];
	memset(path,0,sizeof(path));
	_xmlcfg *tmpcfg=xmlcfg;

	while(curNode!=NULL)
	{
		if(curNode->type == XML_ELEMENT_NODE)
		{
			SysLog(1,"ELement name [%s]\n",curNode->name);
		}else if(curNode->type == XML_TEXT_NODE)
		{
			szKey = xmlNodeGetContent(curNode);
			getNodePath(path,curNode);
			while(strcmp(tmpcfg->xmlname,""))
			{
				//SysLog(1,"LILEI FILE [%s] LINE[%d]路径[%s]变量名[%s]变量值[%s]属性[%d]\n",__FILE__,__LINE__,tmpcfg->fullpath,tmpcfg->varname,szKey,tmpcfg->depth);
				if(!strcmp(tmpcfg->fullpath,path)&&!strcmp(tmpcfg->xmlname,xmltype))
				{
						SysLog(1,"FILE [%s] LINE[%d]curname[%s]\n",__FILE__,__LINE__,curNode->parent->name);
						SysLog(1,"FILE [%s] LINE[%d]curname[%s]\n",__FILE__,__LINE__,curNode->name);
					if(!strcmp(curNode->parent->name,"Ustrd"))
					{
						//SysLog(1,"FILE [%s] LINE[%d]路径[%s]变量名[%s]变量值[%s]属性[%d]\n",__FILE__,__LINE__,(tmpcfg+loop)->fullpath,(tmpcfg+loop)->varname,szKey,(tmpcfg+loop)->depth);
						if(put_var_value((tmpcfg+loop)->varname,strlen(szKey)+1,1,szKey)!=0)
						{
							SysLog(1,"put error\n");
							return -1;
						}
						loop++;
						break;
					}else
					{
						//SysLog(1,"FILE [%s] LINE[%d]路径[%s]变量名[%s]变量值[%s]属性[%d]\n",__FILE__,__LINE__,tmpcfg->fullpath,tmpcfg->varname,szKey,tmpcfg->depth);
						if(put_var_value(tmpcfg->varname,strlen(szKey)+1,1,szKey)!=0)
						{
							SysLog(1,"put error\n");
							return -1;
						}
					}
				}
				tmpcfg++;
			}
			xmlFree(szKey);
		}
		prtvalue(curNode->xmlChildrenNode,xmltype);
		curNode = curNode->next;
	}
	return  0;
}
