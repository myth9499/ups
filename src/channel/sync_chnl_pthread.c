#include "ups.h"
#include <semaphore.h>

char chnl_name[20];
char	syncflag[2];
int iret = 0;
int msgidi=0,msgido=0;
_msgbuf *mbuf=NULL;
_tran *tranbuf=NULL;
long i=1L;

long getseq(void)
{
	/**
	  long seq = 0;
	  int fp =0;
	  struct flock fk;
	  char buff[21];

	  fk.l_type = F_WRLCK;
	  fk.l_start = 0;
	  fk.l_whence = SEEK_SET;
	  fk.l_len =0;
	  fp = open("seq.dat",O_RDWR,"S_IRWXU");
	  if(fp == -1)
	  {
	  printf("file open error\n");
	  return -1;
	  }
	  if(fcntl(fp,F_SETLK,&fk)==-1)
	  {
	  printf("file lock error\n");
	  return -1;
	  }
	  memset(buff,0,sizeof(buff));
	  if(read(fp,buff,sizeof(buff))!=-1)
	  {
	  seq = atol(buff);
	  fk.l_type = F_UNLCK;
	  fk.l_start = 0;
	  fk.l_whence = SEEK_SET;
	  fk.l_len =0;
	  seq++;
	  memset(buff,0,sizeof(buff));
	  sprintf(buff,"%ld",seq);
	  if(write(fp,buff,strlen(buff))!=strlen(buff))
	  {
	  printf("write unlock error\n");
	  }
	  if(fcntl(fp,F_SETLK,&fk)==-1)
	  {
	  printf("file unlock error\n");
	  return -1;
	  }
	  close(fp);
	  return seq;
	  }else
	  {
	  printf("file read error\n");
	  fk.l_type = F_UNLCK;
	  fk.l_start = 0;
	  fk.l_whence = SEEK_SET;
	  fk.l_len =0;
	  if(fcntl(fp,F_SETLK,&fk)==-1)
	  {
	  printf("file unlock error\n");
	  return -1;
	  }
	  close(fp);
	  return seq;
	  }
	  return -1;
	 **/
	srand((unsigned)time(NULL));
	return (long)pthread_self()+rand()%100;
}

void * chnlprocess(void *arg);
	sem_t	sem;
int main(int argc,char *argv[])
{
	int flag =0;
	int sockfd,clifd;
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	int sin_size = 0;


	memset(chnl_name,0,sizeof(chnl_name));
	memset(syncflag,0,sizeof(syncflag));

	strcpy(chnl_name,"测试渠道");
	syncflag[0]='S';

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
		return -1;
	}

	if(getmsgid(chnl_name,&msgidi,&msgido)==-1)
	{
		printf("get msgid error\n");
		return -1;
	}

	/** 设置忽略SIGPIPE信号，防止因socket写的时候客户端关闭导致的SIGPIPE信号 **/
	//signal(SIGPIPE,SIG_IGN);
#ifndef WIN32
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
		perror("创建服务器套接字失败");
		return -1;
	}
	printf("创建服务器套接字成功\n");
	bzero(&serv_addr,sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(10000);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sockfd,(struct sockaddr *)(&serv_addr),sizeof(struct sockaddr))==-1)
	{
		perror("bind 失败");
		return -1;
	}
	if(listen(sockfd,10000)==-1)
	{
		perror("listen 失败");
		return -1;
	}
	/**初始化线程池 **/
	sem_init(&sem,0,0);
	pool_init(100);
	while(1)
	{
		sin_size = sizeof(struct sockaddr_in);
		/**clifd =(int *)malloc(sizeof(int));
		  if(clifd == NULL)
		  {
		  perror("malloc clifd error");
		  return -1;
		  }
		 **/
		/** one commit **/
		if((clifd = accept(sockfd,(struct sockaddr *)(&cli_addr),&sin_size))==-1)
		{
			perror("accept 失败");
			continue;
			//return -1;
		}
		//printf("获取来自[%s]的连接 sockfd is[%d]\n",inet_ntoa(cli_addr.sin_addr),clifd);
		if(pool_add_worker(chnlprocess,&clifd)!=-1)
		{
			sem_post(&sem);
			printf("添加工作任务成功\n");
		}else
		{
			printf("添加工作任务失败,请检查是否已达到初始线程最大值\n");
			send(clifd,"添加工作任务失败,请检查是否已达到初始线程最大值",strlen("添加工作任务失败,请检查是否已达到初始线程最大值"),0);
			close(clifd);
			sleep(1);
		}
	}
	free(tranbuf);
	free(mbuf);
	return 0;
}
void *chnlprocess(void *arg)
{
	sem_wait(&sem);
#ifndef WIN32
	sigset_t signal_mask;
	sigemptyset (&signal_mask);
	sigaddset (&signal_mask, SIGPIPE);
	int rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
	if (rc != 0) {
		printf("block sigpipe error/n");
	} 
#endif  
	char rcvbuf[4096];
	int clifd = 0;
	clifd = *(int *)arg;
	char tranid[5];
	char tranvalue[4096];
	char errmsg[1024];
	char rtmsg [1024];

	memset(errmsg,0,sizeof(errmsg));
	memset(rtmsg,0,sizeof(rtmsg));
	memset(rcvbuf,0,sizeof(rcvbuf));

	printf("thread is 0x%x,working on task sockfd[%d]\n",pthread_self(),clifd);
	if(recv(clifd,rcvbuf,sizeof(rcvbuf),0)==-1)
	{
		perror("read error");
		strcpy(errmsg,"read clifd error");
		if(send(clifd,errmsg,strlen(errmsg),0)==-1)
		{
			perror("write error");
			printf("!!!!error clifd is [%d]\n",clifd);
		}
		//sleep (1);
		while(1)
		{
			if(recv(clifd,rcvbuf,sizeof(rcvbuf),0)==0)
			{
				close(clifd);
				return 0;
			}
		}
		//return NULL;
	}
	printf("rcvbuf is [%s]\n",rcvbuf);
	memset(mbuf,0,sizeof(mbuf));
	//mbuf->innerid = i;
	srand((unsigned)time(NULL));
	mbuf->innerid =  (long)pthread_self()+rand()%100+rand()%33;
	sprintf(mbuf->tranbuf,"%s|%s|[2%010ld]",chnl_name,syncflag,i);
	i++;
	if(shm_hash_insert(mbuf->innerid,rcvbuf,NULL)==-1)
	{
		printf("insert hash shm error\n");
		while(1)
		{
			if(recv(clifd,rcvbuf,sizeof(rcvbuf),0)==0)
			{
				close(clifd);
				return 0;
			}
		}
		return NULL;
	}
	printf("insert hash shm ok\n");

	iret = msgsnd(msgidi,mbuf,strlen(mbuf->tranbuf),IPC_NOWAIT);
	if(iret == -1)
	{
		printf("msg send error[%s]\n",strerror(errno));
		while(1)
		{
			if(recv(clifd,rcvbuf,sizeof(rcvbuf),0)==0)
			{
				close(clifd);
				return 0;
			}
		}
		return NULL;
	}
	printf("msg send ok[%ld]\n",mbuf->innerid);
	iret = msgrcv(msgido,mbuf,sizeof(mbuf->tranbuf),mbuf->innerid,0);
	if(iret == -1)
	{
		printf("msg recv error[%s]\n",strerror(errno));
		while(1)
		{
			if(recv(clifd,rcvbuf,sizeof(rcvbuf),0)==0)
			{
				close(clifd);
				return 0;
			}
		}
		return NULL;
	}
	if(get_shm_hash(mbuf->innerid,tranbuf)!=-1)
	{
		printf("innerid[%ld]\toutbuf[%s]\n",mbuf->innerid,tranbuf->outtran);
		if(write(clifd,tranbuf->outtran,strlen(tranbuf->outtran))==-1)
		{
			perror("write error");
			printf("error clifd is [%d]\n",clifd);
			while(1)
			{
				if(recv(clifd,rcvbuf,sizeof(rcvbuf),0)==0)
				{
					close(clifd);
					return 0;
				}
			}
			//close(clifd);
		}
		memset(tranbuf,0,sizeof(_tran));
		delete_shm_hash(mbuf->innerid);
		printf("return ok\n");
		while(1)
		{
			if(recv(clifd,rcvbuf,sizeof(rcvbuf),0)==0)
			{
				sleep (1);
				close(clifd);
				return 0;
			}
		}
	}
}
