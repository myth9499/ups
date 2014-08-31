#include  "ups.h"
#include  "pool.h"

int SysLog(int loglevel,char *format,...)
{
    FILE *fp = NULL;
    va_list argptr;
    int ret;
    char log_path[100];
    va_start(argptr,format);
	time_t	now;
	time(&now);
	struct tm *ttm;
	ttm = localtime(&now);

	memset(log_path,0,sizeof(log_path));
	if(loglevel==1)
	{
    	sprintf(log_path,"/item/ups/log/sys.log");
	}else
	{
    	sprintf(log_path,"/item/ups/log/log_%ld.log",innerid);
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
