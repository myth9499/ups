#include "ups.h"

int main(int argc,char *argv[])
{
	key_t	key;
	size_t shmsize = HASHCNT*BUCKETSCNT*sizeof(_tran);
	int msgid;

	/** init tran shm **/
	if(getshm(10,shmsize)==-1)
	{
		printf("get shm error\n");
		return -1;
	}
	/** init commmsg **/
	shmsize=MAXCOMMMSG*sizeof(_commmsg);
	if(getshm(9,shmsize)==-1)
	{
		printf("get shm error\n");
		return -1;
	}
	/** init flow **/
	shmsize=MAXFLOW*sizeof(_flow);
	if(getshm(8,shmsize)==-1)
	{
		printf("get shm error\n");
		return -1;
	}
	system("touch /item/ups/etc/测试渠道");
	if((key=ftok("/item/ups/etc/测试渠道",1))==-1)
	{
		printf("ftok error[%s]",strerror(errno));
		return -1;
	}
	if((msgid = msgget(key,IPC_CREAT|IPC_EXCL|00666))==-1)
	{
		perror("msgget error");
		return -1;
	}
	if((key=ftok("/item/ups/etc/测试渠道",2))==-1)
	{
		printf("ftok error[%s]",strerror(errno));
		return -1;
	}
	if((msgid = msgget(key,IPC_CREAT|IPC_EXCL|00666))==-1)
	{
		perror("msgget error");
		return -1;
	}
	printf("msg get ok,msgid [%d]\n",msgid);
	system("touch /item/ups/etc/测试发送渠道");
	if((key=ftok("/item/ups/etc/测试发送渠道",1))==-1)
	{
		printf("ftok error[%s]",strerror(errno));
		return -1;
	}
	if((msgid = msgget(key,IPC_CREAT|IPC_EXCL|00666))==-1)
	{
		perror("msgget error");
		return -1;
	}
	if((key=ftok("/item/ups/etc/测试发送渠道",2))==-1)
	{
		printf("ftok error[%s]",strerror(errno));
		return -1;
	}
	if((msgid = msgget(key,IPC_CREAT|IPC_EXCL|00666))==-1)
	{
		perror("msgget error");
		return -1;
	}
	printf("msg get ok,msgid [%d]\n",msgid);
	/** init server reg **/
	shmsize = MAXSERVREG*sizeof(_servreg);
	if(getshm(7,shmsize)==-1)
	{
		printf("get shm error\n");
		return -1;
	}
	printf("get serverreg shm ok \n");
	/** init xml cfg  **/
	shmsize = MAXXMLCFG*sizeof(_xmlcfg);
	if(getshm(6,shmsize)==-1)
	{
		printf("get shm error\n");
		return -1;
	}
	printf("get xmlcfg shm ok \n");
	/** init tran map cfg  **/
	shmsize = MAXTRANMAP*sizeof(_tranmap);
	if(getshm(5,shmsize)==-1)
	{
		printf("get shm error\n");
		return -1;
	}
	printf("get tranmap shm ok \n");

	/** init sem **/
	if(initservregsem()!=0)
	{
		printf("init sem id error\n");
		return -1;
	}else
	{
		printf("init posix sem ok\n");
	}
	/** system v sem 
	if(init_sem(1)==0)
	{
		printf("init sem ok\n");
	}else
	{
		printf("init sem error\n");
		return -1;
	}
	**/
	return  0;
}
