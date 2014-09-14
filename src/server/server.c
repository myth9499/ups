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
        SysLog(1,"FILE [%s] LINE [%d]:删除共享内存hash表数据失败\n",__FILE__,__LINE__);
		updatestat();
		return ;
    }
    SysLog(1,"FILE [%s] LINE [%d]:删除共享内存hash表数据成功\n",__FILE__,__LINE__);
	**/
}

void exit_free(void)
{
	free(tranbuf);
	free(mbuf);
}
/** 获取配套流程 **/
int	get_flow(char	*trancode,_flow	*localflow)
{
	if(localflow==NULL||trancode==NULL)
	{
		SysLog(1,"获取流程传入参数错误\n");
		return -1;
	}

	/** 根据交易码，获取到流程名称 **/
	SysLog(1,"FILE[%s] LINE[%d] 传入交易码:%s\n",__FILE__,__LINE__,trancode);
	trim(trancode);
	if(gettranmap(&tmap,trancode)==-1)
	{
		SysLog(1,"FILE [%s] LINE[%d] 根据交易码获取交易流程失败，请查看是否配置\n",__FILE__,__LINE__);
		seterr("EEEEEEEEEE","根据交易码获取交易流程失败，请查看是否配置");
		return -1;
	}
	SysLog(1,"交易码[%s]交易名称[%s]交易流程名称[%s]超时时间[%d]\n",tmap.trancode,tmap.tranname,tmap.tranflow,tmap.timeout);
	
	/** 增加超时 **/
	alarm(tmap.timeout);	
	int shmid,loop=0;
	_flow *flow=NULL;
	_flow *tmpshmdt=NULL;
	size_t shmsize;

	SysLog(1,"开始获取交易码为[%s]的流程配置\n",trancode);
	shmsize=MAXFLOW*sizeof(_flow);
	if((shmid=getshmid(8,shmsize))==-1)
	{
		SysLog(1,"FILE [%s] LINE[%d] 获取共享内存号失败\n",__FILE__,__LINE__);
		seterr("EEEEEEEEEE","获取共享内存失败");
		return -1;
	}
	flow = (_flow *)shmat(shmid,NULL,0);
	if(flow == NULL)
	{
		SysLog(1,"FILE [%s] LINE[%d]链接共享内存失败\n",__FILE__,__LINE__);
		seterr("EEEEEEEEEE","链接共享内存失败");
		return -1;
	}
	tmpshmdt = flow;
	SysLog(1,"开始执行流程[%s] \n",tmap.tranflow);
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
	SysLog(1,"开始删除共享内存数据[%ld]\n",getpid());
	pid_t ret = 0;
	int shmid = 0,i=0,semid = 0;
	_servreg *sreg = NULL;
	int shmsize = MAXSERVREG*sizeof(_servreg);
	if((shmid = getshmid(7,shmsize))==-1)
	{
		SysLog(1,"get serv shm id error\n");
		return ;
	}
	SysLog(1,"shmid is[%d]\n",shmid);
	if((sreg = shmat(shmid,NULL,0))==NULL)
	{
		SysLog(1,"shmat sreg error\n");
		return ;
	}
	/** 信号量控制 **/
	for(i=0;i<MAXSERVREG;i++)
	{
		SysLog(1,"i[[[[]]]]]%d servpid [%d][%c]\n",i,(sreg+i)->servpid,(sreg+i)->stat[0]);
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
	int shmsize = MAXSERVREG*sizeof(_servreg);
	if((shmid = getshmid(7,shmsize))==-1)
	{
		SysLog(1,"get serv shm id error\n");
		return -1;
	}
	SysLog(1,"shmid is[%d]\n",shmid);
	if((sreg = shmat(shmid,NULL,0))==NULL)
	{
		SysLog(1,"shmat sreg error\n");
		return -1;
	}
	for(i=0;i<MAXSERVREG;i++)
	{
		//SysLog(1,"i[[[[]]]]]%d servpid [%d][%c]\n",i,(sreg+i)->servpid,(sreg+i)->stat[0]);
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
	SysLog(1,"解除信号量成功\n");
	return ret;
}


int insert_servreg(char *chnlname )
{
	int shmid = 0,i=0;
	_servreg *sreg = NULL;
	int shmsize = MAXSERVREG*sizeof(_servreg);
	if((shmid = getshmid(7,shmsize))==-1)
	{
		SysLog(1,"get serv shm id error\n");
		return -1;
	}
	SysLog(1,"shmid is[%d]\n",shmid);
	if((sreg = shmat(shmid,NULL,0))==NULL)
	{
		SysLog(1,"shmat sreg error\n");
		return -1;
	}
	for(i=0;i<MAXSERVREG;i++)
	{
		SysLog(1,"i[[[[]]]]]%d servpid [%d]\n",i,(sreg+i)->servpid);
		if((sreg+i)->servpid==0)
		{
			(sreg+i)->servpid = getpid();
			strcpy((sreg+i)->chnlname,chnlname);
			(sreg+i)->stat[0]='N';
			(sreg+i)->type[0]='S';
			shmdt(sreg);
			return 0;
		}else if((kill((sreg+i)->servpid,SIGUSR1)==-1)&&(errno == ESRCH))
		{
			(sreg+i)->servpid = getpid();
			strcpy((sreg+i)->chnlname,chnlname);
			(sreg+i)->stat[0]='N';
			shmdt(sreg);
			return 0;
		}
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
	atexit(memset_var_hash);
	atexit(exit_free);
	atexit(delservpid);
	
	mbuf = (_msgbuf *)malloc(sizeof(_msgbuf));
	if(mbuf == (void *)-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:申请msgbuf内存失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}

	tranbuf = (_tran *)malloc(sizeof(_tran));
	if(tranbuf == (void *)-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:申请tranbuf内存失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		free(mbuf);
		return -1;
	}

	iret = init_var_hash();
	if(iret != 0)
	{
		SysLog(1,"FILE [%s] LINE [%d]:初始化变量存放hash表失败\n",__FILE__,__LINE__);
		free(tranbuf);
		free(mbuf);
		return -1;
	}

	/** 渠道参数需要传入，服务根据渠道多少划分，防止一个服务挂掉所有都挂掉 **/
	if(getmsgid(argv[1],&msgidi,&msgido,&msgidr)!=0)
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取测试渠道消息队列失败\n",__FILE__,__LINE__);
		free(tranbuf);
		free(mbuf);
		return -1;
	}

	/** 注册serv **/
	if(insert_servreg(argv[1])==0)
	{
		SysLog(1,"FILE [%s] LINE [%d]:注册服务成功,PID[%ld]\n",__FILE__,__LINE__,getpid());
	}else
	{
		SysLog(1,"FILE [%s] LINE [%d]:注册服务失败,PID[%ld]\n",__FILE__,__LINE__,getpid());
		free(tranbuf);
		free(mbuf);
		return -1;
	}
	
	/** 堵塞到信号 **/
	signal(SIGUSR2,serv);
	signal(SIGALRM,servtimeout);
	signal(SIGPIPE,SIG_IGN);
	signal(SIGUSR1,SIG_IGN);
	while (1)
	{
		pause();
	}
	return 0;
}
void serv(int sig)
{
	SysLog(1,"FILE [%s] LINE [%d]:服务[%ld]获取到信号\n",__FILE__,__LINE__,getpid());
	if(init_malloced_hash()!=0)
	{
		SysLog(1,"FILE [%s] LINE [%d] 初始化变量存放区失败\n",__FILE__,__LINE__);
		return ;
	}
	SysLog(1,"FILE [%s] LINE [%d] 初始化变量存放区成功\n",__FILE__,__LINE__);

	seterr("AAAAAAAA","交易正常结束");

	iret = msgrcv(msgido,mbuf,sizeof(mbuf->tranbuf),0,IPC_NOWAIT);
	if(iret > 0)
	{
		innerid = mbuf->innerid ; 
		SysLog(1,"FILE [%s] LINE [%d] 处理来自[%20s]交易码为[%10s]长度为[%10ld]的交易\n",__FILE__,__LINE__,mbuf->tranbuf.chnlname,mbuf->tranbuf.trancode,mbuf->tranbuf.buffsize);
		SysLog(1,"FILE [%s] LINE[%d] 全局跟踪号为:[%ld]\n",__FILE__,__LINE__,innerid);
		if((get_shm_hash(mbuf->innerid,tranbuf))!=-1)
		{
			SysLog(1,"交易跟踪号[%ld]\t传入交易信息[%s]\n",mbuf->innerid,tranbuf->intran);
			if(unpack(mbuf->tranbuf.chnlname,tranbuf->intran,"|")==-1)
			{
				SysLog(1,"解[%s]包失败\t传入交易信息[%s]\n",mbuf->tranbuf.chnlname,tranbuf->intran);
				seterr("EEEEEEEE","解包失败");
			}else
			{
				if(serv_flow(mbuf->tranbuf.trancode)!=0)
				{
					SysLog(1,"处理[%s]交易流程失败\n",mbuf->tranbuf.trancode);
					seterr("EEEEEEEE","执行交易失败");
				}else
				{
					SysLog(1,"处理[%s]交易流程成功\n",mbuf->tranbuf.trancode);
				}
			}
		}else
		{
			SysLog(1,"FILE [%s] LINE[%d] 获取全局跟踪号:[%ld]信息失败\n",__FILE__,__LINE__,innerid);
			updatestat();
			return ;
		}
	}else if(errno  == ENOMSG)
	{
		SysLog(1,"获取不到交易信息\n");
		seterr("EEEEEEEE","获取不到交易信息");
	}else if(errno  !=ENOMSG )
	{
		SysLog(1,"其他错误\n");
		seterr("EEEEEEEE","其他错误");
	}else
	{
		SysLog(1,"其他错误\n");
		seterr("EEEEEEEE","其他错误");
	}
	/** 删除共享内存hash表中的交易信息 **/
    	if(delete_shm_hash(mbuf->innerid)==-1)
    	{
        	SysLog(1,"FILE [%s] LINE [%d]:删除共享内存hash表数据失败\n",__FILE__,__LINE__);
		updatestat();
		alarm(0);
		return ;
    	}
    	SysLog(1,"FILE [%s] LINE [%d]:删除共享内存hash表数据成功\n",__FILE__,__LINE__);
	/**修改状态为空闲 **/
	updatestat();
	alarm(0);
	return ;
}
int serv_flow(char *trancode)
{
	if(trancode==NULL)
	{
		SysLog(1,"FILE [%s] LINE[%d]获取交易码失败\n",__FILE__,__LINE__);
		seterr("EEEEEEEEEE","获取交易码失败");
		return -1;
	}
	_flow	localflow[1024];
	memset(localflow,0,sizeof(localflow));

	int i=1,j=1;
	/** 获取流程 **/
	if(get_flow(trancode,localflow)!=0)
	{
		SysLog(1,"获取交易码为[%s]的流程失败\n",trancode);
		return -1;
	}
	for(i=1;strcmp(localflow[i].flowname,"END");i++)
	{
		SysLog(1,"----------执行流程序号[%d]\t流程名称[%s]函数名称[%s]\n",i,localflow[i].flowname,localflow[i].flowfunc);
	}
	i=1;
	/** init commmsg **/
	while(strcmp(localflow[i].flowname,"END"))
	{
		SysLog(1,"开始处理流程flowname[%s]库[%s]函数[%s]参数[%s]\t\n",localflow[i].flowname,localflow[i].flowso,localflow[i].flowfunc,localflow[i].funcpar1);
		trim(localflow[i].flowso);
		trim(localflow[i].flowfunc);
		trim(localflow[i].funcpar1);
		if(do_so(localflow[i].flowso,localflow[i].flowfunc,localflow[i].funcpar1)==0)
		{
			SysLog(1,"流程处理成功\n");
		}else
		{
			SysLog(1,"!!!!!!!!!!!!!!!!!!!!!!!!流程处理失败!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			/** 执行错误流程**/
			if(get_flow(localflow[i].errflow,localflow)!=0)
			{
				SysLog(1,"获取交易码为[%s]的流程失败\n",trancode);
				return -1;
			}
			for(j=1;strcmp(localflow[j].flowname,"END");j++)
			{
				SysLog(1,"----------执行流程序号[%d]\t流程名称[%s]函数名称[%s]\n",j,localflow[j].flowname,localflow[j].flowfunc);
			}
			i=1;
			continue;
		}
		i++;
	}
	return 0;
}
