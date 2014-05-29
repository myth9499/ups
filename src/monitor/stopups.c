#include "ups.h"


int main(int argc,char *argv[] )
{
	int shmid = 0,i=0;
	_servreg *sreg = NULL;
	int shmsize = MAXSERVREG*sizeof(_servreg);
	if((shmid = getshmid(7,shmsize))==-1)
	{
		printf("get serv shm id error\n");
		return -1;
	}
	if((sreg = shmat(shmid,NULL,0))==NULL)
	{
		printf("shmat sreg error\n");
		return -1;
	}
	printf("开始停止UPS服务进程.........\n");
	for(i=0;(sreg+i)->servpid!=0;i++)
	{
		printf("开始停止:进程名称:%s\t进程类型:%s\t进程号:%ld\t进程状态:%s\t\n",(sreg+i)->chnlname,(sreg+i)->type,(sreg+i)->servpid,(sreg+i)->stat);
		kill((sreg+i)->servpid,9);
	}
	if(i==0)
	{
		printf("暂时无进程启动\n");
		shmdt(sreg);
		return 0;
	}
	printf("停止UPS服务进程成功\n");
	shmdt(sreg);
	return 0;
}

