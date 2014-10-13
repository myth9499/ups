#include "ups.h"


int main(int argc,char *argv[] )
{
	FILE	*fp =NULL;
	char	servcfgpath[60];
	char	buffer[1024];
	int 	i;

	char	chnlname[60];
	char	systemcmd[100];
	char	servbuf[100];
	int		servcnt;
	int		ret;

	memset(servcfgpath,0,sizeof(servcfgpath));
	memset(chnlname,0,sizeof(chnlname));
	memset(systemcmd,0,sizeof(systemcmd));
	memset(servbuf,0,sizeof(servbuf));
	memset(buffer,0,sizeof(buffer));

	strcpy(servcfgpath,"/item/ups/src/cfg/serv.cfg");

	fp = fopen(servcfgpath,"r");
	if(fp == NULL)
	{
		printf("打开文件[%s]失败:%s\n",servcfgpath,strerror(errno));
		return -1;
	}
	memset(buffer,0,sizeof(buffer));
	while(fgets(buffer,sizeof(buffer),fp)!=NULL)
	{
		buffer[strlen(buffer)-1]='\0';
		if(buffer[0]=='*')
		{
			continue;
		}
		strcpy(chnlname,strtok(buffer,"^"));
		strcpy(systemcmd,strtok(NULL,"^"));
		servcnt = atoi(strtok(NULL,"^"));
		printf("渠道[%s]\t执行命令[%s]\t启动服务数[%d]\n",chnlname,systemcmd,servcnt);
		if(!strcmp(systemcmd,"NULL"))
		{
			printf("后台自动触发进程，无需专门启动\n");
		}else
		{
			ret = system(systemcmd);
			/** 注意：system函数调用后，只有当ret!=-1&&WEXITSTATUS(ret)==0&&WIFEXITED(ret)==0时才算被执行程序执行成功 **/
			//if(WEXITSTATUS(ret)!=0||WIFEXITED(ret)!=0)
			if(WEXITSTATUS(ret)!=0||ret ==-1)
			{
				printf("渠道[%s]\t执行命令[%s]\t启动服务数[%d]启动失败\n",chnlname,systemcmd,servcnt);
				fclose(fp);
				return -1;
			}
		}

		if(servcnt==0)
		{
			printf("渠道[%s]\t执行命令[%s]\t启动成功不需要启动服务\n",chnlname,systemcmd);
			continue;
		}
		printf("渠道[%s]\t执行命令[%s]\t启动服务数[%d]启动成功...开始启动服务\n",chnlname,systemcmd,servcnt);
		sprintf(servbuf,"%s %s &","/item/ups/src/server/server ",chnlname);
		for(i=0;i<servcnt;i++)
		{
			printf("开始启动渠道[%s]第[%d]个服务 启动命令[%s]\n",chnlname,i,servbuf);
			ret = system(servbuf);
			//if(WEXITSTATUS(ret)!=0||WIFEXITED(ret)!=0)
			if(WEXITSTATUS(ret)!=0||ret ==-1)
			//if(system(servbuf)==-1)
			{
				printf("启动渠道[%s]第[%d]个服务 启动命令[%s]失败\n",chnlname,i,servbuf);
				fclose(fp);
				return -1;
			}
			printf("启动渠道[%s]第[%d]个服务 启动命令[%s]成功\n",chnlname,i,servbuf);
		}
		printf("渠道[%s]\t执行命令[%s]\t启动服务数[%d]启动成功\n",chnlname,systemcmd,servcnt);
		memset(buffer,0,sizeof(buffer));
		//sleep (2);
	}
	fclose(fp);
	return 0;
}

