/** XML 解包函数 **/
#include "ups.h"
#include <libxml/tree.h>
#include <libxml/parser.h>

int loop=0;
_xmlcfg *xmlcfg = NULL;
int upnew(char *a)
{
	//fprintf(stderr,"into unnew 1\n");
	//prtusage();
	int iret;
	char	msgtype[30];
	char	msgtypefile[60];

	memset(msgtype,0,sizeof(msgtype));
	memset(msgtypefile,0,sizeof(msgtypefile));

	if(!strcmp(a,"V"))
	{
		SysLog(1,"从变量V_MSGTYPE取报文类型进行处理");
		iret = get_var_value("V_MSGTYPE",sizeof(msgtype),1,msgtype);
		if(iret == -1)
		{
			SysLog(1,"从变量V_MSGTYPE取报文类型进行处理失败");
			return  -1;
		}
		SysLog(1,"获取到待解包报文类型[%s]",msgtype);
		trim(msgtype);

		SysLog(1,"从变量V_XMLFILE读取报文进行处理");
		iret = get_var_value("V_XMLFILE",sizeof(msgtypefile),1,msgtypefile);
		if(iret == -1)
		{
			SysLog(1,"从变量V_XMLFILE读取报文进行处理失败");
			return  -1;
		}
		SysLog(1,"获取到待处理报文[%s]",msgtypefile);
		trim(msgtypefile);
		if(unpack_xml(msgtype,msgtypefile)==-1)
		{
			SysLog(1,"解报文类型[%s]失败",msgtype);
			return  -1;
		}
		return  0;
	}
	sprintf(msgtypefile,"%s/%s_1.xml","/item/ups/src/unpack",a);
	if(unpack_xml(a,msgtypefile)==-1)
	{
		SysLog(1,"解报文类型[%s]失败",a);
		return  -1;
	}
	//fprintf(stderr,"into unnew 2\n");
	//prtusage();
	SysLog(1,"unpack xml ok");
	return 0;
}
int unpack_xml(char *xmltype,char *filename)
{
	//fprintf(stderr,"into unpack_xml 1\n");
	//prtusage();
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
	//fprintf(stderr,"into unpack_xml 2\n");
	//prtusage();

	xmlKeepBlanksDefault(0);
	doc = xmlReadFile(filename,"UTF-8",XML_PARSE_RECOVER);
	if(doc == NULL)
	{
		SysLog(1,"parse file error\n");
		return -1;
	}
	//fprintf(stderr,"into unpack_xml 3\n");
	//prtusage();

	curNode = xmlDocGetRootElement(doc);
	if(curNode == NULL)
	{
		SysLog(1,"get root elemenet error\n");
		xmlFreeDoc(doc);
		return -1;
	}
	//fprintf(stderr,"into unpack_xml 4\n");
	//prtusage();

	if(prtvalue(curNode,xmltype)!=-1)
	{
		SysLog(1,"解包到变量成功\n");
	//fprintf(stderr,"into unpack_xml 5\n");
	//prtusage();
	}else
	{
		SysLog(1,"解包到变量失败\n");
	//fprintf(stderr,"into unpack_xml 6\n");
	//prtusage();
	}
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();
	shmdt(xmlcfg);
	//fprintf(stderr,"into unpack_xml 7\n");
	//prtusage();
	return 0;
}
int prtvalue(xmlNodePtr cur,char *xmltype)
{
	//fprintf(stderr,"开始进入prtvalue1\n");
	//prtusage();
	xmlChar	*szKey;
	xmlNodePtr curNode ;
	curNode = cur;
	char	path[256];
	memset(path,0,sizeof(path));
	_xmlcfg *tmpcfg=xmlcfg;
	//fprintf(stderr,"into prtvalue 2\n");
	//prtusage();

	while(curNode!=NULL)
	{
		if(curNode->type == XML_ELEMENT_NODE)
		{
			SysLog(1,"ELement name [%s]\n",curNode->name);
		}else if(curNode->type == XML_TEXT_NODE)
		{
	//fprintf(stderr,"[%s]into prtvalue 3\n",szKey);
	//prtusage();
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
							xmlFree(szKey);
							SysLog(1,"put error\n");
							return -1;
						}
						xmlFree(szKey);
						loop++;
						break;
					}else
					{
						//SysLog(1,"FILE [%s] LINE[%d]路径[%s]变量名[%s]变量值[%s]属性[%d]\n",__FILE__,__LINE__,tmpcfg->fullpath,tmpcfg->varname,szKey,tmpcfg->depth);
						if(put_var_value(tmpcfg->varname,strlen(szKey)+1,1,szKey)!=0)
						{
							xmlFree(szKey);
							SysLog(1,"put error\n");
							return -1;
						}
						xmlFree(szKey);
						/** 防止多与的循环，找到后直接跳出循环 **/
						break;
					}
				}
				tmpcfg++;
			}
	//fprintf(stderr,"into prtvalue 4\n");
	//prtusage();
		}
		prtvalue(curNode->xmlChildrenNode,xmltype);
		curNode = curNode->next;
	}
	//fprintf(stderr,"结束退出prtvalue4\n");
	//prtusage();
	return  0;
}
