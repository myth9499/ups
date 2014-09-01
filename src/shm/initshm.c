#include "ups.h"

int	initmsg(char	*chnlname)
{
	char	keypath[60];
	char	tmptouch[66];
	memset(keypath,0,sizeof(keypath));
	memset(tmptouch,0,sizeof(tmptouch));
	sprintf(keypath,"%s%s","/item/ups/etc/",chnlname);
	sprintf(tmptouch,"touch %s",keypath);
	system(tmptouch);
	key_t	key;
	int msgid;
	/** 发送到核心队列**/
	if((key=ftok(keypath,1))==-1)
	{
		SysLog(1,"获取渠道[%s]消息队列key值失败[%s]",chnlname,strerror(errno));
		return -1;
	}
	if((msgid = msgget(key,IPC_CREAT|IPC_EXCL|00666))==-1)
	{
		SysLog(1,"获取渠道[%s]消息队列失败[%s]",chnlname,strerror(errno));
		return -1;
	}
	SysLog(1,"FILE[%s]LINE[%d]渠道[%s]发送核心消息队列为[%d]\n",__FILE__,__LINE__,chnlname,msgid);
	/**接收核心返回队列**/
	if((key=ftok(keypath,2))==-1)
	{
		SysLog(1,"获取渠道[%s]消息队列key值失败[%s]",chnlname,strerror(errno));
		return -1;
	}
	if((msgid = msgget(key,IPC_CREAT|IPC_EXCL|00666))==-1)
	{
		SysLog(1,"获取渠道[%s]消息队列失败[%s]",chnlname,strerror(errno));
		return -1;
	}
	SysLog(1,"FILE[%s]LINE[%d]渠道[%s]接收核心消息队列为[%d]\n",__FILE__,__LINE__,chnlname,msgid);
	/** 返回核心应答队列**/
	if((key=ftok(keypath,3))==-1)
	{
		SysLog(1,"获取渠道[%s]消息队列key值失败[%s]",chnlname,strerror(errno));
		return -1;
	}
	if((msgid = msgget(key,IPC_CREAT|IPC_EXCL|00666))==-1)
	{
		SysLog(1,"获取渠道[%s]消息队列失败[%s]",chnlname,strerror(errno));
		return -1;
	}
	SysLog(1,"FILE[%s]LINE[%d]渠道[%s]返回核心应答消息队列为[%d]\n",__FILE__,__LINE__,chnlname,msgid);
	return  0;
}
int main(int argc,char *argv[])
{
	size_t shmsize = HASHCNT*BUCKETSCNT*sizeof(_tran);
	FILE *fp;
	char	tmpbuf[60];
	memset(tmpbuf,0,sizeof(tmpbuf));

	/** init tran shm **/
	if(getshm(10,shmsize)==-1)
	{
		SysLog(1,"获取交易hash桶共享内存ID失败\n");
		return -1;
	}
	/** init commmsg **/
	shmsize=MAXCOMMMSG*sizeof(_commmsg);
	if(getshm(9,shmsize)==-1)
	{
		SysLog(1,"获取渠道间通信共享内存ID失败\n");
		return -1;
	}
	/** init flow **/
	shmsize=MAXFLOW*sizeof(_flow);
	if(getshm(8,shmsize)==-1)
	{
		SysLog(1,"获取流程配置区共享内存ID失败\n");
		return -1;
	}
	/** init server reg **/
	shmsize = MAXSERVREG*sizeof(_servreg);
	if(getshm(7,shmsize)==-1)
	{
		SysLog(1,"获取交易系统服务登记共享内存ID失败\n");
		return -1;
	}
	/** init xml cfg  **/
	shmsize = MAXXMLCFG*sizeof(_xmlcfg);
	if(getshm(6,shmsize)==-1)
	{
		SysLog(1,"获取XML配置共享内存失败\n");
		return -1;
	}
	/** init tran map cfg  **/
	shmsize = MAXTRANMAP*sizeof(_tranmap);
	if(getshm(5,shmsize)==-1)
	{
		SysLog(1,"获取交易码映射配置共享内存失败\n");
		return -1;
	}

	/** init the vardef shm cfg **/
	shmsize = MAXVARDEF*sizeof(_vardef);
	if(getshm(4,shmsize)==-1)
	{
		SysLog(1,"获取变量定义映射配置共享内存失败\n");
		return -1;
	}

	/** 初始化系统所有渠道的队列区 **/
	fp = fopen("/item/ups/src/cfg/chnl.cfg","r");
	if(fp == NULL)
	{
		SysLog(1,"打开渠道初始化配置文件失败:[%s]\n",strerror(errno));
		return -1;
	}
	while(fgets(tmpbuf,sizeof(tmpbuf),fp)!=NULL)
	{
		tmpbuf[strlen(tmpbuf)-1]='\0';
		if(tmpbuf[0]=='*')
			continue;
		else if(tmpbuf[0]=='@')
		{
			if(initmsg(tmpbuf+1)!=0)
			{
				SysLog(1,"初始化:[%s]渠道队列区失败\n",tmpbuf+1);
				return -1;
			}
			SysLog(1,"初始化:[%s]渠道队列区成功\n",tmpbuf+1);
		}else
		{
			continue;
		}
	}
	fclose(fp);

	/** init sem **/
	if(initservregsem()!=0)
	{
		SysLog(1,"获取服务登记区信号灯失败\n");
		return -1;
	}else
	{
		SysLog(1,"获取服务登记区信号灯成功\n");
	}
	/** system v sem 
	if(init_sem(1)==0)
	{
		SysLog(1,"init sem ok\n");
	}else
	{
		SysLog(1,"init sem error\n");
		return -1;
	}
	**/
	return  0;
}
