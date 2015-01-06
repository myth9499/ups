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
	size_t shmsize = MAXSERVREG*sizeof(_servreg);
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
		printf("开始停止:进程名称:%-30s\t进程类型:%-6s\t进程号:%ld\t进程状态:%-6s\t\n",(sreg+i)->chnlname,(sreg+i)->type,(sreg+i)->servpid,(sreg+i)->stat);
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

