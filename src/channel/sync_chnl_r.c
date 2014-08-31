#include "ups.h"

char chnl_name[20];
char	syncflag[2];
int iret = 0;
int msgidi=0,msgido=0,msgidr=0;
char	rip[16];
int	rport;


/** 主进程注册信号，当子进程退出时进行后续处理
 * 防止僵尸进程
 **/
void child_exit(int signal)
{
	pid_t   pid;
	int     stat;
	while((pid = waitpid(-1,&stat,WNOHANG))>0)
	{
		printf("child %d ternimated\n",pid);
	}
	return ;
}
/** 注册进程timeout函数，暂定超时60秒 
 * 防止交易堵死 
 * */
void timeout(int signal)
{
	SysLog(1,"交易超时结束\n");
	return ;
}
/** 注册程序退出函数 
 * 释放申请的资源等信息 **/
void do_exit(void)
{
	//free(mbuf);
}
int  sendprocess(long );


/** 传入参数
 * 渠道名称+对方地址+对方端口
 **/

int main(int argc,char *argv[])
{
	atexit(do_exit);
	int sin_size = 0;
	pid_t pid;
	_msgbuf *mbuf=NULL;


	memset(chnl_name,0,sizeof(chnl_name));
	memset(syncflag,0,sizeof(syncflag));
	memset(rip,0,sizeof(rip));

	if(argc < 4)
	{
		SysLog(1,"FILE[%s] LINE[%d] 启动参数错误:appname + channelname+remoteip+remoteport\n",__FILE__,__LINE__);
		return -1;
	}
	strcpy(chnl_name,argv[1]);
	strcpy(rip,argv[2]);
	rport = atoi(argv[3]);
	syncflag[0]='S';

	mbuf = (_msgbuf *)malloc(sizeof(_msgbuf));
	if(mbuf == (void *)-1)
	{
		SysLog(1,"FILE[%s] LINE[%d] 申请消息队列内存失败[%s]\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	if(getmsgid(chnl_name,&msgidi,&msgido,&msgidr)==-1)
	{
		SysLog(1,"FILE[%s] LINE[%d] 获取渠道[%s]消息队列失败[%s]\n",__FILE__,__LINE__,chnl_name,strerror(errno));
		return -1;
	}
	/** 设置忽略SIGPIPE信号，防止因socket写的时候客户端关闭导致的SIGPIPE信号 **/
	signal(SIGPIPE,SIG_IGN);
	//signal(SIGCHLD,child_exit);
	signal(SIGCHLD,SIG_IGN);
#ifdef WIN32
	sigset_t signal_mask;
	sigemptyset (&signal_mask);
	sigaddset (&signal_mask, SIGPIPE);
	int rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
	if (rc != 0) {
		printf("block sigpipe error/n");
	} 
#endif  
	if(insert_chnlreg(chnl_name)!=0)
	{
		SysLog(1,"FILE [%s] LINE [%d]:添加渠道到监控内存失败\n",__FILE__,__LINE__);
		return -1;
	}
	while(1)
	{
		memset(mbuf,0,sizeof(mbuf));
		if(msgrcv(msgidi,mbuf,sizeof(mbuf->tranbuf),0,0)==-1)
		{
			if(errno == EINTR)
			{
				continue;
			}else
			{
				SysLog(1,"FILE[%s] LINE[%d] 获取渠道[%s]消息队列消息失败[%s]\n",__FILE__,__LINE__,chnl_name,strerror(errno));
				sleep (1);
				continue;
			}
			//continue;
		}
		SysLog(1,"FILE[%s] LINE[%d] 渠道[%s]获取到交易码[%s]渠道跟踪号[%ld]\n",__FILE__,__LINE__,chnl_name,mbuf->tranbuf.trancode,mbuf->innerid);
		pid = fork();
		if(pid == 0)
		{
			if(sendprocess(mbuf->innerid)==0)
			{
				SysLog(1,"FILE [%s] LINE[%d] 处理成功\n",__FILE__,__LINE__);
				/** 返回核心 **/
				if(msgsnd(msgidr,mbuf,sizeof(mbuf->tranbuf),IPC_NOWAIT)==0)
				{
					SysLog(1,"FILE [%s] LINE[%d] 返回核心交易结果成功\n",__FILE__,__LINE__);
					exit(0);
				}else
				{
					SysLog(1,"FILE [%s] LINE[%d] 返回核心交易结果失败\n",__FILE__,__LINE__);
					exit(-1);
				}
			}else
			{
				SysLog(1,"FILE [%s] LINE[%d] 处理失败\n",__FILE__,__LINE__);
				/** 返回核心 **/
				if(msgsnd(msgidr,mbuf,sizeof(mbuf->tranbuf),IPC_NOWAIT)==0)
				{
					SysLog(1,"FILE [%s] LINE[%d] 返回核心交易结果成功\n",__FILE__,__LINE__);
					exit(0);
				}else
				{
					SysLog(1,"FILE [%s] LINE[%d] 返回核心交易结果失败\n",__FILE__,__LINE__);
					exit(-1);
				}
			}
			exit(0);
		}
	}
	free(mbuf);
	return 0;
}
int sendprocess(long inerid)
{
	_tran	*tranbuf=NULL;
	SysLog(1,"&&&&&&&&&&&&&&&&&FILE [%s] LINE[%d] 开始处理[%ld]&&&&&&&&&&&&&&&&&&&\n",__FILE__,__LINE__,inerid);
	/** 注册超时信号 **/
	signal(SIGALRM,timeout);
	//alarm(10);
	int sockfd;

	struct sockaddr_in  servaddr;
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd <0)
	{
		SysLog(1,"FILE [%s] LINE[%d] 获取sockfd失败:%s\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(rport);
	inet_pton(AF_INET,rip,&servaddr.sin_addr);

	if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr))<0)
	{
		SysLog(1,"FILE [%s] LINE[%d] 建立连接失败:%s\n",__FILE__,__LINE__,strerror(errno));
		close(sockfd);
		if(shm_hash_update(inerid,"EEEEEEE|建立连接失败",NULL)==-1)
		{
			SysLog(1,"更新共享内存区失败\n");
			return -1;
		}
		return -1;
	}else
	{
	SysLog(1,"FILE[%s]LINE[%d] 链接成功，开始发送....\n",__FILE__,__LINE__);
		tranbuf = (_tran *)malloc(sizeof(_tran));
		if(tranbuf == (void *)-1)
		{
			SysLog(1,"MALLOC tranbuf 失败:%s\n",strerror(errno));
			return -1;
		}
		if(get_shm_hash(inerid,tranbuf)!=-1)
		{
			/** 获取共享内存信息，outtran 读取发送到外部系统 **/
			SysLog(1,"FILE[%s]LINE[%d] 链接成功，开始发送[%s]....\n",__FILE__,__LINE__,tranbuf->outtran);
			if(send(sockfd,tranbuf->outtran,strlen(tranbuf->outtran),MSG_DONTWAIT)==-1)
			{
				SysLog(1,"发送到其他系统失败:%s\n",strerror(errno));
				close(sockfd);
				free(tranbuf);
				return  -1;
			}else
			{
				SysLog(1,"发送到其他系统成功:%ld\n",inerid);
				close(sockfd);
				free(tranbuf);
				return  0;
			}
		}else
		{
				SysLog(1,"核心无:[%ld]信息\n",innerid);
				close(sockfd);
				free(tranbuf);
				return -1;
		}
		close(sockfd);
	}	
	return 0;
}
