#include "ups.h"

unsigned int hashfunc(char *str)
{
#ifdef ll
	unsigned int hash = 0;
	unsigned int x =0;
	unsigned int i = 0;
	for(i=0; i<strlen(str); str++,i++)
	{
		hash = (hash << 4)+(*str);
		if((x=hash&0XF0000000L)!=0)
		{
			hash ^=(x >> 24);
		}
		hash &= ~x;
	}
	return hash%HASHCNT;
#endif 
	unsigned int seed = 131; // 31 131 1313 13131 131313 etc..  
	unsigned int hash = 0;  

	while (*str)  
	{  
		hash = hash * seed + (*str++);  
	}  

	return ((hash & 0x7FFFFFFF)%HASHCNT); 
#ifdef lilei2
	unsigned int b    = 378551;
	unsigned int a    = 63689;
	unsigned int hash = 0;
	int i;

	for(i = 0; i < strlen(str); i++)
	{
		hash = hash * a + str[i];
		a    = a * b;
	}

	return hash%HASHCNT;
#endif 
}

int shm_hash_insert(long innerid,char *intran,char *outtran)
{
	_tran *transhm =NULL;
	key_t   key;
	char inpid[20];
	int pos=0,i=0;
	int 	iret=0;
	memset(inpid,0,sizeof(inpid));

	sprintf(inpid,"2%010ld",innerid);

	size_t shmsize = HASHCNT*BUCKETSCNT*sizeof(_tran);
	int shmid;

	if((key = ftok("/item/ups/etc/mq_1",10))==-1)
	{
		SysLog(1,"获取hash存储区主键失败");
		return -1;
	}
	if((shmid = shmget(key,shmsize,IPC_EXCL))==-1)
	{
		SysLog(1,"获取hash存储区共享内存失败");
		return -1;
	}
	transhm = (_tran *)shmat(shmid,NULL,0);
	if(transhm ==  (void *)-1)
	{
		SysLog(1,"连接hash存储区共享内存失败");
		return -1;
	}
	pos = hashfunc(inpid);
	for(i=0;i<BUCKETSCNT;i++)
	{
		if((transhm+pos+i)->innerid == 0)
		{
			iret = sem_trywait(&((transhm+pos+i)->sem1));
			if(iret !=0&&errno==EAGAIN)
			{
				SysLog(1,"FILE[%s]LINE[%d]暂时无可用hash空间\n",__FILE__,__LINE__);
			}else if(iret == 0)
			{
				(transhm+pos+i)->innerid = innerid;
				if(intran!=NULL)
				{
					strcpy((transhm+pos+i)->intran,intran);
				}
				if(outtran!=NULL)
				{
					strcpy((transhm+pos+i)->outtran,outtran);
				}
				(transhm+pos+i)->stat[0]='N';
				sem_post(&((transhm+pos+i)->sem1));
				shmdt(transhm);
				return 0;
			}else
			{
				/** 添加hash表错误 **/
				SysLog(1,"FILE[%s]LINE[%d]添加hash错误\n",__FILE__,__LINE__);
				shmdt(transhm);
				return -1;
			}
		}
	}
	SysLog(1,"FILE[%s] LINE[%d]HASH桶满\n",__FILE__,__LINE__);
	shmdt(transhm);
	return -1;
}
int get_shm_hash(long innerid,_tran *tranbuf)
{
	_tran *transhm =NULL;
	key_t   key;
	size_t shmsize = HASHCNT*BUCKETSCNT*sizeof(_tran);
	int shmid;

	char inpid[20];
	int pos=0,i=0;
	memset(inpid,0,sizeof(inpid));

	sprintf(inpid,"2%010ld",innerid);

	if((key = ftok("/item/ups/etc/mq_1",10))==-1)
	{
		SysLog(1,"获取hash存储区主键失败");
		return -1;
	}
	if((shmid = shmget(key,shmsize,IPC_EXCL))==-1)
	{
		SysLog(1,"获取HASH存储区共享内存失败");
		return -1;
	}
	transhm = (_tran *)shmat(shmid,NULL,0);
	if(transhm ==  (void *)-1)
	{
		SysLog(1,"链接HASH存储区共享内存失败");
		return -1;
	}
	pos = hashfunc(inpid);
	for(i=0;i<BUCKETSCNT;i++)
	{
		SysLog(1,"fucking FILE[%s] LINE[%d] pos[%d] i[%d] intran[%s] outtran[%s]\n",__FILE__,__LINE__,pos,i,(transhm+pos+i)->intran,(transhm+pos+i)->outtran);
		if((transhm+pos+i)->innerid == innerid)
		{
			//printf("in hash intran[%s]\t outtran[%s]\n",(transhm+pos+i)->intran,(transhm+pos+i)->outtran);
			strcpy(tranbuf->intran,(transhm+pos+i)->intran);
			strcpy(tranbuf->outtran,(transhm+pos+i)->outtran);
			tranbuf->stat[0]='N';
			shmdt(transhm);
			return 0;
		}
	}
	shmdt(transhm);
	return -1;
}

int delete_shm_hash(long innerid)
{
	_tran *transhm =NULL;
	key_t   key;
	size_t shmsize = HASHCNT*BUCKETSCNT*sizeof(_tran);
	int shmid;

	char inpid[20];
	int pos=0,i=0;
	memset(inpid,0,sizeof(inpid));

	sprintf(inpid,"2%010ld",innerid);

	if((key = ftok("/item/ups/etc/mq_1",10))==-1)
	{
		SysLog(1,"获取HASH存储区主键失败");
		return -1;
	}
	if((shmid = shmget(key,shmsize,IPC_EXCL))==-1)
	{
		SysLog(1,"获取HASH存储区恭喜内存失败");
		return -1;
	}
	transhm = (_tran *)shmat(shmid,NULL,0);
	if(transhm ==  (void *)-1)
	{
		SysLog(1,"链接HASH存储区恭喜内存失败");
		return -1;
	}
	pos = hashfunc(inpid);
	for(i=0;i<BUCKETSCNT;i++)
	{
		if((transhm+pos+i)->innerid == innerid)
		{
			sem_wait(&((transhm+pos+i)->sem1));
			memset(transhm+pos+i,0,sizeof(_tran));
			sem_post(&((transhm+pos+i)->sem1));
			shmdt(transhm);
			return 0;
		}
	}
	shmdt(transhm);
	return -1;
}

/** init var hash **/
int init_var_hash(void)
{
	int i = 0;
	kvalue  = (_keyvalue *)malloc(HASHCNT*sizeof(_keyvalue));
	if(kvalue == NULL)
	{
		SysLog(1,"申请服务变量存放区内存失败\n");
		return -1;
	}
	for(i=0;i<HASHCNT;i++)
	{
		(kvalue+i)->pre == NULL;
		(kvalue+i)->next == NULL;
		memset((kvalue+i)->varname,0,sizeof((kvalue+i)->varname));
	}
	return 0;
}
int put_var_value(char *varname,int len,int loop,char *value)
{
	_keyvalue *tmpkvalue;
	_keyvalue *pre;
	char varnameloop[1024];
	int hash = 0;

	if(kvalue == NULL||varname ==NULL||strlen(varname)==0)
	{
		SysLog(1,"服务变量存放区内存未申请或变量名为空\n");
		return -1;
	}
	if(loop==0)
	{
		hash = hashfunc(varname);
		SysLog(1,"FILE[%s] LINE[%d] 变量值[%s]HASH值[%d]\n",__FILE__,__LINE__,varname,hash);
		tmpkvalue = kvalue+hash;

		if(tmpkvalue->varname[0]==' ')
		{
			/** 注意注意，需要根据变量定义来分配大小，否则可能导致后期问题 **/
			SysLog(1,"原内存无该变量，第一次赋值 \n");
			strcpy(tmpkvalue->varname,varname);
			//tmpkvalue->value = (char *)malloc(len*sizeof(char));
			tmpkvalue->value = (char *)malloc(len);
			if(tmpkvalue->value == NULL)
			{
				SysLog(1,"申请变量值存放内存失败\n");
				return -1;
			}
			//strncpy(tmpkvalue->value,value,len*sizeof(char));
			memcpy(tmpkvalue->value,value,len);
			SysLog(1,"FILE[%s] LINE[%d] 变量值[%s]传入值[%s]放入后值[%s]\n",__FILE__,__LINE__,varname,value,tmpkvalue->value);
			return 0;
		}else
		{
			int i=0;
			while(tmpkvalue !=NULL)
			{
				pre=tmpkvalue;
				/** fangzhi  chongxin malloc **/
				if(!strcmp(tmpkvalue->varname,varname))
				{
					memset(tmpkvalue->value,0x00,strlen(tmpkvalue->value));
					//strncpy(tmpkvalue->value,value,len*sizeof(char));
					memcpy(tmpkvalue->value,value,len);
					return 0;
				}
				tmpkvalue = tmpkvalue->next;
				i++;
				//SysLog(1,"第[%d]次查找变量\n",i);
			}
			tmpkvalue = (_keyvalue *)malloc(sizeof(_keyvalue));
			if(tmpkvalue == NULL)
			{
				SysLog(1,"申请新的keyvalue失败\n");
				return -1;
			}
			tmpkvalue->end=tmpkvalue;
			tmpkvalue->pre=pre;
			pre->next=tmpkvalue;
			tmpkvalue->next=NULL;
			strcpy(tmpkvalue->varname,varname);
			//tmpkvalue->value = (char *)malloc(len*sizeof(char));
			tmpkvalue->value = (char *)malloc(len);
			if(tmpkvalue->value == NULL)
			{
				SysLog(1,"申请变量值内存区失败\n");
				return -1;
			}
			//strncpy(tmpkvalue->value,value,len*sizeof(char));
			strncpy(tmpkvalue->value,value,len);
			SysLog(1,"FILE[%s] LINE[%d] 变量值[%s]传入值[%s]放入后值[%s]\n",__FILE__,__LINE__,varname,value,tmpkvalue->value);
			return  0;
		}
	}else
	{
		memset(varnameloop,0,sizeof(varnameloop));
		sprintf(varnameloop,"%s_%d_%c",varname,loop,varname[1]);
		hash = hashfunc(varnameloop);
		SysLog(1,"FILE[%s] LINE[%d] 变量值[%s]HASH值[%d]\n",__FILE__,__LINE__,varname,hash);
		SysLog(1,"FILE[%s] LINE[%d] 变量值[%s]HASH值[%d]\n",__FILE__,__LINE__,varnameloop,hash);
		tmpkvalue = kvalue+hash;
		if(tmpkvalue->varname[0]==' ')
		{
			SysLog(1,"原内存无该变量，第一次赋值 \n");
			strcpy(tmpkvalue->varname,varname);
			//tmpkvalue->value = (char *)malloc(len*sizeof(char));
			tmpkvalue->value = (char *)malloc(len);
			if(tmpkvalue->value == NULL)
			{
				SysLog(1,"申请变量值内存失败\n");
				return -1;
			}
			//strncpy(tmpkvalue->value,value,len*sizeof(char));
			memcpy(tmpkvalue->value,value,len);
			SysLog(1,"FILE[%s] LINE[%d] 变量值[%s]传入值[%s]放入后值[%s]长度[%d]\n",__FILE__,__LINE__,varname,value,tmpkvalue->value,len);
			return 0;
		}else
		{
			int i=0;
			while(tmpkvalue !=NULL)
			{
				pre=tmpkvalue;
				/** fangzhi  chongxin malloc **/
				if(!strcmp(tmpkvalue->varname,varname))
				{
					memset(tmpkvalue->value,0,strlen(tmpkvalue->value));
					memcpy(tmpkvalue->value,value,len);
					return 0;
				}
				tmpkvalue = tmpkvalue->next;
				i++;
				//SysLog(1,"第[%d]次查找变量\n",i);
			}
			tmpkvalue = (_keyvalue *)malloc(sizeof(_keyvalue));
			if(tmpkvalue == NULL)
			{
				SysLog(1,"申请tmpkeyvalue内存失败\n");
				return -1;
			}
			tmpkvalue->end=tmpkvalue;
			tmpkvalue->pre=pre;
			pre->next=tmpkvalue;
			tmpkvalue->next=NULL;
			strcpy(tmpkvalue->varname,varname);
			//tmpkvalue->value = (char *)malloc(len*sizeof(char));
			tmpkvalue->value = (char *)malloc(len);
			if(tmpkvalue->value == NULL)
			{
				SysLog(1,"申请变量值内存失败\n");
				return -1;
			}
			//strncpy(tmpkvalue->value,value,len*sizeof(char));
			memcpy(tmpkvalue->value,value,len);
			SysLog(1,"FILE[%s] LINE[%d] 变量值[%s]传入值[%s]放入后值[%s]长度[%d]\n",__FILE__,__LINE__,varname,value,tmpkvalue->value,len);
			return  0;
		}
		return 0;
	}
	return 0;
}
int get_var_value(char *varname,int *len,int loop,char *value)
{
	_keyvalue *tmpkvalue;
	char varnameloop[1024];
	int hash = 0;
	if(kvalue == NULL)
	{
		SysLog(1,"进程变量值内存空间未申请 \n");
		return -1;
	}
	if(loop == 0)
	{
		hash = hashfunc(varname);
		SysLog(1,"变量[%s]HASH值为[%d]\n",varname,hash);
	}else
	{
		memset(varnameloop,0,sizeof(varnameloop));
		//sprintf(varnameloop,"%s[%d]",varname,loop);
		sprintf(varnameloop,"%s_%d_%c",varname,loop,varname[1]);
		hash = hashfunc(varnameloop);
		SysLog(1,"变量[%s]HASH值为[%d]\n",varnameloop,hash);
	}
	tmpkvalue = kvalue+hash;
	while(tmpkvalue!=NULL)
	{
		//SysLog(1,"变量名[%s]变量值为[%s]\n",tmpkvalue->varname,tmpkvalue->value);
		if(!strcmp(tmpkvalue->varname,varname))
		{
			SysLog(1,"本次获取变量名[%s]变量值为[%s]\n",varname,tmpkvalue->value);
			strcpy(value,tmpkvalue->value);
			return 0;
		}
		tmpkvalue = tmpkvalue->next;
	}
	return -1;
}
void destory_var_hash(void)
{
	int i = 0;
	_keyvalue *endkvalue;
	if(kvalue==NULL)
		return ;
	while(i<HASHCNT)
	{
		SysLog(1,"start at [%d]\n",i);
		endkvalue = kvalue+i;
		if(endkvalue->next==NULL)
		{
			i++;
			continue;
		}
		while(endkvalue!=NULL)
		{
			endkvalue = endkvalue->next;
		}
		while(endkvalue->pre!=NULL)
		{
			free(endkvalue->value);
			free(endkvalue);
			endkvalue = endkvalue->pre;
		}
		i++;
	}
	free(kvalue);
	return ;
}
int shm_hash_update(long innerid,char *intran,char *outtran)
{
	_tran *transhm =NULL;
	key_t   key;
	char inpid[20];
	int pos=0,i=0;
	int	iret=0;
	memset(inpid,0,sizeof(inpid));

	sprintf(inpid,"2%010ld",innerid);

	size_t shmsize = HASHCNT*BUCKETSCNT*sizeof(_tran);
	int shmid;

	if((key = ftok("/item/ups/etc/mq_1",10))==-1)
	{
		SysLog(1,"获取hash存储区主键失败");
		return -1;
	}
	if((shmid = shmget(key,shmsize,IPC_EXCL))==-1)
	{
		SysLog(1,"获取hash存储区共享内存失败");
		return -1;
	}
	transhm = (_tran *)shmat(shmid,NULL,0);
	if(transhm ==  (void *)-1)
	{
		SysLog(1,"连接hash存储区共享内存失败");
		return -1;
	}
	pos = hashfunc(inpid);
	for(i=0;i<BUCKETSCNT;i++)
	{
		if((transhm+pos+i)->innerid == innerid)
		{
			//iret = sem_trywait(&((transhm+pos+i)->sem1));
			iret = sem_wait(&((transhm+pos+i)->sem1));
			/**
			if(iret !=0&&errno==EAGAIN)
			{
				SysLog(1,"FILE[%s]LINE[%d]暂时无可用hash空间\n",__FILE__,__LINE__);
				i++;
				continue;
			}
			**/
			if(intran!=NULL)
			{
				memset((transhm+pos+i)->intran,0,sizeof((transhm+pos+i)->intran));
				strcpy((transhm+pos+i)->intran,intran);
			}
			if(outtran!=NULL)
			{
				memset((transhm+pos+i)->outtran,0,sizeof((transhm+pos+i)->outtran));
				strcpy((transhm+pos+i)->outtran,outtran);
			}
			(transhm+pos+i)->stat[0]='N';
			sem_post(&((transhm+pos+i)->sem1));
			shmdt(transhm);
			return 0;
		}
	}
	SysLog(1,"FILE[%s] LINE[%d]HASH桶满,更新失败\n",__FILE__,__LINE__);
	shmdt(transhm);
	return -1;
}
