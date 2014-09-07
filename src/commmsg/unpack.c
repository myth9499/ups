#include "ups.h"
int getshmid(int procid,size_t shmsize)
{
	int shmid;
	key_t	key;
	if((key = ftok("/item/ups/etc/mq_1",procid))==-1)
	{
		SysLog(1,"FILE [%s] LINE[%d] 获取渠道交易区共享内存主键失败:%s\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	if((shmid = shmget(key,shmsize,IPC_EXCL))==-1)
	{
		SysLog(1,"FILE [%s] LINE[%d] 获取渠道交易区共享内存失败:%s\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	return shmid;
}
int unpack(char *chnlname,char *commbuf)
{
	key_t	key;
	int shmid,i=0;
	_commmsg *cmsg=NULL;
	char *tmpbuf = NULL;
	_commmsg *tmpshmdt=NULL;
	size_t shmsize;
	int flag = 0;
	/** init commmsg **/
	shmsize=MAXCOMMMSG*sizeof(_commmsg);
	if((shmid=getshmid(9,shmsize))==-1)
	{
		SysLog(1,"FILE [%s] LINE[%d] 获取渠道交易区共享内存失败:%s\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	if((chnlname==NULL)||(commbuf==NULL))
	{
		SysLog(1,"FILE [%s] LINE[%d] 解包传入参数有误\n",__FILE__,__LINE__);
		return -1;
	}
	SysLog(1,"FILE [%s] LINE[%d] start unpack commbuf[%s] from channel[%s]\n",__FILE__,__LINE__,commbuf,chnlname);
	cmsg = (_commmsg *)shmat(shmid,NULL,0);
	if(cmsg == NULL)
	{
		SysLog(1,"FILE [%s] LINE[%d] 连接渠道交易区共享内存失败:%s\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	tmpshmdt = cmsg;
	while(strcmp(cmsg->commname,"END"))
	{
		//SysLog(1,"FILE[%s] LINE[%d]渠道配置名称[%s]\n",__FILE__,__LINE__,cmsg->commname);
		if(!strcmp(cmsg->commname,chnlname))
		{
			flag = 1;
		//	printf("commname[%s]\tlen[%d]\tcommvar[%s]\tmark[%s]\t\n",cmsg->commname,cmsg->len,cmsg->commvar,cmsg->commmark);
			SysLog(1,"commname[%s]\tlen[%d]\tcommvar[%s]\tmark[%s]\t\n",cmsg->commname,cmsg->len,cmsg->commvar,cmsg->commmark);
			if(i==0)
			{
				tmpbuf = strtok(commbuf,"|");
				if(tmpbuf==NULL)
				{
					SysLog(1,"FILE [%s] LINE[%d] 待解包数据格式错\n",__FILE__,__LINE__);
					shmdt(tmpshmdt);
					return -1;
				}
				if(strlen(tmpbuf)>cmsg->len)
				{
					SysLog(1,"FILE [%s] LINE[%d] 待解包变量长度[%d]大于待解入变量长度[%d]\n",__FILE__,__LINE__,strlen(tmpbuf),cmsg->len);
					shmdt(tmpshmdt);
					return -1;
				}
				SysLog(1,"FILE [%s] LINE [%d] 放入变量 [%s] 值为  [%s]\n",__FILE__,__LINE__,cmsg->commvar,tmpbuf);
				if(put_var_value(cmsg->commvar,cmsg->len,1,tmpbuf)!=0)
				{
					SysLog(1,"FILE [%s] LINE [%d] 放入变量 [%s] 失败\n",__FILE__,__LINE__,cmsg->commvar);
					shmdt(tmpshmdt);
					return -1;
				}
				//printf("put var[%s] ok\n",cmsg->commvar);
				i++;
				cmsg++;
				continue;
			}else
			{
				tmpbuf = strtok(NULL,"|");
				if(tmpbuf==NULL)
				{
					SysLog(1,"FILE [%s] LINE[%d] 待解包数据格式错\n",__FILE__,__LINE__);
					shmdt(tmpshmdt);
					return -1;
				}
				if(strlen(tmpbuf)>cmsg->len)
				{
					SysLog(1,"FILE [%s] LINE[%d] 待解包变量长度[%d]大于待解入变量长度[%d]\n",__FILE__,__LINE__,strlen(tmpbuf),cmsg->len);
					shmdt(tmpshmdt);
					return -1;
				}
				SysLog(1,"FILE [%s] LINE [%d] 放入变量 [%s] 值为  [%s]\n",__FILE__,__LINE__,cmsg->commvar,tmpbuf);
				if(put_var_value(cmsg->commvar,cmsg->len,1,tmpbuf)!=0)
				{
					SysLog(1,"FILE [%s] LINE [%d] 放入变量 [%s] 失败\n",__FILE__,__LINE__,cmsg->commvar);
					return -1;
				}
				//printf("put var[%s] ok\n",cmsg->commvar);
				i++;
				cmsg++;
				continue;
			}
		}
		/**
		else if(!strcmp(cmsg->commname,"over"))
		{
			printf("cfg do ok \n");
			shmdt(tmpshmdt);
			return 0;
		}
		**/
		cmsg++;
	}
	shmdt(tmpshmdt);
	if(flag == 0)
	{
		SysLog(1,"未发现[%s]格式的相关配置 \n",chnlname);
		return -1;
	}else
	{
		return 0;
	}
}
int pack(char *msgtype)
{
		//strcpy(tranbuf->outtran,tranbuf->intran);
		char outtran[1024];
		memset(outtran,0,sizeof(outtran));

		SysLog(1,"开始打包[%s]跟踪号INNERPID[%ld]\n",msgtype,innerid);
		int iret ;

		int shmid,i=0;
		_commmsg *cmsg=NULL;
		char *tmpbuf = NULL;
		_commmsg *tmpshmdt=NULL;
		size_t shmsize;
		int flag = 0;
		/** init commmsg **/
		shmsize=MAXCOMMMSG*sizeof(_commmsg);
		if((shmid=getshmid(9,shmsize))==-1)
		{
			SysLog(1,"获取渠道配置区共享内存失败\n");
			return -1;
		}
		if((msgtype==NULL))
		{
			SysLog(1,"待打包报文类型不正确\n");
			return -1;
		}
		//printf("start pack msgtype[%s] \n",msgtype);
		cmsg = (_commmsg *)shmat(shmid,NULL,0);
		if(cmsg == NULL)
		{
			SysLog(1,"链接共享内存区失败\n");
			return -1;
		}
		memset(cmsg,0,sizeof(cmsg));
		tmpshmdt = cmsg;
		while(strcmp(cmsg->commname,"END"))
		{
			//printf("cmsg commname[%s]\n",cmsg->commname);
			if(!strcmp(cmsg->commname,msgtype))
			{
				flag = 1;
				SysLog(1,"待打包报文类型[%s]\t变量长度[%d]\t变量名[%s]\t备注[%s]\t\n",cmsg->commname, cmsg->len,cmsg->commvar,cmsg->commmark);
				//tmpbuf = (char *)malloc(cmsg->len+sizeof(char)+1);
				tmpbuf = (char *)malloc(cmsg->len+sizeof(char)+1);
				if(tmpbuf == NULL)
				{
					SysLog(1,"申请变量临时内存失败\n");
					shmdt(tmpshmdt);
					break;
				}
				memset(tmpbuf,0,sizeof(tmpbuf));
				if(get_var_value(cmsg->commvar,cmsg->len,1,tmpbuf)==-1)
				{
					SysLog(1,"获取变量[%s]失败\n",cmsg->commvar);
					free(tmpbuf);
					shmdt(tmpshmdt);
					return -1;
				}
				SysLog(1,"获取变量[%s]成功,value[%s]\n",cmsg->commvar,tmpbuf);
				sprintf(tmpbuf,"%s|",tmpbuf);
				strncat(outtran,tmpbuf,strlen(tmpbuf));
				free(tmpbuf);
			}
			cmsg++;
		}
		shmdt(tmpshmdt);
		if(flag == 0)
		{
			SysLog(1,"无打包渠道为[%s]的配置 \n",msgtype);
			return -1;
		}else
		{
			SysLog(1,"打类型[%s]包成功 \n",msgtype);
		}
		/** 根据报文格式配置，打返回包**/
		iret = shm_hash_update(innerid,NULL,outtran);
		if(iret == -1)
		{
			SysLog(1,"放置打包信息到共享内存失败 \n");
			return -1;
		}
		SysLog(1,"打[%s]包成功 \n",msgtype);
		return 0;
}
