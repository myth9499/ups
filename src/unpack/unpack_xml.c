/** XML 解包函数 **/
#include "ups.h"
#include <libxml/tree.h>
#include <libxml/parser.h>

_xmlcfg *xmlcfg = NULL;
int upnew(char *a)
{
	if(unpack_xml("hvps.111.001.01","/item/ups/src/unpack/hvps.111.001.01_1.xml")==-1)
	{
		printf("unpack xml error");
		return -1;
	}
		printf("unpack xml ok");
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
		printf("get xml cfg error\n");
		return -1;
	}
	if((xmlcfg = shmat(shmid,NULL,0))==(void *)-1)
	{
		printf("shmat xml cfg error\n");
		return -1;
	}

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
	if(prtvalue(curNode,xmltype)!=-1)
	{
		printf("解包到变量成功\n");
	}else
	{
		printf("解包到变量失败\n");
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
			printf("ELement name [%s]\n",curNode->name);
		}else if(curNode->type == XML_TEXT_NODE)
		{
			szKey = xmlNodeGetContent(curNode);
			getNodePath(path,curNode);
			while(strcmp(tmpcfg->xmlname,""))
			{
//				printf("xmlnames[%s]fullpath[%s]path[%s]xmltype[%s]\n",tmpcfg->xmlname,tmpcfg->fullpath,path,xmltype);
				if(!strcmp(tmpcfg->fullpath,path)&&!strcmp(tmpcfg->xmlname,xmltype))
				{
					printf("路径[%s]变量名[%s]变量值[%s]\n",tmpcfg->fullpath,tmpcfg->varname,szKey);
					if(put_var_value(tmpcfg->varname,strlen(szKey)+1,1,szKey)!=0)
					{
						printf("put error\n");
						return -1;
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
