#include "ups.h"

char chnl_name[20];
char	syncflag[2];
int iret = 0;
int msgidi=0,msgido=0;


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
	printf("交易超时结束\n");
	return ;
}
/** 注册程序退出函数 
 * 释放申请的资源等信息 **/
void do_exit(void)
{
	//free(mbuf);
}
int  sendprocess(long );


int main(int argc,char *argv[])
{
	atexit(do_exit);
	int sin_size = 0;
	pid_t pid;
	_msgbuf *mbuf=NULL;


	memset(chnl_name,0,sizeof(chnl_name));
	memset(syncflag,0,sizeof(syncflag));

	strcpy(chnl_name,"测试发送渠道");
	syncflag[0]='S';

	mbuf = (_msgbuf *)malloc(sizeof(_msgbuf));
	if(mbuf == (void *)-1)
	{
		perror("malloc msgbuf error");
		return -1;
	}
	if(getmsgid(chnl_name,&msgidi,&msgido)==-1)
	{
		printf("get msgid error\n");
		return -1;
	}
	/** 设置忽略SIGPIPE信号，防止因socket写的时候客户端关闭导致的SIGPIPE信号 **/
	signal(SIGPIPE,SIG_IGN);
	signal(35,child_exit);
	signal(SIGCHLD,child_exit);
#ifdef WIN32
	sigset_t signal_mask;
	sigemptyset (&signal_mask);
	sigaddset (&signal_mask, SIGPIPE);
	int rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
	if (rc != 0) {
		printf("block sigpipe error/n");
	} 
#endif  
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
				printf("pid [%ld] mbuf[%s] errno is [%ld][%d]\n",getpid(),mbuf->tranbuf.trancode,errno,EIDRM);
				perror("msg recv error\n");
			}
			//continue;
		}
		printf("!!!!!!!![%s]\n",mbuf->tranbuf.trancode);
		pid = fork();
		if(pid == 0)
		{
			if(sendprocess(mbuf->innerid)==0)
			{
				printf("发送渠道处理成功\n");
				msgsnd(msgido,mbuf,sizeof(mbuf->tranbuf),IPC_NOWAIT);
			}else
			{
				printf("发送渠道处理失败\n");
				msgsnd(msgido,mbuf,sizeof(mbuf->tranbuf),IPC_NOWAIT);
			}
			/** 防止SIGCHLD信号丢失**/
			kill(getppid(),35);
			exit(0);
		}
	}
	free(mbuf);
	return 0;
}
int sendprocess(long inerid)
{
	/** 注册超时信号 **/
	signal(SIGALRM,timeout);
	alarm(20);
	int sockfd;

	struct sockaddr_in  servaddr;
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd <0)
	{
		printf("get sockfd error\n");
		return -1;
	}
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(12345);
	inet_pton(AF_INET,"192.168.56.102",&servaddr.sin_addr);

	if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr))<0)
	{
		perror("connect error");
		/**
		if(shm_hash_insert(inerid,"EEEEEEEE",NULL)==-1)
		{
			printf("insert hash shm error\n");
			return -1;
		}
		**/
		return -1;
	}else
	{
		if(send(sockfd,"error",strlen("error"),0)==-1)
		{
			perror("write error");
			return  -1;
		}
	}
	return 0;
}
