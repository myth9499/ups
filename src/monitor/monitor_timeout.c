#include "ups.h"

/** 监控交易表中的交易信息，发现超过超时时间后进行一下动作
 * 1、将该条交易信息登记初始化
 * 2、轮询查找所有消息队列，确认是否存在该交易对应的消息未清除
 **/

int main(int argc,char *argv[] )
{
	/** 初始化全局共享内存前，先获取ups根路径 **/
	if(setupshome()==-1)
	{
		printf("设置全局变量upshome错误,请检查UPSHOME环境变量是否设置\n");
		return -1;
	}

	int shmid = 0,i=0;
	_tran *tran = NULL;
	time_t	curtime;
	long	inpid=0;
	int shmsize = HASHCNT*BUCKETSCNT*sizeof(_tran);
	if((shmid = getshmid(10,shmsize))==-1)
	{
		SysLog(LOG_SYS_ERR,"FILE[%s] LINE[%d] get shmid error\n",__FILE__,__LINE__);
		return -1;
	}
	if((tran = shmat(shmid,NULL,0))==NULL)
	{
		SysLog(LOG_SYS_ERR,"FILE[%s] LINE[%d] shmat shmid error\n",__FILE__,__LINE__);
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
				/** 获取当前时间 超时监控处理长时间超时情况，默认情况由serv自己执行超时处理 **/
				time(&curtime);
				if(curtime-((tran+i)->stime)>(tran+i)->timeout*2)
				{
					/** 上锁当前共享内存 **/
					sem_wait(&((tran+i)->sem1));
					SysLog(LOG_SYS_ERR,"FILE[%s] LINE[%d]PID[%ld]超时超时时间[%ld]，需要进行处理\n",__FILE__,__LINE__,(tran+i)->innerid,(tran+i)->timeout);
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
						SysLog(LOG_SYS_ERR,"FILE[%s] LINE[%d]删除跟踪号[%ld]对应队列失败\n",__FILE__,__LINE__,inpid);
					}
				}
				continue;
			}
		}
		SysLog(LOG_SYS_ERR,"FILE[%s] LINE[%d]当前无服务超时\n",__FILE__,__LINE__);
		/** 监控是否有进程退出运行 **/
		viewexit();
		sleep(5);
	}
	shmdt(tran);
	return 0;
}
/** 查看是否有异常退出进程，若存在异常退出进程，进行自动重启动 **/
int	viewexit()
{
	int shmid = 0,i=0,ret,result=0;
	_servreg *sreg = NULL;
	int shmsize = MAXSERVREG*sizeof(_servreg);
	if((shmid = getshmid(7,shmsize))==-1)
	{
		printf("get serv shm id error\n");
		return -1;
	}
	if((sreg = shmat(shmid,NULL,0))==NULL)
	{
		printf("shmat sreg error\n");
		return -1;
	}
	for(i=0;(sreg+i)->servpid!=0;i++)
	{
		if((kill((sreg+i)->servpid,SIGUSR1)==-1)&&(errno == ESRCH))
		{
			result = 1;
			SysLog(LOG_SYS_ERR,"FILE[%s] LINE[%d] 渠道[%s] 进程[%ld]退出运行，需要重新启动\n",__FILE__,__LINE__,(sreg+i)->chnlname,(sreg+i)->servpid);
			ret = system((sreg+i)->startcmd);
			if(WEXITSTATUS(ret)!=0||ret ==-1)
			{
				SysLog(LOG_SYS_ERR,"启动渠道[%s]服务 启动命令[%s]失败\n",(sreg+i)->chnlname,(sreg+i)->startcmd);
			}
		}
	}
	if(result == 0)
	{
		SysLog(LOG_SYS_ERR,"FILE[%s] LINE[%d] 当前无进程异常退出\n",__FILE__,__LINE__);
	}
	shmdt(sreg);
	return  0;
}
