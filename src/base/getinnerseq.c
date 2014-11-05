#include  "ups.h"
#include  "pool.h"
#include  <sys/file.h>

/** 获取交易跟踪号 **/

long getinnerid()
{

#ifdef lilei
        FILE    *fp ;
        char    filepath[100];
        char    str[100];
		long	ret=0;
        memset(filepath,0,sizeof(filepath));
        memset(str,0,sizeof(str));

        sprintf(filepath,"%s%s",upshome,"/etc/innerid.seq");
        fp = fopen(filepath,"r+");
        if(fp == NULL)
        {
			SysLog(1,"FILE [%s] LINE [%d]:打开seq文件失败:%s\n",__FILE__,__LINE__,strerror(errno));
            return  -1L;
        }
		if(flock(fileno(fp),LOCK_EX)==0)
		{
			memset(str,0x00,sizeof(str));
			if(fgets(str,sizeof(str),fp)==(void *)-1)
			{
				SysLog(1,"FILE [%s] LINE [%d]:获取SEQ失败:%s\n",__FILE__,__LINE__,strerror(errno));
				sprintf(str,"%8ld",ret);
				fwrite(str,strlen(str),1,fp);
				flock(fileno(fp), LOCK_UN);
				fclose(fp);
				return 1L;
			}
			ret = atol(str);
			ret = ret++;
			memset(str,0x00,sizeof(str));
			sprintf(str,"%8ld",ret);
			fwrite(str,strlen(str),1,fp);
			flock(fileno(fp), LOCK_UN);
        	fclose(fp);
			return ret ;
		}
        fclose(fp);
        return -1L;
#endif
		return	globalinnerid++;
}
