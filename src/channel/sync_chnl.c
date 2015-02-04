#include "ups.h"

char	chnl_name[20];
char	syncflag[2];
int		iret = 0;
int		msgidi=0,msgido=0,msgidr=0;
int		chnlprocess(int clifd,_msgbuf *mbuf,_tran *tranbuf );

/** 主进程注册信号，当子进程退出时进行后续处理
 * 防止僵尸进程
 * 李磊
 **/
void child_exit(int signal)
{
	pid_t   pid;
	int     stat;
	while((pid = waitpid(-1,&stat,WNOHANG|WUNTRACED))>0)
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:子进程[%d]退出，退出状态[%d]\n",__FILE__,__LINE__,pid,stat);
	}
	if(pid == -1)
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:ERROR[%s]!!!\n",__FILE__,__LINE__,strerror(errno));
	}
	return ;
}
/** 注册进程timeout函数，暂定超时60秒 
 * 防止交易堵死 
 * 李磊
 * */
void timeout(int signal)
{
	SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:交易超时结束,删除消息队列和hash表数据\n",__FILE__,__LINE__);
	return ;
}
/** 注册程序退出函数 
 * 释放申请的资源等信息 
 * 李磊
 **/
void do_exit(void)
{
	SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:程序退出\n",__FILE__,__LINE__);
}

/**
 * socket 监听渠道主程序
 * 李磊
 **/
int main(int argc,char *argv[])
{

	if(argc<3)
	{
		printf("启动渠道参数错误:usage appname+chnlname+listenport\n");
		return -1;
	}
	/** 初始化全局共享内存前，先获取ups根路径 **/
	if(setupshome()==-1)
	{
		printf("设置全局变量upshome错误,请检查UPSHOME环境变量是否设置\n");
		return -1;
	}

	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	int sin_size = 0;
	pid_t pid;
	int sockfd,clifd;
	char	startcmd[200];
	int	i=0;

	memset(startcmd,0x00,sizeof(startcmd));
	memset(chnl_name,0,sizeof(chnl_name));
	memset(syncflag,0,sizeof(syncflag));

	atexit(do_exit);
	/** 修改为从命令行读取 **/
	strcpy(chnl_name,argv[1]);
	syncflag[0]='S';

	if(getmsgid(chnl_name,&msgidi,&msgido,&msgidr)==-1)
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:获取渠道[%s] 消息队列失败[%s]\n",__FILE__,__LINE__,chnl_name,strerror(errno));
		return -1;
	}

	/** 设置忽略SIGPIPE信号，防止因socket写的时候客户端关闭导致的SIGPIPE信号 **/
	signal(SIGPIPE,SIG_IGN);
	//signal(SIGCHLD,child_exit);
	signal(SIGCHLD,SIG_IGN);
	//signal(35,child_exit);
	signal(SIGUSR1,SIG_IGN);
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
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:创建服务器套接字失败 ERROR[%s]\n",__FILE__,__LINE__,chnl_name,strerror(errno));
		return -1;
	}
	SysLog(LOG_CHNL_SHOW,"FILE [%s] LINE [%d]:创建服务器套接字成功\n",__FILE__,__LINE__);
	bzero(&serv_addr,sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sockfd,(struct sockaddr *)(&serv_addr),sizeof(struct sockaddr))==-1)
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:渠道[%s] 绑定端口失败 ERROR[%s]\n",__FILE__,__LINE__,chnl_name,strerror(errno));
		return -1;
	}
	if(listen(sockfd,10000)==-1)
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:渠道[%s] 监听端口失败 ERROR[%s]\n",__FILE__,__LINE__,chnl_name,strerror(errno));
		return -1;
	}
	for(i=0;i<argc;i++)
	{
		if(strlen(argv[i])==0)
			break;
		strcat(startcmd," ");
		strcat(startcmd,argv[i]);
	}
	strcat(startcmd," ");
	strcat(startcmd,"&");

	/** 添加当前渠道到服务登记区，用来监控 **/
	if(insert_chnlreg(startcmd,chnl_name)!=0)
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:添加渠道到监控内存失败\n",__FILE__,__LINE__);
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
				SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:接收accept失败 ERROR[%s]\n",__FILE__,__LINE__,chnl_name,strerror(errno));
				sleep (1);
			}
		}
		/** 创建子进程用来处理交易 **/
		pid = fork();
		if(pid == 0)
		{
			close(sockfd);
			_msgbuf *mbuf = NULL;
			_tran *tranbuf = NULL;
			int iiret = 0;

			mbuf = (_msgbuf *)malloc(sizeof(_msgbuf));
			if(mbuf == (void *)-1)
			{
				SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:获取消息队列内容区内存失败[%s]\n",__FILE__,__LINE__,strerror(errno));
				return -2;
			}

			tranbuf = (_tran *)malloc(sizeof(_tran));
			if(tranbuf == (void *)-1)
			{
				SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:获取交易临时存放区失败[%s]\n",__FILE__,__LINE__,strerror(errno));
				free(mbuf);
				return -2;
			}

			iret = chnlprocess(clifd,mbuf,tranbuf);
			if(iret == 0)
			{
				if(get_shm_hash(mbuf->innerid,tranbuf)==-1)
				{
					SysLog(LOG_CHNL_SHOW,"FILE [%s] LINE[%d] 处理成功，但核心已超时，不发送消息队列通知\n",__FILE__,__LINE__);
					exit(-1);
				}else
				{
					/** 返回交易信息到服务端**/
					iret = shm_hash_update(mbuf->innerid,"AAAAAAA|渠道处理成功",NULL);
					if(iret == -1)
					{
						SysLog(LOG_CHNL_ERR,"放置打包信息到共享内存失败 \n");
					}else
					{
						msgsnd(msgidr,mbuf,sizeof(mbuf->tranbuf),IPC_NOWAIT);
					}
					/** 检测一下再关闭，适合在机器性能较差的机器上做检测
					  防止出现CLOSE_WAIT较多导致服务连接不上的情况，高性能机器直接关闭 **/
					/** for 低性能机器代码 
					  SysLog(LOG_CHNL_ERR,"开始检测\n");
					  iiret =recv(clifd,rcvbuf,sizeof(rcvbuf),0);
					//iiret =recv(clifd,rcvbuf,sizeof(rcvbuf),MSG_DONTWAIT);
					if(iiret ==-1||iiret ==0)
					{
					close(clifd);
					}
					//	shutdown(clifd,SHUT_RDWR);
					 **/
					/**for 高性能机器代码，不检查，直接关闭 **/
					SysLog(LOG_CHNL_SHOW,"FILE [%s] LINE [%d]:渠道处理成功\n",__FILE__,__LINE__);
					close(clifd);
					free(mbuf);
					free(tranbuf);
				}
			}else if(iret == -1)
			{
				if(get_shm_hash(mbuf->innerid,tranbuf)==-1)
				{
					SysLog(LOG_CHNL_SHOW,"FILE [%s] LINE[%d] 处理失败，但核心已超时，不发送消息队列通知\n",__FILE__,__LINE__);
					exit(-1);
				}else
				{
					/** 返回交易信息到服务端**/
					iret = shm_hash_update(mbuf->innerid,"EEEEEEE|渠道处理失败",NULL);
					if(iret == -1)
					{
						SysLog(LOG_CHNL_ERR,"放置打包信息到共享内存失败 \n");
					}else
					{
						msgsnd(msgidr,mbuf,sizeof(mbuf->tranbuf),IPC_NOWAIT);
					}
					/** 检测一下再关闭，适合在机器性能较差的机器上做检测
					  防止出现CLOSE_WAIT较多导致服务连接不上的情况，高性能机器直接关闭 **/
					/** for 低性能机器代码 
					  SysLog(LOG_CHNL_ERR,"开始检测\n");
					  iiret =recv(clifd,rcvbuf,sizeof(rcvbuf),0);
					//iiret =recv(clifd,rcvbuf,sizeof(rcvbuf),MSG_DONTWAIT);
					if(iiret ==-1||iiret ==0)
					{
					close(clifd);
					}
					//	shutdown(clifd,SHUT_RDWR);
					 **/
					/**for 高性能机器代码，不检查，直接关闭 **/
					SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:渠道处理失败\n",__FILE__,__LINE__);
					close(clifd);
					free(mbuf);
					free(tranbuf);
				}
			}else 
			{
				/** 检测一下再关闭，适合在机器性能较差的机器上做检测
				  防止出现CLOSE_WAIT较多导致服务连接不上的情况，高性能机器直接关闭 **/
				/** for 低性能机器代码 
				  SysLog(LOG_CHNL_ERR,"开始检测\n");
				  iiret =recv(clifd,rcvbuf,sizeof(rcvbuf),0);
				//iiret =recv(clifd,rcvbuf,sizeof(rcvbuf),MSG_DONTWAIT);
				if(iiret ==-1||iiret ==0)
				{
				close(clifd);
				}
				//	shutdown(clifd,SHUT_RDWR);
				 **/
				/**for 高性能机器代码，不检查，直接关闭 **/
				SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:渠道处理失败\n",__FILE__,__LINE__);
				close(clifd);
				free(mbuf);
				free(tranbuf);
			}
			/** 防止SIGCHLD信号丢失**/
			//kill(getppid(),35);
			exit(0);
		}else if(pid<0)
		{
			SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:创建子进程失败[%s]\n",__FILE__,__LINE__,strerror(errno));
		}
		//alarm(0);
		close(clifd);
	}
	return 0;
}

/** socket渠道接收到消息后处理函数
 * 李磊
 * 传入：socketfd 消息队列临时存放区 交易信息临时存放区
 **/
int chnlprocess(int clifd,_msgbuf *mbuf,_tran *tranbuf)
{

	int ipid = 0;
	char rcvbuf[41];
	char	*tranbuffer=NULL;
	char	*intran=NULL;//需要放到共享内存区数据
	char	servhead[100];//需要构建与serv通信解析的头,根据实际生产系统定制
	char errmsg[1024];

	memset(errmsg,0,sizeof(errmsg));
	memset(rcvbuf,0,sizeof(rcvbuf));

	/** 注册超时信号 **/
	signal(SIGALRM,timeout);
	signal(SIGPIPE,SIG_IGN);
	signal(SIGUSR1,SIG_IGN);

	/** 设置socket参数 **/
	int	keepAlive = 1;
	if(setsockopt(clifd, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(keepAlive))!=0)
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:设置socket参数失败ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	/**  设置recv超时时间 **/
	struct timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	if(setsockopt(clifd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval))!=0)
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:设置socket参数失败ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		return -2;
	}

	if(recv(clifd,rcvbuf,sizeof(rcvbuf)-1,0)==-1)
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:读取socket报文失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		strcpy(errmsg,"读取报文失败");
		if(send(clifd,errmsg,strlen(errmsg),0)==-1)
		{
			SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:反馈渠道错误信息失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		}
		return  -2;
	}
	SysLog(LOG_CHNL_DEBUG,"FILE [%s] LINE [%d]:获取到的交易头信息为[%s]\n",__FILE__,__LINE__,rcvbuf);
	memset(mbuf,0,sizeof(mbuf));

	/**填充消息队列数据 **/
	/** 利用随机数产生唯一的交易跟踪号 后期考虑改动**/
	srand((unsigned)time(NULL));
	mbuf->innerid =  (long)getpid()+rand()%1000000+rand()%3333333;


	/** 
	 * 20位渠道名称+10位交易码+10位交易长度
	 * 利用SOCKET预读取先确定待读取内容长度
	 * **/
	memset(mbuf->tranbuf.chnlname,0,sizeof(mbuf->tranbuf.chnlname));
	memset(mbuf->tranbuf.trancode,0,sizeof(mbuf->tranbuf.trancode));
	memcpy(mbuf->tranbuf.chnlname,rcvbuf,20);
	mbuf->tranbuf.chnlname[strlen(mbuf->tranbuf.chnlname)-1]='\0';
	memcpy(mbuf->tranbuf.trancode,rcvbuf+20,10);
	mbuf->tranbuf.trancode[strlen(mbuf->tranbuf.trancode)-1]='\0';
	mbuf->tranbuf.buffsize = atoi(rcvbuf+30);

	/** 再次获取剩余的交易信息，根据传入的长度进行获取　**/
	tranbuffer = (char *)malloc(sizeof(char)*mbuf->tranbuf.buffsize);
	if(tranbuffer == NULL)
	{
		SysLog(LOG_CHNL_SHOW,"FILE [%s] LINE [%d]:获取临时交易信息存放区失败[%s]\n",__FILE__,__LINE__,strerror(errno));
		return -2;
	}
	SysLog(LOG_CHNL_SHOW,"FILE [%s] LINE [%d]:全系统跟踪号为[%ld],处理来自[%s]渠道,交易码[%s],长度[%d]的交易\n",__FILE__,__LINE__,mbuf->innerid,mbuf->tranbuf.chnlname,mbuf->tranbuf.trancode,mbuf->tranbuf.buffsize);
	if(recv(clifd,tranbuffer,mbuf->tranbuf.buffsize,0)==-1)
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:读取socket报文失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		strcpy(errmsg,"读取报文失败");
		if(send(clifd,errmsg,strlen(errmsg),0)==-1)
		{
			SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:反馈渠道错误信息失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		}
		free(tranbuffer);
		return  -2;
	}
	SysLog(LOG_CHNL_DEBUG,"FILE [%s] LINE [%d]:获取到的交易信息为[%s]\n",__FILE__,__LINE__,tranbuffer);
	/** 发送信号到核心服务 **/
	SysLog(LOG_CHNL_DEBUG,"FILE [%s] LINE [%d]:获取可用服务并发送控制信号\n",__FILE__,__LINE__);
	if((ipid = getservpid(chnl_name))<=0)
	{
		/** 同时要变更查找到的服务为可用 **/
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:暂无可用服务\n",__FILE__,__LINE__);
		updatestat_foroth(ipid);
		if(send(clifd,"系统繁忙",strlen("系统繁忙"),0)==-1)
		{
			SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:反馈渠道错误信息失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		}
		free(tranbuffer);
		return -2;
	}
	SysLog(LOG_CHNL_DEBUG,"FILE [%s] LINE [%d]:获取可用服务[%ld]\n",__FILE__,__LINE__,ipid);
	/** 定制与serv通信的定义头 **/
	memset(servhead,0x00,sizeof(servhead));
	sprintf(servhead,"%s|%s|%s",mbuf->tranbuf.chnlname,mbuf->tranbuf.trancode,"ibps.101.001.01");
	intran = (char *)malloc(sizeof(char)*mbuf->tranbuf.buffsize+sizeof(servhead));
	if(intran ==NULL)
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:获取内存失败[%s]\n",__FILE__,__LINE__,strerror(errno));
		free(tranbuffer);
		updatestat_foroth(ipid);
		return -2;
	}
	sprintf(intran,"%s|%s",servhead,tranbuffer);
	if(shm_hash_insert(mbuf->innerid,intran,NULL)==-1)
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:放置交易报文信息到共享内存hash表中失败\n",__FILE__,__LINE__);
		if(send(clifd,"系统异常",strlen("系统异常"),0)==-1)
		{
			SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:反馈渠道错误信息失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		}
		updatestat_foroth(ipid);
		free(tranbuffer);
		free(intran);
		return -1;
	}
	SysLog(LOG_CHNL_SHOW,"FILE [%s] LINE [%d]:放置交易报文信息到共享内存hash表中成功,跟踪号：[%ld],报文长度[%d]\n",__FILE__,__LINE__,mbuf->innerid,strlen(intran));
	free(intran);
	iret = msgsnd(msgido,mbuf,sizeof(mbuf->tranbuf),IPC_NOWAIT);
	if(iret == -1)
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:发送控制信息到消息队列失败[%s]\n",__FILE__,__LINE__,strerror(errno));
		if(send(clifd,"error",strlen("error"),0)==-1)
		{
			SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:反馈渠道错误信息失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		}
		if(delete_shm_hash(mbuf->innerid)==-1)
		{
			SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:删除共享内存hash表数据失败\n",__FILE__,__LINE__);
		}
		updatestat_foroth(ipid);
		free(tranbuffer);
		return -1;
	}
	SysLog(LOG_CHNL_SHOW,"FILE [%s] LINE [%d]:准备发送到的服务进程为 [%ld]\n",__FILE__,__LINE__,ipid);
	if(kill(ipid,SIGUSR2)==0)
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:发送控制信号到服务 [%ld]成功\n",__FILE__,__LINE__,ipid);
	}else
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:发送控制信号到服务 [%ld]失败：[%s]\n",__FILE__,__LINE__,ipid,strerror(errno));
		if(send(clifd,"error",strlen("error"),0)==-1)
		{
			SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:反馈渠道错误信息失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		}
		/**还需要删除消息队列信息，防止堵塞 **/
		if(delete_shm_hash(mbuf->innerid)==-1)
		{
			SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:删除共享内存hash表数据失败\n",__FILE__,__LINE__);
		}
		if(msgrcv(msgido,mbuf,sizeof(mbuf->tranbuf),mbuf->innerid,0)==-1)
		{
			SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:删除消息队列数据失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		}
		updatestat_foroth(ipid);
		free(tranbuffer);
		return -1;
	}
	iret = msgrcv(msgidi,mbuf,sizeof(mbuf->tranbuf),mbuf->innerid,0);
	if(iret < 0)
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:获取服务返回失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		if(send(clifd,"交易处理失败",strlen("交易处理失败"),0)==-1)
		{
			SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:反馈渠道错误信息失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		}
		free(tranbuffer);
		return -1;
	}else
	{
		SysLog(LOG_CHNL_SHOW,"FILE [%s] LINE [%d]:获取到服务返回信息\n",__FILE__,__LINE__);
		if(get_shm_hash(mbuf->innerid,tranbuf)!=-1)
		{
			SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:获取到服务返回信息，跟踪号[%ld]返回信息[%s]\n",__FILE__,__LINE__,mbuf->innerid,tranbuf->outtran);
			if(write(clifd,tranbuf->outtran,strlen(tranbuf->outtran))==-1)
			{
				SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:反馈渠道错误信息失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
			}
			memset(tranbuf,0,sizeof(_tran));
			free(tranbuffer);
			return  0;
		}else
		{
			SysLog(LOG_CHNL_SHOW,"FILE [%s] LINE [%d]:获取到服务返回信息错误跟踪号[%ld]\n",__FILE__,__LINE__,mbuf->innerid);
			if(send(clifd,"error",strlen("error"),0)==-1)
			{
				SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:反馈渠道错误信息失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
			}
			free(tranbuffer);
			return  -1;
		}
	}
	free(tranbuffer);
	return 0;
}
