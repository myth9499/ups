#include  "ups.h"
#include  "pool.h"

/** 获取当前设置的日志级别
 * 当前日志级别大于传入loglevel时进行打印
 * 小于传入loglevel时，直接进行退出
 **/
int SysLog(int loglevel,char *format,...)
{
    int ret;
	char	type[5];

	if(loglevel>=LOG_SYS_DEBUG&&loglevel<=LOG_SYS_ERR)
	{
		ret = getcursysloglvl("SYS");
		memset(type,0,sizeof(type));
		strcpy(type,"sys");
	}else if(loglevel>=LOG_CHNL_DEBUG&&loglevel<=LOG_CHNL_ERR)
	{
		ret = getcursysloglvl("CHNL");
		memset(type,0,sizeof(type));
		strcpy(type,"chnl");
	}else if(loglevel>=LOG_APP_DEBUG&&loglevel<=LOG_APP_ERR)
	{
		ret = getcursysloglvl("APP");
		memset(type,0,sizeof(type));
		strcpy(type,"app");
	}else
	{
		/** 在未初始化共享内存前的日志打印 **/
		ret =1;
		memset(type,0,sizeof(type));
		strcpy(type,"sys");
	}
	if(ret == -1)
	{
		ret = 1;//默认最低级
	}
	if	(ret>loglevel)
	{
		return 0;
	}
    FILE *fp = NULL;
    va_list argptr;
    char log_path[100];
    char trancode[11];
    va_start(argptr,format);
	time_t	now;
	time(&now);
	struct tm *ttm;
	ttm = localtime(&now);

	char	year[5];
	char	mon[3];
	char	day[3];
	char	dat[11];
	
	memset(year,0,sizeof(year));
	memset(mon,0,sizeof(mon));
	memset(day,0,sizeof(day));
	memset(dat,0,sizeof(dat));

	sprintf(year,"%d",ttm->tm_year+1900);
	if((ttm->tm_mon+1)<10)
	{
		sprintf(mon,"0%d",(ttm->tm_mon+1));
	}else
	{
		sprintf(mon,"%d",(ttm->tm_mon+1));
	}
	if(ttm->tm_mday<10)
	{
		sprintf(day,"0%d",ttm->tm_mday);
	}else
	{
		sprintf(day,"%d",ttm->tm_mday);
	}

	sprintf(dat,"%s%s%s",year,mon,day);
	memset(log_path,0,sizeof(log_path));
	sprintf(log_path,"%s%s%s",upshome,"/log/",dat);
	DIR	*dp=NULL;
	dp = opendir(log_path);
	if((dp==NULL)&&(errno == ENOENT))
	{
		mkdir(log_path,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	}
	closedir(dp);
	memset(log_path,0,sizeof(log_path));
	if(!strcmp(type,"sys"))
	{
    	sprintf(log_path,"%s%s%s/%s",upshome,"/log/",dat,"sys.log");
	}else if(!strcmp(type,"chnl"))
	{
    	//sprintf(log_path,"%s%s",upshome,"/log/log_%ld.log",innerid);
    	sprintf(log_path,"%s%s%s/%s",upshome,"/log/",dat,"chnl.log");
	}else if(!strcmp(type,"app"))
	{
		/** 获取当前交易码 **/
		memset(trancode,0,sizeof(trancode));
		get_var_value("V_TRAN",sizeof(trancode),1,trancode);
		if(strlen(trancode)==0)
		{
			sprintf(log_path,"%s%s%s/%s",upshome,"/log/",dat,"app.log");
		}else
		{
			sprintf(log_path,"%s%s%s%s",upshome,"/log/",trancode,".log");
		}
	}
	//printf("log path is [%s]\n",log_path);
    fp = fopen(log_path,"a");
    if(fp == NULL)
    {
        printf("file open error:[%s]\n",log_path);
        return -1;
    }
	fprintf(fp,"进程号:[%ld]\tTIME[%d:%d:%d]平台跟踪号[%ld]",(long)getpid(),ttm->tm_hour,ttm->tm_min,ttm->tm_sec,innerid);
    ret = vfprintf(fp,format,argptr);
    fclose(fp);
    va_end(argptr);
    return (ret);
}
int	getcursysloglvl(char *type)
{
	_sys_param	*sp = NULL;
	int	shmsize,ret;
	shmsize	=	sizeof(_sys_param)*3;
	int	shmid;
	if((shmid=getshmid(3,shmsize))==-1)
	{
		SysLog(LOG_SYS,"FILE [%s] LINE[%d] 获取系统公用共享内存失败:%s\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	sp = (_sys_param *)shmat(shmid,NULL,0);
	if(sp == NULL)
	{
		SysLog(LOG_SYS,"FILE [%s] LINE[%d] 链接系统公用共享内存失败:%s\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	int loop=0;
	for(loop=0;loop<3;loop++)
	{
		if(!strcmp((sp+loop)->type,type))
		{
			ret = (sp+loop)->curloglvl;
			break;
		}
	}
	shmdt(sp);
	return ret;
}
