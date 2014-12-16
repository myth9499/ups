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
			SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:打开seq文件失败:%s\n",__FILE__,__LINE__,strerror(errno));
            return  -1L;
        }
		if(flock(fileno(fp),LOCK_EX)==0)
		{
			memset(str,0x00,sizeof(str));
			if(fgets(str,sizeof(str),fp)==(void *)-1)
			{
				SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:获取SEQ失败:%s\n",__FILE__,__LINE__,strerror(errno));
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
		return	globalinnerid++;
#endif
		FILE    *fp = NULL;
		char    seqno[20];
		long        seq = 0;
        char    filepath[100];
		memset(seqno,0x00,sizeof(seqno));
		memset(filepath,0x00,sizeof(filepath));

        sprintf(filepath,"%s%s",upshome,"/etc/innerid.seq");
		fp = fopen(filepath,"r+w");
		if(fp == NULL)
		{
			SysLog(LOG_SYS_ERR,"file open error");
			return -1;
		}
		if(0==flock(fileno(fp),LOCK_EX))
		{
			fgets(seqno,sizeof(seqno),fp);
			seq = atol(seqno);
			if(seq==99999999)
				seq=1L;
			seq++;
			sprintf(seqno,"%8ld",seq);
			fseek(fp,SEEK_SET,0);
			fwrite(seqno,strlen(seqno),1,fp);
			flock(fileno(fp),LOCK_UN);
		}
		fclose(fp);
		return seq;
}
