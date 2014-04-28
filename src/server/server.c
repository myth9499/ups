#include "ups.h"

void destory_var_hash(void);
void serv(int sig);

/** 超时函数 **/
void servtimeout(int signal)
{
	seterr("EEEEEEEE","交易超时结束");
}
void delservpid(void)
{
	printf("开始删除共享内存数据[%ld]\n",getpid());
	pid_t ret = 0;
	int shmid = 0,i=0,semid = 0;
	_servreg *sreg = NULL;
	int shmsize = MAXSERVREG*sizeof(_servreg);
	if((shmid = getshmid(7,shmsize))==-1)
	{
		printf("get serv shm id error\n");
		return ;
	}
	printf("shmid is[%d]\n",shmid);
	if((sreg = shmat(shmid,NULL,0))==NULL)
	{
		printf("shmat sreg error\n");
		return ;
	}
	/** 信号量控制 **/
	for(i=0;i<MAXSERVREG;i++)
	{
		printf("i[[[[]]]]]%d servpid [%d][%c]\n",i,(sreg+i)->servpid,(sreg+i)->stat[0]);
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
		printf("get serv shm id error\n");
		return -1;
	}
	printf("shmid is[%d]\n",shmid);
	if((sreg = shmat(shmid,NULL,0))==NULL)
	{
		printf("shmat sreg error\n");
		return -1;
	}
	for(i=0;i<MAXSERVREG;i++)
	{
		printf("i[[[[]]]]]%d servpid [%d][%c]\n",i,(sreg+i)->servpid,(sreg+i)->stat[0]);
		if((sreg+i)->servpid==getpid())
		{
			sem_wait(&((sreg+i)->sem1));
			(sreg+i)->stat[0]='N';
			ret = 0;
			sem_post(&((sreg+i)->sem1));
			break;
		}

	}
	shmdt(sreg);
	printf("解除信号量成功\n");
	return ret;
}


int insert_servreg(char *chnlname )
{
	int shmid = 0,i=0;
	_servreg *sreg = NULL;
	int shmsize = MAXSERVREG*sizeof(_servreg);
	if((shmid = getshmid(7,shmsize))==-1)
	{
		printf("get serv shm id error\n");
		return -1;
	}
	printf("shmid is[%d]\n",shmid);
	if((sreg = shmat(shmid,NULL,0))==NULL)
	{
		printf("shmat sreg error\n");
		return -1;
	}
	for(i=0;i<MAXSERVREG;i++)
	{
		printf("i[[[[]]]]]%d servpid [%d]\n",i,(sreg+i)->servpid);
		if((sreg+i)->servpid==0)
		{
			(sreg+i)->servpid = getpid();
			strcpy((sreg+i)->chnlname,chnlname);
			(sreg+i)->stat[0]='N';
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


int iret = 0;
int msgidi=0,msgido=0;
key_t key;
_msgbuf *mbuf=NULL;
_tran *tranbuf = NULL;


int main(int argc,char *argv[])
{
	atexit(destory_var_hash);
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
	if(getmsgid("测试渠道",&msgidi,&msgido)!=0)
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取测试渠道消息队列失败\n",__FILE__,__LINE__);
		free(tranbuf);
		free(mbuf);
		return -1;
	}

	/** 注册serv **/
	if(insert_servreg("测试渠道")==0)
	{
		SysLog(1,"FILE [%s] LINE [%d]:注册服务成功,PID[%ld]\n",__FILE__,__LINE__,getpid());
	}else
	{
		SysLog(1,"FILE [%s] LINE [%d]:注册服务失败,PID[%ld]\n",__FILE__,__LINE__,getpid());
		free(tranbuf);
		free(mbuf);
		return -1;
	}
	seterr("AAAAAAAA","交易正常结束");
	
	/** 堵塞到信号 **/
	signal(SIGUSR2,serv);
	signal(SIGALRM,servtimeout);
	signal(SIGUSR1,SIG_IGN);
	while (1)
	{
		pause();
	}
	free(tranbuf);
	free(mbuf);
	return 0;
}
void serv(int sig)
{
	SysLog(1,"FILE [%s] LINE [%d]:服务[%ld]获取到信号\n",__FILE__,__LINE__,getpid());

	iret = msgrcv(msgido,mbuf,sizeof(mbuf->tranbuf),0,IPC_NOWAIT);
	if(iret > 0)
	{
		innerid = mbuf->innerid ; 
		SysLog(1,"FILE [%s] LINE [%d] 处理来自[%20s]交易码为[%10s]长度为[%10ld]的交易\n",__FILE__,__LINE__,mbuf->tranbuf.chnlname,mbuf->tranbuf.trancode,mbuf->tranbuf.buffsize);
		SysLog(1,"FILE [%s] LINE[%d] 全局跟踪号为:[%ld]\n",__FILE__,__LINE__,innerid);
		if((get_shm_hash(mbuf->innerid,tranbuf))!=-1)
		{
			SysLog(1,"交易跟踪号[%ld]\t传入交易信息[%s]\n",mbuf->innerid,tranbuf->intran);
			if(unpack(mbuf->tranbuf.chnlname,tranbuf->intran)==-1)
			{
				SysLog(1,"解[%s]包失败\t传入交易信息[%s]\n",mbuf->tranbuf.chnlname,tranbuf->intran);
				seterr("EEEEEEEE","解包失败");
			}
			if(serv_flow(mbuf->tranbuf.trancode)!=0)
			{
				SysLog(1,"处理[%s]交易流程失败\n",mbuf->tranbuf.trancode);
				seterr("EEEEEEEE","执行交易失败");
			}else
			{
				SysLog(1,"处理[%s]交易流程成功\n",mbuf->tranbuf.trancode);
			}
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
	/** 删除共享内存hash表中的交易信息 
    if(delete_shm_hash(mbuf->innerid)==-1)
    {
        SysLog(1,"FILE [%s] LINE [%d]:删除共享内存hash表数据失败\n",__FILE__,__LINE__);
    }
**/
	/**修改状态为空闲 **/
	updatestat();
	alarm(0);
	return ;
}
int testvar(void)
{
	char value[1024];
	memset(value,0,sizeof(value));
	get_var_value("V_CHANNEL",1024,1,value);
	printf("var value is [%s]\n",value);
	get_var_value("V_TRAN",1024,1,value);
	printf("var value is [%s]\n",value);
	get_var_value("V_BUFF",1024,1,value);
	printf("var value is [%s]\n",value);
	return 0;
}
int serv_flow(char *trancode)
{
	_tranmap tmap;
	/** 根据交易码，获取到流程名称 **/
	printf("1111\n");
	trim(trancode);
	if(gettranmap(&tmap,trancode)==-1)
	{
		SysLog(1,"FILE [%s] LINE[%d] 根据交易码获取交易流程失败，请查看是否配置\n",__FILE__,__LINE__);
		seterr("EEEEEEEEEE","根据交易码获取交易流程失败，请查看是否配置");
		return -1;
	}
	SysLog(1,"交易码[%s]交易名称[%s]交易流程名称[%s]超时时间[%d]\n",tmap.trancode,tmap.tranname,tmap.tranflow,tmap.timeout);
	alarm(tmap.timeout);
	key_t	key;
	int shmid,i=0;
	_flow *flow=NULL;
	char *tmpbuf = NULL;
	_flow *tmpshmdt=NULL;
	size_t shmsize;
	/** init commmsg **/
	shmsize=MAXFLOW*sizeof(_flow);
	if((shmid=getshmid(8,shmsize))==-1)
	{
		SysLog(1,"FILE [%s] LINE[%d] 获取共享内存号失败\n",__FILE__,__LINE__);
		seterr("EEEEEEEEEE","获取共享内存失败");
		return -1;
	}
	if(trancode==NULL)
	{
		SysLog(1,"FILE [%s] LINE[%d]获取交易码失败\n",__FILE__,__LINE__);
		seterr("EEEEEEEEEE","获取交易码失败");
		return -1;
	}
	SysLog(1,"开始执行流程[%s] \n",tmap.tranname);
	flow = (_flow *)shmat(shmid,NULL,0);
	if(flow == NULL)
	{
		SysLog(1,"FILE [%s] LINE[%d]链接共享内存失败\n",__FILE__,__LINE__);
		seterr("EEEEEEEEEE","链接共享内存失败");
		return -1;
	}
	tmpshmdt = flow;
	while(strcmp(flow->flowname,"END"))
	{
		if(!strcmp(flow->flowname,tmap.tranflow)&&(strcmp(flow->flowso,"")))
		{
			SysLog(1,"开始处理流程flowname[%s]库[%s]函数[%s]参数[%s]\t",flow->flowname,
					flow->flowso,flow->flowfunc,flow->funcpar1);
			trim(flow->flowso);
			trim(flow->flowfunc);
			trim(flow->funcpar1);
			if(do_so(flow->flowso,flow->flowfunc,flow->funcpar1)==0)
			{
				SysLog(1,"流程处理成功\n");
			}else
			{
				SysLog(1,"流程处理失败\n");
				/** 执行错误流程**/
			}
			flow++;
			continue;
		}
		flow++;
	}
	shmdt(tmpshmdt);
	return 0;
}
