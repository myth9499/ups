#include "ups.h"

int main(int argc,char *argv[] )
{
	/** 初始化全局共享内存前，先获取ups根路径 **/
	if(setupshome()==-1)
	{
		printf("设置全局变量upshome错误,请检查UPSHOME环境变量是否设置\n");
		return -1;
	}

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
	for(i=0;(sreg+i)->servpid!=0;i++)
	{
		if((kill((sreg+i)->servpid,SIGUSR1)==-1)&&(errno == ESRCH))
		{
			strcpy((sreg+i)->stat,"E");
		}
		if(!strcmp((sreg+i)->stat,"N"))
		{
			printf("进程名称:%s\t进程类型:%s\t进程号:%ld\t进程状态:%s\t启动命令:[%s]\t\n",(sreg+i)->chnlname,(!strcmp((sreg+i)->type,"C"))?"渠道":"服务",(sreg+i)->servpid,"正常",(sreg+i)->startcmd);
		}else if(!strcmp((sreg+i)->stat,"L"))
		{
			printf("进程名称:%s\t进程类型:%s\t进程号:%ld\t进程状态:%s\t启动命令:[%s]\t\n",(sreg+i)->chnlname,(!strcmp((sreg+i)->type,"C"))?"渠道":"服务",(sreg+i)->servpid,"运行",(sreg+i)->startcmd);
		}else
		{
			printf("进程名称:%s\t进程类型:%s\t进程号:%ld\t进程状态:%s\t启动命令:[%s]\t\n",(sreg+i)->chnlname,(!strcmp((sreg+i)->type,"C"))?"渠道":"服务",(sreg+i)->servpid,"停止",(sreg+i)->startcmd);
		}
	}
	if(i==0)
	{
		printf("暂时无进程启动\n");
	}
	shmdt(sreg);
	return 0;
}

