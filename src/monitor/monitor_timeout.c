#include "ups.h"

/** 监控交易表中的交易信息，发现超过超时时间后进行一下动作
 * 1、将该条交易信息登记初始化
 * 2、轮询查找所有消息队列，确认是否存在该交易对应的消息未清除
 **/


int main(int argc,char *argv[] )
{
	int shmid = 0,i=0;
	_tran *tran = NULL;
	time_t	curtime;
	long	inpid=0;
	int shmsize = HASHCNT*BUCKETSCNT*sizeof(_tran);
	if((shmid = getshmid(10,shmsize))==-1)
	{
		SysLog(1,"FILE[%s] LINE[%d] get shmid error\n",__FILE__,__LINE__);
		return -1;
	}
	printf("shmid is[%d]\n",shmid);
	if((tran = shmat(shmid,NULL,0))==NULL)
	{
		SysLog(1,"FILE[%s] LINE[%d] shmat shmid error\n",__FILE__,__LINE__);
		return -1;
	}
	while(1)
	{
		for(i=0;i<HASHCNT*BUCKETSCNT;i++)
		{
			if((tran+i)->stime==0)
				continue;
			else
			{
				/** 获取当前时间 **/
				time(&curtime);
				if(curtime-((tran+i)->stime)>60)
				{
					/** 上锁当前共享内存 **/
					sem_wait(&((tran+i)->sem1));
					SysLog(1,"FILE[%s] LINE[%d]PID[%ld]超时，需要进行处理\n",__FILE__,__LINE__,(tran+i)->innerid);
					inpid = (tran+i)->innerid;
					(tran+i)->innerid =0;
					memset((tran+i)->intran,0,sizeof((tran+i)->intran));
					memset((tran+i)->outtran,0,sizeof((tran+i)->outtran));
					(tran+i)->stime =0;
					(tran+i)->etime =0;
					memset((tran+i)->stat,0,sizeof((tran+i)->stat));
					sem_post(&((tran+i)->sem1));
					/** 第二步，查看当前跟踪号是否有未释放的消息队列，有的话进行释放 **/
					if(delete_msgq(inpid)!=0)
					{
						SysLog(1,"FILE[%s] LINE[%d]删除跟踪号[%ld]对应队列失败\n",__FILE__,__LINE__,inpid);
					}
				}
				continue;
			}
		}
		SysLog(1,"FILE[%s] LINE[%d]当前无服务超时\n",__FILE__,__LINE__);
		sleep(5);
	}
	shmdt(tran);
	return 0;
}
int	delete_msgq(long	inpid)
{
	/***轮询所有渠道对应消息队列，查看是否有未释放的消息队列数据 ***/
	int shmid = 0,i=0;
	int	msgidi,msgido,msgidr;
	_servreg *sreg = NULL;
	_msgbuf	mbuf;
	int shmsize = MAXSERVREG*sizeof(_servreg);
	if((shmid = getshmid(7,shmsize))==-1)
	{
		SysLog(1,"get serv shm id error\n");
		return -1;
	}
	if((sreg = shmat(shmid,NULL,0))==NULL)
	{
		SysLog(1,"shmat sreg error\n");
		return -1;
	}
	for(i=0;(sreg+i)->servpid!=0;i++)
	{
		if(!strcmp((sreg+i)->type,"C"))
		{
			SysLog(1,"开始查找进程名称:%s\t进程类型:%s\t进程号:%ld\t进程状态:%s\t\n",(sreg+i)->chnlname,(sreg+i)->type,(sreg+i)->servpid,(sreg+i)->stat);
			if((getmsgid((sreg+i)->chnlname),&msgidi,&msgido,&msgidr)!=0)
			{
				SysLog(1,"获取渠道:%s消息队列失败\t\n",(sreg+i)->chnlname);
				continue;
			}
			if(msgrcv(msgidi,&mbuf,sizeof(mbuf.tranbuf),inpid,0)==-1)
			{
				SysLog(1,"FILE [%s] LINE [%d]:删除消息队列数据失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
			}
			if(msgrcv(msgido,&mbuf,sizeof(mbuf.tranbuf),inpid,0)==-1)
			{
				SysLog(1,"FILE [%s] LINE [%d]:删除消息队列数据失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
			}
			if(msgrcv(msgidr,&mbuf,sizeof(mbuf.tranbuf),inpid,0)==-1)
			{
				SysLog(1,"FILE [%s] LINE [%d]:删除消息队列数据失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
			}
			SysLog(1,"FILE [%s] LINE [%d]:删除消息队列数据成功 inpid[%s]\n",__FILE__,__LINE__,inpid);
		}
	}
	shmdt(sreg);
	return 0;

}

