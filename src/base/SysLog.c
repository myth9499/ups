#include  "ups.h"
#include  "pool.h"

/** 获取当前设置的日志级别
 * 当前日志级别大于传入loglevel时进行打印
 * 小于传入loglevel时，直接进行退出
 **/
int SysLog(int loglevel,char *format,...)
{
    int ret;
	ret = getcursysloglvl();
	if(ret == -1)
	{
		ret = LOG_APP_DEBUG;//默认最低级
	}
	if	(ret>loglevel)
	{
		return 0;
	}
    FILE *fp = NULL;
    va_list argptr;
    char log_path[100];
    va_start(argptr,format);
	time_t	now;
	time(&now);
	struct tm *ttm;
	ttm = localtime(&now);

	memset(log_path,0,sizeof(log_path));
	if(loglevel==LOG_SYS_ERR||loglevel==LOG_SYS_DEBUG)
	{
    	sprintf(log_path,"%s%s",upshome,"/log/sys.log");
	}else
	{
    	//sprintf(log_path,"%s%s",upshome,"/log/log_%ld.log",innerid);
    	sprintf(log_path,"%s%s",upshome,"/log/log_APP.log");
	}
	//printf("log path is [%s]\n",log_path);
    fp = fopen(log_path,"a");
    if(fp == NULL)
    {
        perror("file open error");
        return -1;
    }
	fprintf(fp,"进程号:[%ld]\tTIME[%d:%d:%d]平台跟踪号[%ld]",(long)getpid(),ttm->tm_hour,ttm->tm_min,ttm->tm_sec,innerid);
    ret = vfprintf(fp,format,argptr);
    fclose(fp);
    va_end(argptr);
    return (ret);
}
int	getcursysloglvl(void)
{
	_sys_param	*sp = NULL;
	int	shmsize,ret;
	shmsize	=	sizeof(_sys_param);
	int	shmid;
	if((shmid=getshmid(3,shmsize))==-1)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE[%d] 获取系统公用共享内存失败:%s\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	sp = (_sys_param *)shmat(shmid,NULL,0);
	if(sp == NULL)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE[%d] 链接系统公用共享内存失败:%s\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	ret = sp->curloglvl;
	shmdt(sp);
	return ret;
}
