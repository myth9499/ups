#include "ups.h"
int getshmid(int procid,size_t shmsize)
{
	int shmid;
	key_t	key;
	if((key = ftok("/item/ups/etc/mq_1",procid))==-1)
	{
		perror("ftok error");
		return -1;
	}
	if((shmid = shmget(key,shmsize,IPC_EXCL))==-1)
	{
		perror("shmget error");
		return -1;
	}
	printf("shmget ok[%d]\n",shmid);
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
		printf("get shm error\n");
		return -1;
	}
	if((chnlname==NULL)||(commbuf==NULL))
	{
		printf("the msg error\n");
		return -1;
	}
	printf("start unpack commbuf[%s] from channel[%s]\n",commbuf,chnlname);
	cmsg = (_commmsg *)shmat(shmid,NULL,0);
	if(cmsg == NULL)
	{
		printf("cmsg shmat error\n");
		return -1;
	}
	tmpshmdt = cmsg;
	while(strcmp(cmsg->commname,"END"))
	{
		printf("cmsg commname[%s]\n",cmsg->commname);
		if(!strcmp(cmsg->commname,chnlname))
		{
			flag = 1;
			printf("commname[%s]\tlen[%d]\tcommvar[%s]\tmark[%s]\t\n",cmsg->commname,
					cmsg->len,cmsg->commvar,cmsg->commmark);
			if(i==0)
			{
				tmpbuf = strtok(commbuf,"|");
				if(tmpbuf==NULL)
				{
					printf("geshi cuowo error\n");
					shmdt(tmpshmdt);
					return -1;
				}
				if(strlen(tmpbuf)>cmsg->len)
				{
					printf("[%s] len is low\n",cmsg->commvar);
					shmdt(tmpshmdt);
					return -1;
				}
				printf("file [%s] line [%d] put var [%s] tmpbuf is [%s]\n",__FILE__,__LINE__,cmsg->commvar,tmpbuf);
				if(put_var_value(cmsg->commvar,cmsg->len,1,tmpbuf)!=0)
				{
					printf("put error\n");
					shmdt(tmpshmdt);
					return -1;
				}
				printf("put var[%s] ok\n",cmsg->commvar);
				i++;
				cmsg++;
				continue;
			}else
			{
				tmpbuf = strtok(NULL,"|");
				if(tmpbuf==NULL)
				{
					printf("geshi cuowo error\n");
					shmdt(tmpshmdt);
					return -1;
				}
				if(strlen(tmpbuf)>cmsg->len)
				{
					printf("[%s] len is low\n",cmsg->commvar);
					shmdt(tmpshmdt);
					return -1;
				}
				printf("file [%s] line [%d] put var [%s] tmpbuf is [%s]\n",__FILE__,__LINE__,cmsg->commvar,tmpbuf);
				if(put_var_value(cmsg->commvar,cmsg->len,1,tmpbuf)!=0)
				{
					printf("put error\n"); shmdt(tmpshmdt);
					return -1;
				}
				printf("put var[%s] ok\n",cmsg->commvar);
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
		printf("not found msg cfg \n");
		return -1;
	}else
	{
		return 0;
	}
}
int pack(char *msgtype,char *par2,char *par3)
{
		//strcpy(tranbuf->outtran,tranbuf->intran);
		char outtran[1024];
		memset(outtran,0,sizeof(outtran));

		printf("INNERPID[%ld]\n",innerid);
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
			printf("get shm error\n");
			return -1;
		}
		if((msgtype==NULL))
		{
			printf("the msg type error\n");
			return -1;
		}
		printf("start pack msgtype[%s] \n",msgtype);
		cmsg = (_commmsg *)shmat(shmid,NULL,0);
		if(cmsg == NULL)
		{
			printf("cmsg shmat error\n");
			return -1;
		}
		tmpshmdt = cmsg;
		while(strcmp(cmsg->commname,"END"))
		{
			printf("cmsg commname[%s]\n",cmsg->commname);
			if(!strcmp(cmsg->commname,msgtype))
			{
				flag = 1;
				printf("msgtype[%s]\tlen[%d]\tcommvar[%s]\tmark[%s]\t\n",cmsg->commname,
						cmsg->len,cmsg->commvar,cmsg->commmark);
				tmpbuf = (char *)malloc(cmsg->len*(sizeof(char)));
				if(tmpbuf == NULL)
				{
					printf("malloc tmp buff error\n");
					shmdt(tmpshmdt);
					break;
				}
				if(get_var_value(cmsg->commvar,cmsg->len,1,tmpbuf)==-1)
				{
					printf("get var [%s] error\n",cmsg->commvar);
					shmdt(tmpshmdt);
					return -1;
				}
				sprintf(tmpbuf,"%s|",tmpbuf);
				strncat(outtran,tmpbuf,strlen(tmpbuf));
				free(tmpbuf);
			}
			cmsg++;
		}
		shmdt(tmpshmdt);
		if(flag == 0)
		{
			printf("not found msg cfg \n");
		}else
		{
			printf("found msg cfg \n");
		}
		/** 根据报文格式配置，打返回包**/
		iret = shm_hash_update(innerid,NULL,outtran);
		if(iret == -1)
		{
			return -1;
		}
		return 0;
}
