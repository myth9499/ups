#include "ups.h"

char chnl_name[20];
char	syncflag[2];
int iret = 0;
int msgidi=0,msgido=0;
_msgbuf *mbuf=NULL;
_tran *tranbuf=NULL;
long i=1L;

int getservpid(void)
{
	pid_t ret = 0;
	int shmid = 0,i=0;
	_servreg *sreg = NULL;
	int shmsize = MAXSERVREG*sizeof(_servreg);
	if((shmid = getshmid(7,shmsize))==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取服务登记表失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	if((sreg = shmat(shmid,NULL,0))==NULL)
	{
		SysLog(1,"FILE [%s] LINE [%d]:连接服务登记表失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	/** 信号量控制 **/
	for(i=0;i<MAXSERVREG;i++)
	{
		//printf("i[[[[]]]]]%d servpid [%d][%c]\n",i,(sreg+i)->servpid,(sreg+i)->stat[0]);
		if((sreg+i)->stat[0]=='N'&&!strcmp((sreg+i)->chnlname,chnl_name))
		{
			sem_wait(&((sreg+i)->sem1));
			(sreg+i)->stat[0]='L';
			ret = (sreg+i)->servpid ;
			sem_post(&((sreg+i)->sem1));
			break;
		}
	}
	shmdt(sreg);
	return ret;
}
/** 主进程注册信号，当子进程退出时进行后续处理
 * 防止僵尸进程
 **/
void child_exit(int signal)
{
	pid_t   pid;
	int     stat;
	while((pid = waitpid(-1,&stat,WNOHANG))>0)
	{
		SysLog(1,"FILE [%s] LINE [%d]:子进程[%d]退出，退出状态[%d]\n",__FILE__,__LINE__,pid,stat);
	}
	return ;
}
/** 注册进程timeout函数，暂定超时60秒 
 * 防止交易堵死 
 * */
void timeout(int signal)
{
	SysLog(1,"FILE [%s] LINE [%d]:交易超时结束\n",__FILE__,__LINE__);
	if(msgrcv(msgidi,mbuf,sizeof(mbuf->tranbuf),mbuf->innerid,0)==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:删除消息队列数据失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
	}
	return ;
}
/** 注册程序退出函数 
 * 释放申请的资源等信息 **/
void do_exit(void)
{
	free(tranbuf);
	free(mbuf);
}
int  chnlprocess(int clifd );
int main(int argc,char *argv[])
{

	/** 需要屏蔽所有信号 **/
	atexit(do_exit);
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	int sin_size = 0;
	pid_t pid;
	int sockfd,clifd;


	memset(chnl_name,0,sizeof(chnl_name));
	memset(syncflag,0,sizeof(syncflag));

	/** 修改为从命令行读取 **/
	strcpy(chnl_name,"测试渠道");
	syncflag[0]='S';

	mbuf = (_msgbuf *)malloc(sizeof(_msgbuf));
	if(mbuf == (void *)-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:MALLOC MSGBUF ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}

	tranbuf = (_tran *)malloc(sizeof(_tran));
	if(tranbuf == (void *)-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:MALLOC TRANBUF ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		free(mbuf);
		return -1;
	}

	if(getmsgid(chnl_name,&msgidi,&msgido)==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:GET CHANNEL[%s] MSGID ERROR[%s]\n",__FILE__,__LINE__,chnl_name,strerror(errno));
		free(tranbuf);
		free(mbuf);
		return -1;
	}

	/** 设置忽略SIGPIPE信号，防止因socket写的时候客户端关闭导致的SIGPIPE信号 **/
	signal(SIGPIPE,SIG_IGN);
	signal(SIGCHLD,child_exit);
	signal(35,child_exit);
#ifdef WIN32
	sigset_t signal_mask;
	sigemptyset (&signal_mask);
	sigaddset (&signal_mask, SIGPIPE);
	int rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
	if (rc != 0) {
		printf("block sigpipe error/n");
	} 
#endif  
	if((sockfd = socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:创建服务器套接字失败 ERROR[%s]\n",__FILE__,__LINE__,chnl_name,strerror(errno));
		return -1;
	}
	SysLog(1,"FILE [%s] LINE [%d]:创建服务器套接字成功\n",__FILE__,__LINE__);
	bzero(&serv_addr,sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(10000);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sockfd,(struct sockaddr *)(&serv_addr),sizeof(struct sockaddr))==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:绑定端口失败 ERROR[%s]\n",__FILE__,__LINE__,chnl_name,strerror(errno));
		return -1;
	}
	if(listen(sockfd,10000)==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:监听端口失败 ERROR[%s]\n",__FILE__,__LINE__,chnl_name,strerror(errno));
		return -1;
	}
	while(1)
	{
		sin_size = sizeof(struct sockaddr_in);
		if((clifd = accept(sockfd,(struct sockaddr *)(&cli_addr),&sin_size))<0)
		{
			/** 防止由于子进程产生信号导致主进程退出 **/
			if(errno == EINTR)
			{
				continue;
			}else
			{
				SysLog(1,"FILE [%s] LINE [%d]:接收accept失败 ERROR[%s]\n",__FILE__,__LINE__,chnl_name,strerror(errno));
				sleep (1);
			}
		}
		/** 创建子进程用来处理交易 **/
		pid = fork();
		if(pid == 0)
		{
			close(sockfd);
			if(chnlprocess(clifd)==0)
			{
				SysLog(1,"FILE [%s] LINE [%d]:渠道处理成功\n",__FILE__,__LINE__);
			}else
			{
				SysLog(1,"FILE [%s] LINE [%d]:渠道处理失败\n",__FILE__,__LINE__);
			}
			/** 防止SIGCHLD信号丢失**/
			kill(getppid(),35);
			exit(0);
		}
		close(clifd);
	}
	free(tranbuf);
	free(mbuf);
	return 0;
}
int chnlprocess(int clifd)
{
	int ipid = 0;
	char rcvbuf[4096];
	char tranid[5];
	char tranvalue[4096];
	char errmsg[1024];
	char rtmsg [1024];

	memset(errmsg,0,sizeof(errmsg));
	memset(rtmsg,0,sizeof(rtmsg));
	memset(rcvbuf,0,sizeof(rcvbuf));

	/** 注册超时信号 **/
	signal(SIGALRM,timeout);
	alarm(30);
	if(recv(clifd,rcvbuf,sizeof(rcvbuf),0)==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:读取socket报文失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		strcpy(errmsg,"读取报文失败");
		if(send(clifd,errmsg,strlen(errmsg),0)==-1)
		{
			SysLog(1,"FILE [%s] LINE [%d]:反馈渠道错误信息失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		}
		return  -1;
	}
	SysLog(1,"FILE [%s] LINE [%d]:获取到的报文信息为[%s]\n",__FILE__,__LINE__,rcvbuf);
	memset(mbuf,0,sizeof(mbuf));

	/**填充消息队列数据 **/
	/** 利用随机数产生唯一的交易跟踪号 **/
	srand((unsigned)time(NULL));
	mbuf->innerid =  (long)getpid()+rand()%1000000+rand()%3333333;
	//sprintf(mbuf->tranbuf,"%20s|%10s|%10d",chnl_name,"IXO101",strlen(rcvbuf));
	//
	strcpy(mbuf->tranbuf.chnlname,chnl_name);
	strcpy(mbuf->tranbuf.trancode,"IXO101");
	mbuf->tranbuf.buffsize = strlen(rcvbuf);

	SysLog(1,"FILE [%s] LINE [%d]:全系统跟踪号为[%ld]\n",__FILE__,__LINE__,mbuf->innerid);
	i++;
	if(shm_hash_insert(mbuf->innerid,rcvbuf,NULL)==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:放置交易报文信息到共享内存hash表中失败\n",__FILE__,__LINE__);
		if(send(clifd,"error",strlen("error"),0)==-1)
		{
			SysLog(1,"FILE [%s] LINE [%d]:反馈渠道错误信息失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		}
		return -1;
	}
	SysLog(1,"FILE [%s] LINE [%d]:放置交易报文信息到共享内存hash表中成功,跟踪号：[%ld],报文长度[%d]\n",__FILE__,__LINE__,mbuf->innerid,strlen(rcvbuf));
	iret = msgsnd(msgido,mbuf,sizeof(mbuf->tranbuf),IPC_NOWAIT);
	if(iret == -1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:发送控制信息到消息队列失败[%s]\n",__FILE__,__LINE__,strerror(errno));
		if(send(clifd,"error",strlen("error"),0)==-1)
		{
			SysLog(1,"FILE [%s] LINE [%d]:反馈渠道错误信息失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		}
		if(delete_shm_hash(mbuf->innerid)==-1)
		{
			SysLog(1,"FILE [%s] LINE [%d]:删除共享内存hash表数据失败\n",__FILE__,__LINE__);
		}
		return -1;
	}
	SysLog(1,"FILE [%s] LINE [%d]:获取可用服务并发送控制信号\n",__FILE__,__LINE__);
	/** 发送信号到核心服务 **/
	if((ipid = getservpid())<=0)
	{
		SysLog(1,"FILE [%s] LINE [%d]:暂无可用服务\n",__FILE__,__LINE__);
		/** 删除消息队列信息，防止堵塞 **/
		if(delete_shm_hash(mbuf->innerid)==-1)
		{
			SysLog(1,"FILE [%s] LINE [%d]:删除共享内存hash表数据失败\n",__FILE__,__LINE__);
		}
		if(msgrcv(msgidi,mbuf,sizeof(mbuf->tranbuf),mbuf->innerid,0)==-1)
		{
			SysLog(1,"FILE [%s] LINE [%d]:删除消息队列数据失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		}
		if(send(clifd,"error",strlen("error"),0)==-1)
		{
			SysLog(1,"FILE [%s] LINE [%d]:反馈渠道错误信息失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		}
		return -1;
	}
	SysLog(1,"FILE [%s] LINE [%d]:准备发送到的服务进程为 [%ld]\n",__FILE__,__LINE__,ipid);
	if(kill(ipid,SIGUSR2)==0)
	{
		SysLog(1,"FILE [%s] LINE [%d]:发送控制信号到服务 [%ld]成功\n",__FILE__,__LINE__,ipid);
	}else
	{
		SysLog(1,"FILE [%s] LINE [%d]:发送控制信号到服务 [%ld]失败：[%s]\n",__FILE__,__LINE__,ipid,strerror(errno));
		if(send(clifd,"error",strlen("error"),0)==-1)
		{
			SysLog(1,"FILE [%s] LINE [%d]:反馈渠道错误信息失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		}
		/**还需要删除消息队列信息，防止堵塞 **/
		if(delete_shm_hash(mbuf->innerid)==-1)
		{
			SysLog(1,"FILE [%s] LINE [%d]:删除共享内存hash表数据失败\n",__FILE__,__LINE__);
		}
		if(msgrcv(msgidi,mbuf,sizeof(mbuf->tranbuf),mbuf->innerid,0)==-1)
		{
			SysLog(1,"FILE [%s] LINE [%d]:删除消息队列数据失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		}
		return -1;
	}
	iret = msgrcv(msgidi,mbuf,sizeof(mbuf->tranbuf),mbuf->innerid,0);
	if(iret < 0)
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取服务返回失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		if(send(clifd,"交易处理失败",strlen("交易处理失败"),0)==-1)
		{
			SysLog(1,"FILE [%s] LINE [%d]:反馈渠道错误信息失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		}
		/**还需要删除消息队列信息，防止堵塞 **/
		if(delete_shm_hash(mbuf->innerid)==-1)
		{
			SysLog(1,"FILE [%s] LINE [%d]:删除共享内存hash表数据失败\n",__FILE__,__LINE__);
		}
		close(clifd);
		return -1;
	}else
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取到服务返回信息\n",__FILE__,__LINE__);
		//printf("渠道名称[%20s]交易码[%10s]返回数据长度[%10ld]\n",mbuf->tranbuf.chnlname,mbuf->tranbuf.trancode,mbuf->tranbuf.buffsize);
		if(get_shm_hash(mbuf->innerid,tranbuf)!=-1)
		{
			SysLog(1,"FILE [%s] LINE [%d]:获取到服务返回信息，跟踪号[%ld]返回信息[%s]\n",__FILE__,__LINE__,mbuf->innerid,tranbuf->outtran);
			if(write(clifd,tranbuf->outtran,strlen(tranbuf->outtran))==-1)
			{
				SysLog(1,"FILE [%s] LINE [%d]:反馈渠道错误信息失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
			}
			memset(tranbuf,0,sizeof(_tran));
			if(delete_shm_hash(mbuf->innerid)==-1)
			{
				SysLog(1,"FILE [%s] LINE [%d]:删除共享内存hash表数据失败\n",__FILE__,__LINE__);
			}
			close(clifd);
			return  0;
		}else
		{
			if(send(clifd,"error",strlen("error"),0)==-1)
			{
				SysLog(1,"FILE [%s] LINE [%d]:反馈渠道错误信息失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
			}
			if(delete_shm_hash(mbuf->innerid)==-1)
			{
				SysLog(1,"FILE [%s] LINE [%d]:删除共享内存hash表数据失败\n",__FILE__,__LINE__);
			}
			close(clifd);
			return  -1;
		}
	}
}
