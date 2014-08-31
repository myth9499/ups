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
		iret = sem_trywait(&((transhm+pos+i)->sem1));
		if(iret !=0&&errno==EAGAIN)
		{
			SysLog(1,"FILE[%s]LINE[%d]暂时无可用hash空间\n",__FILE__,__LINE__);
			continue;
		}else if(iret == 0)
		{
			if((transhm+pos+i)->innerid == 0)
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
			}
			else
			{
				sem_post(&((transhm+pos+i)->sem1));
				continue;	
			}
		}
		else
		{
			/** 添加hash表错误 **/
			SysLog(1,"FILE[%s]LINE[%d]添加hash错误\n",__FILE__,__LINE__);
			shmdt(transhm);
			return -1;
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
		(kvalue+i)->head = kvalue+i; /** 链表的头指针等于NULL **/
		(kvalue+i)->pre = kvalue+i;     /** 链表的前个指针为自身 **/
		(kvalue+i)->next = NULL;     /** 链表的下一个指针为空 **/
		(kvalue+i)->value = NULL;    /** 链表的值指针为空**/
		(kvalue+i)->end = NULL;    /** 链表的值指针为空**/
		memset((kvalue+i)->varname,0,sizeof((kvalue+i)->varname)); /** 初始化变量名存放区 **/
	}
	return 0;
}

/** memset var hash **/
int	memset_var_hash(void)
{
	/** 释放所有已申请空间 **/
	int i = 0;
	_keyvalue *tmpk=NULL,*tk=NULL;
	for(i=0;i<HASHCNT;i++)
	{
		tmpk = (kvalue+i)->end;/** 指向最后一个地址 **/
		while(tmpk!=(kvalue+i)->head&&tmpk!=NULL)
		{
			SysLog(1,"FILE [%s] LINE[%d] 开始初始化变量[%s]\n",__FILE__,__LINE__,tmpk->varname);
			memset(tmpk->varname,0,sizeof(tmpk->value));
			tk=tmpk;
			free(tmpk->value);/** 释放申请的变量区 **/
			free(tmpk);       /** 释放本身 **/
			tmpk=tk->pre;
			tmpk->next=NULL;
			tk = NULL;
		}
	}
	return 0;
}
/** 利用双向链表管理 **/
int put_var_value(char *varname,int len,int loop,char *value)
{
	_keyvalue *tmpkvalue,*tmpkey,*head;
	_keyvalue *pre;
	char varnameloop[1024];
	int hash = 0;

	if(kvalue == NULL||varname ==NULL||strlen(varname)==0)
	{
		SysLog(1,"服务变量存放区内存未申请或变量名为空\n");
		return -1;
	}
	memset(varnameloop,0,sizeof(varnameloop));
	sprintf(varnameloop,"%s_%d_%c",varname,loop,varname[1]);
	hash = hashfunc(varnameloop);
	SysLog(1,"FILE[%s] LINE[%d] 变量值[%s]HASH值[%d]\n",__FILE__,__LINE__,varname,hash);
	SysLog(1,"FILE[%s] LINE[%d] 变量名[%s]HASH值[%d]变量值[%s]\n",__FILE__,__LINE__,varnameloop,hash,value);
	head = kvalue+hash;
	tmpkvalue = head;
	pre = tmpkvalue;
	while(tmpkvalue!=NULL)
	{
	SysLog(1,"FILE [%s]  LINE[%d] lilei1234原来已有申请新内存!!!!!\n",__FILE__,__LINE__);
		if(!strcmp(tmpkvalue->varname,varname))
		{
	SysLog(1,"FILE [%s]  LINE[%d] 原来已有申请新内存!!!!!\n",__FILE__,__LINE__);
			/** 初始化一次 **/
			free(tmpkvalue->value);
			tmpkvalue->value = (char *)malloc(len+1);
			if(tmpkvalue->value==NULL)
			{
				SysLog(1,"FILE[%s] LINE[%d] 变量值[%s]传入值[%s]申请内存失败：%s\n",__FILE__,__LINE__,varname,value,strerror(errno));
				return -1;
			}else
			{
				memcpy(tmpkvalue->value,value,len);
				SysLog(1,"FILE[%s] LINE[%d] [多次传入]变量值[%s]传入值[%s]放入后值[%s]长度[%d]\n",__FILE__,__LINE__,varname,value,tmpkvalue->value,len);
				return 0;
			}
		}
		pre = tmpkvalue;
		tmpkvalue=tmpkvalue->next;
	}
	/** 第一次申请内存 **/
	SysLog(1,"FILE [%s]  LINE[%d] 开始申请新内存!!!!!\n",__FILE__,__LINE__);
	tmpkey = (_keyvalue *)malloc(sizeof(_keyvalue));
	if(tmpkey == NULL)
	{
		SysLog(1,"FILE[%s] LINE[%d] 变量值[%s]传入值[%s]申请内存失败：%s\n",__FILE__,__LINE__,varname,value,strerror(errno));
		return -1;
	}
	tmpkey->value = (char *)malloc(len+1);
	if(tmpkey->value==NULL)
	{
		SysLog(1,"FILE[%s] LINE[%d] 变量值[%s]传入值[%s]申请内存失败：%s\n",__FILE__,__LINE__,varname,value,strerror(errno));
		return -1;
	}else
	{
		head->end=tmpkey;
		strcpy(tmpkey->varname,varname);
		memcpy(tmpkey->value,value,len);
		SysLog(1,"FILE[%s] LINE[%d] 第一次申请变量值[%s]传入值[%s]放入后值[%s]长度[%d]\n",__FILE__,__LINE__,varname,value,tmpkey->value,len);
		tmpkey->next = NULL;
		tmpkey->pre = pre;
		pre->next = tmpkey;	
		tmpkvalue = tmpkey;
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
		SysLog(1,"变量名[%s]变量值为[%s]\n",tmpkvalue->varname,tmpkvalue->value);
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
