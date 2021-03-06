#include "ups.h"
#include  <sys/resource.h>
void memset_var_hash(void);
void serv(int sig);
int iret = 0;
int msgidi=0,msgido=0,msgidr;
key_t key;
_msgbuf *mbuf=NULL;
_tran *tranbuf = NULL;

extern void prtusage(void);
/** 超时函数 **/
_tranmap tmap;
void servtimeout(int signal)
{
	seterr("TTTTTTTT","交易超时结束");
	/** 删除共享内存hash表中的交易信息 
    if(delete_shm_hash(mbuf->innerid)==-1)
    {
        SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:删除共享内存hash表数据失败\n",__FILE__,__LINE__);
		updatestat();
		return ;
    }
    SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:删除共享内存hash表数据成功\n",__FILE__,__LINE__);
	if(serv_flow("TIMEOUT")==0)
	{
    	SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:处理超时流程正常结束\n",__FILE__,__LINE__);
	}else
	{
    	SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:处理超时流程失败结束\n",__FILE__,__LINE__);
	}
	**/
}

void exit_free(void)
{
    SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:进程[%ld]退出运行\n",__FILE__,__LINE__,(long)getpid());
	free(tranbuf);
	free(mbuf);
}
/** 获取配套流程 **/
int	get_flow(char	*trancode,_flow	*localflow)
{
	if(localflow==NULL||trancode==NULL)
	{
		SysLog(LOG_SYS_ERR,"获取流程传入参数错误\n");
		return -1;
	}

	/** 根据交易码，获取到流程名称 **/
	SysLog(LOG_SYS_SHOW,"FILE[%s] LINE[%d] 传入交易码:%s\n",__FILE__,__LINE__,trancode);
	trim(trancode);
	if(gettranmap(&tmap,trancode)==-1)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE[%d] 根据交易码获取交易流程失败，请查看是否配置\n",__FILE__,__LINE__);
		seterr("EEEEEEEEEE","根据交易码获取交易流程失败，请查看是否配置");
		return -1;
	}
	SysLog(LOG_SYS_SHOW,"交易码[%s]交易名称[%s]交易流程名称[%s]超时时间[%d]\n",tmap.trancode,tmap.tranname,tmap.tranflow,tmap.timeout);
	
	/** 增加超时 **/
	/**修改hash表中超时时间为获取到的超时时间 **/
	alarm(tmap.timeout);	
	if(shm_hash_tout(mbuf->innerid,tmap.timeout)!=0)
	{
		SysLog(LOG_SYS_ERR,"设置[%ld]的超时时间失败\n",mbuf->innerid);
		return -1;
	}
	int shmid,loop=0;
	_flow *flow=NULL;
	_flow *tmpshmdt=NULL;
	size_t shmsize;

	SysLog(LOG_SYS_SHOW,"开始获取交易码为[%s]的流程配置\n",trancode);
	shmsize=MAXFLOW*sizeof(_flow);
	if((shmid=getshmid(8,shmsize))==-1)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE[%d] 获取共享内存号失败\n",__FILE__,__LINE__);
		seterr("EEEEEEEEEE","获取共享内存失败");
		return -1;
	}
	flow = (_flow *)shmat(shmid,NULL,0);
	if(flow == NULL)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE[%d]链接共享内存失败\n",__FILE__,__LINE__);
		seterr("EEEEEEEEEE","链接共享内存失败");
		return -1;
	}
	tmpshmdt = flow;
	SysLog(LOG_SYS_SHOW,"开始执行流程[%s] \n",tmap.tranflow);
	while(strcmp(flow->flowname,"END"))
	{
		if(!strcmp(flow->flowname,tmap.tranflow))
		{
			memcpy(localflow+loop,flow,sizeof(_flow));
			loop++;
		}
		flow++;
	}
	strcpy((localflow+loop)->flowname,"END");
	shmdt(tmpshmdt);
	return 0;
}
void delservpid(void)
{
	SysLog(LOG_SYS_SHOW,"开始删除共享内存数据[%ld]\n",getpid());
	pid_t ret = 0;
	int shmid = 0,i=0,semid = 0;
	_servreg *sreg = NULL;
	size_t shmsize = MAXSERVREG*sizeof(_servreg);
	if((shmid = getshmid(7,shmsize))==-1)
	{
		SysLog(LOG_SYS_ERR,"get serv shm id error\n");
		return ;
	}
	if((sreg = shmat(shmid,NULL,0))==NULL)
	{
		SysLog(LOG_SYS_ERR,"shmat sreg error\n");
		return ;
	}
	/** 信号量控制 **/
	for(i=0;i<MAXSERVREG;i++)
	{
		SysLog(LOG_SYS_DEBUG,"i[[[[]]]]]%d servpid [%d][%c]\n",i,(sreg+i)->servpid,(sreg+i)->stat[0]);
		if((sreg+i)->servpid==getpid())
		{
			sem_wait(&((sreg+i)->sem1));
			(sreg+i)->servpid='0';
			ret = 0;
			sem_post(&((sreg+i)->sem1));
			break;
		}

	}
	shmdt(sreg);
	return ;
}
int updatestat(void)
{
	pid_t ret = 0;
	int shmid = 0,i=0,semid = 0;
	_servreg *sreg = NULL;
	size_t shmsize = MAXSERVREG*sizeof(_servreg);
	if((shmid = getshmid(7,shmsize))==-1)
	{
		SysLog(LOG_SYS_ERR,"get serv shm id error\n");
		return -1;
	}
	SysLog(LOG_SYS_ERR,"shmid is[%d]\n",shmid);
	if((sreg = shmat(shmid,NULL,0))==NULL)
	{
		SysLog(LOG_SYS_ERR,"shmat sreg error\n");
		return -1;
	}
	for(i=0;i<MAXSERVREG;i++)
	{
		//SysLog(LOG_SYS_ERR,"i[[[[]]]]]%d servpid [%d][%c]\n",i,(sreg+i)->servpid,(sreg+i)->stat[0]);
		if((sreg+i)->servpid==getpid())
		{
			sem_wait(&((sreg+i)->sem1));
			(sreg+i)->stat[0]='N';
			ret = 0;
			sem_post(&((sreg+i)->sem1));
			break;
		}
		//sem_post(&((sreg+i)->sem1));

	}
	shmdt(sreg);
	SysLog(LOG_SYS_SHOW,"解除信号量成功\n");
	return ret;
}


int insert_servreg(char	*startcmd,char *chnlname )
{
	int shmid = 0,i=0;
	_servreg *sreg = NULL;
	size_t shmsize = MAXSERVREG*sizeof(_servreg);
	if((shmid = getshmid(7,shmsize))==-1)
	{
		SysLog(LOG_SYS_ERR,"get serv shm id error\n");
		return -1;
	}
	if((sreg = shmat(shmid,NULL,0))==NULL)
	{
		SysLog(LOG_SYS_ERR,"shmat sreg error\n");
		return -1;
	}
	for(i=0;i<MAXSERVREG;i++)
	{
		SysLog(LOG_SYS_DEBUG,"i[[[[]]]]]%d servpid [%d]\n",i,(sreg+i)->servpid);
		sem_wait(&((sreg+i)->sem2));
		if((sreg+i)->servpid==0)
		{
			(sreg+i)->servpid = getpid();
			strcpy((sreg+i)->chnlname,chnlname);
			strcpy((sreg+i)->startcmd,startcmd);
			(sreg+i)->stat[0]='N';
			(sreg+i)->type[0]='S';
			sem_post(&((sreg+i)->sem2));
			shmdt(sreg);
			return 0;
		}else if((kill((sreg+i)->servpid,SIGUSR1)==-1)&&(errno == ESRCH))
		{
			(sreg+i)->servpid = getpid();
			strcpy((sreg+i)->chnlname,chnlname);
			strcpy((sreg+i)->startcmd,startcmd);
			(sreg+i)->stat[0]='N';
			sem_post(&((sreg+i)->sem2));
			shmdt(sreg);
			return 0;
		}
		sem_post(&((sreg+i)->sem2));
	}
	shmdt(sreg);
	return -1;
}



int main(int argc,char *argv[])
{
	if(argc < 2)
	{
		printf("服务启动参数不正确:usage appname chnlname\n");
		return -1;
	}
	/** 初始化全局共享内存前，先获取ups根路径 **/
	if(setupshome()==-1)
	{
		printf("设置全局变量upshome错误,请检查UPSHOME环境变量是否设置\n");
		return -1;
	}

	atexit(memset_var_hash);
	atexit(exit_free);
	atexit(delservpid);

	char	startcmd[200];
	int		i=0;
	memset(startcmd,0x00,sizeof(startcmd));	
	mbuf = (_msgbuf *)malloc(sizeof(_msgbuf));
	if(mbuf == (void *)-1)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:申请msgbuf内存失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}

	tranbuf = (_tran *)malloc(sizeof(_tran));
	if(tranbuf == (void *)-1)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:申请tranbuf内存失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		free(mbuf);
		return -1;
	}

	iret = init_var_hash();
	if(iret != 0)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:初始化变量存放hash表失败\n",__FILE__,__LINE__);
		free(tranbuf);
		free(mbuf);
		return -1;
	}

	/** 渠道参数需要传入，服务根据渠道多少划分，防止一个服务挂掉所有都挂掉 **/
	if(getmsgid(argv[1],&msgidi,&msgido,&msgidr)!=0)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:获取测试渠道消息队列失败\n",__FILE__,__LINE__);
		free(tranbuf);
		free(mbuf);
		return -1;
	}

	/** 注册serv **/

	for(i=0;i<argc;i++)
	{
		if(strlen(argv[i])==0)
			break;
		strcat(startcmd," ");
		strcat(startcmd,argv[i]);
	}
	strcat(startcmd," ");
	strcat(startcmd,"&");
	if(insert_servreg(startcmd,argv[1])==0)
	{
		SysLog(LOG_SYS_SHOW,"FILE [%s] LINE [%d]:注册服务成功,PID[%ld]\n",__FILE__,__LINE__,getpid());
	}else
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:注册服务失败,PID[%ld]\n",__FILE__,__LINE__,getpid());
		free(tranbuf);
		free(mbuf);
		return -1;
	}
	
	/** 堵塞到信号 **/
	signal(SIGPIPE,SIG_IGN);
	signal(SIGUSR1,SIG_IGN);
	signal(SIGUSR2,serv);
	signal(SIGALRM,servtimeout);
	while (1)
	{
		pause();
	}
	return 0;
}
void serv(int sig)
{
	struct	timeval	t_start,t_end;
	float 	usetime;
	gettimeofday(&t_start,NULL);
	SysLog(LOG_SYS_SHOW,"FILE [%s] LINE [%d]:服务[%ld]获取到信号\n",__FILE__,__LINE__,getpid());
	if(init_malloced_hash()!=0)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d] 初始化变量存放区失败\n",__FILE__,__LINE__);
		return ;
	}
	SysLog(LOG_SYS_SHOW,"FILE [%s] LINE [%d] 初始化变量存放区成功\n",__FILE__,__LINE__);

	seterr("AAAAAAAA","交易正常结束");

	iret = msgrcv(msgido,mbuf,sizeof(mbuf->tranbuf),0,IPC_NOWAIT);
	if(iret > 0)
	{
		innerid = mbuf->innerid ; 
		trim(mbuf->tranbuf.chnlname);
		trim(mbuf->tranbuf.trancode);
		SysLog(LOG_APP_SHOW,"FILE [%s] LINE [%d] 处理来自[%20s]交易码为[%10s]长度为[%10ld]的交易\n",__FILE__,__LINE__,mbuf->tranbuf.chnlname,mbuf->tranbuf.trancode,mbuf->tranbuf.buffsize);
		SysLog(LOG_APP_SHOW,"FILE [%s] LINE[%d] 全局跟踪号为:[%ld]\n",__FILE__,__LINE__,innerid);
		if((get_shm_hash(mbuf->innerid,tranbuf))!=-1)
		{
			SysLog(LOG_APP_ERR,"交易跟踪号[%ld]\t传入交易信息[%s]\n",mbuf->innerid,tranbuf->intran);
			if(unpack(mbuf->tranbuf.chnlname,tranbuf->intran,"|")==-1)
			{
				SysLog(LOG_APP_ERR,"解[%s]包失败\t传入交易信息[%s]\n",mbuf->tranbuf.chnlname,tranbuf->intran);
				seterr("EEEEEEEE","解包失败");
			}else
			{
				if(serv_flow(mbuf->tranbuf.trancode)!=0)
				{
					SysLog(LOG_APP_ERR,"处理[%s]交易流程失败\n",mbuf->tranbuf.trancode);
					seterr("EEEEEEEE","执行交易失败");
				}else
				{
					SysLog(LOG_APP_SHOW,"处理[%s]交易流程成功\n",mbuf->tranbuf.trancode);
				}
			}
		}else
		{
			SysLog(LOG_SYS_ERR,"FILE [%s] LINE[%d] 获取全局跟踪号:[%ld]信息失败\n",__FILE__,__LINE__,innerid);
			gettimeofday(&t_end,NULL);
			usetime=((float)(t_end.tv_sec*1000000+t_end.tv_usec-(t_start.tv_sec*1000000+t_start.tv_usec)))/1000000;
			SysLog(LOG_APP_SHOW,"FILE [%s] LINE[%d] 交易失败结束开始[%ld]结束[%ld],耗时[+%10f]秒\n",__FILE__,__LINE__,t_start.tv_usec,t_end.tv_usec,usetime);
			updatestat();
			return ;
		}
	}else if(errno  == ENOMSG)
	{
		SysLog(LOG_SYS_ERR,"获取不到交易信息\n");
		seterr("EEEEEEEE","获取不到交易信息");
	}else if(errno  !=ENOMSG )
	{
		SysLog(LOG_SYS_ERR,"其他错误\n");
		seterr("EEEEEEEE","其他错误");
	}else
	{
		SysLog(LOG_SYS_ERR,"其他错误\n");
		seterr("EEEEEEEE","其他错误");
	}
	/** 删除共享内存hash表中的交易信息 **/
	if(delete_shm_hash(mbuf->innerid)==-1)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:删除共享内存hash表数据失败\n",__FILE__,__LINE__);
		gettimeofday(&t_end,NULL);
		usetime=((float)(t_end.tv_sec*1000000+t_end.tv_usec-(t_start.tv_sec*1000000+t_start.tv_usec)))/1000000;
		SysLog(LOG_APP_SHOW,"FILE [%s] LINE[%d] 交易失败结束开始[%ld]结束[%ld],耗时[+%10f]秒\n",__FILE__,__LINE__,t_start.tv_usec,t_end.tv_usec,usetime);
		updatestat();
		alarm(0);
		return ;
	}
	SysLog(LOG_SYS_SHOW,"FILE [%s] LINE [%d]:删除共享内存hash表数据成功\n",__FILE__,__LINE__);
	if(delete_msgq(mbuf->innerid)==-1)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:删除消息队列数据失败\n",__FILE__,__LINE__);
		gettimeofday(&t_end,NULL);
		usetime=((float)(t_end.tv_sec*1000000+t_end.tv_usec-(t_start.tv_sec*1000000+t_start.tv_usec)))/1000000;
		SysLog(LOG_APP_SHOW,"FILE [%s] LINE[%d] 交易失败结束开始[%ld]结束[%ld],耗时[+%10f]秒\n",__FILE__,__LINE__,t_start.tv_usec,t_end.tv_usec,usetime);
		updatestat();
		alarm(0);
		return ;
	}
	SysLog(LOG_SYS_SHOW,"FILE [%s] LINE [%d]:删除消息队列数据成功\n",__FILE__,__LINE__);
	gettimeofday(&t_end,NULL);
	usetime=((float)(t_end.tv_sec*1000000+t_end.tv_usec-(t_start.tv_sec*1000000+t_start.tv_usec)))/1000000;
	SysLog(LOG_APP_SHOW,"FILE [%s] LINE[%d] 交易成功结束[%ld]结束[%ld],耗时[+%10f]秒\n",__FILE__,__LINE__,t_start.tv_usec,t_end.tv_usec,usetime);
	/**修改状态为空闲 **/
	updatestat();
	alarm(0);
	return ;
}
int serv_flow(char *trancode)
{
	struct	timeval	t_start,t_end;
	if(trancode==NULL)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE[%d]获取交易码失败\n",__FILE__,__LINE__);
		seterr("EEEEEEEEEE","获取交易码失败");
		return -1;
	}
	_flow	localflow[1024];
	memset(localflow,0,sizeof(localflow));

	int i=1,j=1;
	/** 获取流程 **/
	if(get_flow(trancode,localflow)!=0)
	{
		SysLog(LOG_SYS_ERR,"获取交易码为[%s]的流程失败\n",trancode);
		return -1;
	}
	SysLog(LOG_APP_SHOW,"交易码[%s]流程开始\n",trancode);
	for(i=1;strcmp(localflow[i].flowname,"END");i++)
	{
		SysLog(LOG_APP_ERR,"执行流程序号[%-3d]\t流程名称[%-20s]函数名称[%-20s]\n",i,localflow[i].flowname,localflow[i].flowfunc);
	}
	SysLog(LOG_APP_SHOW,"交易码[%s]流程结束\n",trancode);
	i=1;
	/** init commmsg **/
	while(strcmp(localflow[i].flowname,"END"))
	{
		gettimeofday(&t_start,NULL);
		SysLog(LOG_APP_SHOW,"开始处理流程flowname[%-20s]库[%-20s]函数[%-20s]参数[%-20s]\t\n",localflow[i].flowname,localflow[i].flowso,localflow[i].flowfunc,localflow[i].funcpar1);
		trim(localflow[i].flowso);
		trim(localflow[i].flowfunc);
		trim(localflow[i].funcpar1);
		if(do_so(localflow[i].flowso,localflow[i].flowfunc,localflow[i].funcpar1)==0)
		{
			gettimeofday(&t_end,NULL);
			SysLog(LOG_APP_SHOW,"流程处理成功,耗时[+%10f]秒\n",((float)(t_end.tv_sec*1000000+t_end.tv_usec-(t_start.tv_sec*1000000+t_start.tv_usec)))/1000000);
		}else
		{
			gettimeofday(&t_end,NULL);
			SysLog(LOG_APP_ERR,"ERROR!!![流程处理失败,耗时[+%10f]秒]ERROR\n",((float)(t_end.tv_sec*1000000+t_end.tv_usec-(t_start.tv_sec*1000000+t_start.tv_usec)))/1000000);
			/** 执行错误流程**/
			if(get_flow(localflow[i].errflow,localflow)!=0)
			{
				SysLog(LOG_APP_ERR,"获取交易码为[%s]的流程失败\n",trancode);
				return -1;
			}
			SysLog(LOG_APP_ERR,"交易码[%s]错误/超时流程开始\n",trancode);
			for(j=1;strcmp(localflow[j].flowname,"END");j++)
			{
				SysLog(LOG_APP_ERR,"执行流程序号[%-3d]\t流程名称[%-20s]函数名称[%-20s]\n",j,localflow[j].flowname,localflow[j].flowfunc);
			}
			SysLog(LOG_APP_SHOW,"交易码[%s]错误/超时流程结束\n",trancode);
			i=1;
			continue;
		}
		i++;
	}
	return 0;
}
