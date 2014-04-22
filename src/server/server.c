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
		perror("malloc msgbuf error");
		return -1;
	}

	tranbuf = (_tran *)malloc(sizeof(_tran));
	if(tranbuf == (void *)-1)
	{
		perror("malloc msgbuf error");
		free(mbuf);
		return -1;
	}

	iret = init_var_hash();
	if(iret != 0)
	{
		printf("init var hash error\n");
		free(tranbuf);
		free(mbuf);
		return -1;
	}

	/** 渠道参数需要传入，服务根据渠道多少划分，防止一个服务挂掉所有都挂掉 **/
	if(getmsgid("测试渠道",&msgidi,&msgido)!=0)
	{
		printf("get msgid error\n");
		free(tranbuf);
		free(mbuf);
		return -1;
	}

	/** 注册serv **/
	if(insert_servreg("测试渠道")==0)
	{
		printf("注册服务成功\n");
	}else
	{
		printf("注册服务失败\n");
		free(tranbuf);
		free(mbuf);
		return -1;
	}
	
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
	printf("获取信号!!!\n");
	char errmsg[80];
	char errcode[20];
	memset(errmsg,0,sizeof(errmsg));
	memset(errcode,0,sizeof(errcode));

	strcpy(errcode,"AAAAAAAA");
	strcpy(errmsg,"ok");

	printf("mbuf size[%d]\n",sizeof(mbuf->tranbuf));
	iret = msgrcv(msgido,mbuf,sizeof(mbuf->tranbuf),0,IPC_NOWAIT);
	if(iret > 0)
	{
		/** 先返回 0000表示成功 暂时不要尝试一下  
		  strncpy(mbuf->tranbuf,"0000",strlen("0000"));
		  strncpy(mbuf->tranbuf+4,"ok",strlen("ok"));
		  iret  = msgsnd(msgidi,mbuf,sizeof(mbuf->tranbuf),IPC_NOWAIT);
		  if(iret <0 )
		  {
		  printf("返回原发起渠道错\n");
		  continue;
		  }
		 **/
		innerid = mbuf->innerid ; 
		printf("处理来自[%20s]交易码为[%10s]长度为[%10ld]的交易\n",mbuf->tranbuf.chnlname,mbuf->tranbuf.trancode,mbuf->tranbuf.buffsize);
		printf("全局跟踪号为:[%ld]\n",innerid);
		if((get_shm_hash(mbuf->innerid,tranbuf))!=-1)
		{
			printf("innerid[%ld]\tinbuf[%s]\n",mbuf->innerid,tranbuf->intran);
			/*
			strcpy(tranbuf->outtran,tranbuf->intran);
			iret = shm_hash_update(mbuf->innerid,NULL,tranbuf->outtran);
			if(iret == -1)
			{
				memset(errmsg,0,sizeof(errmsg));
				memset(errcode,0,sizeof(errcode));
				strcpy(errcode,"EEEEEEEE");
				strcpy(errmsg,"shm hash update error");
			}
			*/
			if(unpack(mbuf->tranbuf.chnlname,tranbuf->intran)==-1)
			{
				memset(errmsg,0,sizeof(errmsg));
				memset(errcode,0,sizeof(errcode));
				strcpy(errcode,"EEEEEEEE");
				strcpy(errmsg,"unpack tran buf  error");
			}
			testvar();
			if(serv_flow(mbuf->tranbuf.trancode)!=0)
			{
				memset(errmsg,0,sizeof(errmsg));
				memset(errcode,0,sizeof(errcode));
				strcpy(errcode,"EEEEEEEE");
				strcpy(errmsg,"do serv flow  error");
			}else
			{
				memset(errmsg,0,sizeof(errmsg));
				memset(errcode,0,sizeof(errcode));
				strcpy(errcode,"EEEEEEEE");
				strcpy(errmsg,"do serv flow  ok");
			}
		}
	}else if(errno  == ENOMSG)
	{
		memset(errmsg,0,sizeof(errmsg));
		memset(errcode,0,sizeof(errcode));
		strcpy(errcode,"EEEEEEEE");
		strcpy(errmsg,"no msg ");
	}else if(errno  !=ENOMSG )
	{
		perror("123");
		memset(errmsg,0,sizeof(errmsg));
		memset(errcode,0,sizeof(errcode));
		strcpy(errcode,"EEEEEEEE");
		strcpy(errmsg,"msg  recv error");
	}else
	{
		memset(errmsg,0,sizeof(errmsg));
		memset(errcode,0,sizeof(errcode));
		strcpy(errcode,"EEEEEEEE");
		strcpy(errmsg,"msg  recv other error");
	}
	printf("errmsg:[%s]\n",errmsg);
	strcpy(mbuf->tranbuf.chnlname,"核心服务一");
	strcpy(mbuf->tranbuf.trancode,errmsg);
	mbuf->tranbuf.buffsize = strlen(errmsg);

	/** 返回处理结果到前端 
	iret = msgsnd(msgidi,mbuf,sizeof(mbuf->tranbuf),IPC_NOWAIT);
	if(iret == -1)
	{
		strcpy(errmsg,"respos msg error");
	}
	strcpy(errmsg,"respos msg ok");
	printf("errmsg:[%s]\n",errmsg);
	**/
	/**修改状态为空闲 **/
	updatestat();
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
		printf("根据交易码获取交易流程失败，请查看是否配置\n");
		seterr("EEEEEEEEEE","根据交易码获取交易流程失败，请查看是否配置");
		return -1;
	}
	printf("trancode[%s]tranname[%s]tranflow[%s]timeout[%d]\n",tmap.trancode,tmap.tranname,tmap.tranflow,tmap.timeout);
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
		printf("get shm error\n");
		return -1;
	}
	if(trancode==NULL)
	{
		printf("the flow error\n");
		return -1;
	}
	printf("start serv flow trancode[%s] \n",trancode);
	flow = (_flow *)shmat(shmid,NULL,0);
	if(flow == NULL)
	{
		printf("flow shmat error\n");
		return -1;
	}
	tmpshmdt = flow;
	while(strcmp(flow->flowname,"END"))
	{
		printf("flow flowname[%s]\n",flow->flowname);
		if(!strcmp(flow->flowname,tmap.tranflow)&&(strcmp(flow->flowso,"")))
		{
			printf("开始处理流程flowname[%s]库[%s]函数[%s]参数[%s]\t",flow->flowname,
					flow->flowso,flow->flowfunc,flow->funcpar1);
			if(do_so(flow->flowso,flow->flowfunc,flow->funcpar1,NULL,NULL)==0)
			{
				printf("流程处理成功\n");
			}else
			{
				printf("流程处理失败\n");
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
