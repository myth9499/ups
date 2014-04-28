#include "ups.h"

int get_sem()
{
	key_t	key;
	int semid;

	system("touch /item/ups/etc/mq_2");
	if((key=ftok("/item/ups/etc/mq_2",1))==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取信号量主建失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		printf("ftok error[%s]",strerror(errno));
		return -1;
	}
	if((semid = semget(key,0,IPC_EXCL))==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取信号量ID失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	return  semid;
}
int init_sem(int	init_value)
{
	key_t	key;
	int semid;

	system("touch /item/ups/etc/mq_2");
	if((key=ftok("/item/ups/etc/mq_2",1))==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取信号量主建失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	if((semid = semget(key,1,IPC_CREAT|IPC_EXCL|00666))==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取信号量ID失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	return  0;
	union	semun sem_union;
	sem_union.val = init_value;
	if(semctl(semid,0,SETVAL,sem_union)==-1)
	{
		perror("sem init error");
		return -1;
	}
	return 0;
}
int del_sem(int sem_id)
{
	union semun	sem_union;
	if(semctl(sem_id,1,IPC_RMID,sem_union)==-1)
	{
		perror("sem delete");
		return -1;
	}
	return 0;
}

int sem_p(int sem_id)
{
	printf("begin sem p func\n");
	struct	sembuf	sem_buf;
	sem_buf.sem_num = 0;
	sem_buf.sem_op = -1;
	sem_buf.sem_flg = SEM_UNDO;
	if(semop(sem_id,&sem_buf,1)==-1)
	{
		perror("sem p error");
		return -1;
	}
	/** end sem p func **/
	printf("end sem p func\n");
	return 0;
}
int sem_v(int sem_id)
{
	/** begin sem v func **/
	printf("start sem v funa\n");
	struct	sembuf	sem_buf;
	sem_buf.sem_num = 0;
	sem_buf.sem_op = 1;
	sem_buf.sem_flg = SEM_UNDO;
	if(semop(sem_id,&sem_buf,1)==-1)
	{
		perror("sem v error");
		return -1;
	}
	/** end sem v func **/
	printf("end sem v funa\n");
	return 0;
}
