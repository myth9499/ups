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
	memset(inpid,0,sizeof(inpid));

	sprintf(inpid,"2%010ld",innerid);

	size_t shmsize = HASHCNT*BUCKETSCNT*sizeof(_tran);
	int shmid;

	if((key = ftok("/item/ups/etc/mq_1",10))==-1)
	{
		perror("ftok error");
		return -1;
	}
	if((shmid = shmget(key,shmsize,IPC_EXCL))==-1)
	{
		perror("shmget error");
		return -1;
	}
	transhm = (_tran *)shmat(shmid,NULL,0);
	if(transhm ==  (void *)-1)
	{
		perror("shmat error");
		return -1;
	}
	pos = hashfunc(inpid);
	for(i=0;i<BUCKETSCNT;i++)
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
			shmdt(transhm);
			return 0;
		}
	}
	printf("hash bucket full\n");
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
		perror("ftok error");
		return -1;
	}
	if((shmid = shmget(key,shmsize,IPC_EXCL))==-1)
	{
		perror("shmget error");
		return -1;
	}
	transhm = (_tran *)shmat(shmid,NULL,0);
	if(transhm ==  (void *)-1)
	{
		perror("shmat error");
		return -1;
	}
	pos = hashfunc(inpid);
	for(i=0;i<BUCKETSCNT;i++)
	{
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
		perror("ftok error");
		return -1;
	}
	if((shmid = shmget(key,shmsize,IPC_EXCL))==-1)
	{
		perror("shmget error");
		return -1;
	}
	transhm = (_tran *)shmat(shmid,NULL,0);
	if(transhm ==  (void *)-1)
	{
		perror("shmat error");
		return -1;
	}
	pos = hashfunc(inpid);
	for(i=0;i<BUCKETSCNT;i++)
	{
		if((transhm+pos+i)->innerid == innerid)
		{
			memset(transhm+pos+i,0,sizeof(_tran));
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
		printf("malloc keyvalue error\n");
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

	if(kvalue == NULL)
	{
		printf("key value not malloc \n");
		return -1;
	}
	if(loop==0)
	{
		hash = hashfunc(varname);
		printf("#####################[%d]#######\n",hash);
		tmpkvalue = kvalue+hash;

		if(tmpkvalue->varname[0]==' ')
		{
			printf("$$$$$$$$the first \n");
			strcpy(tmpkvalue->varname,varname);
			//tmpkvalue->value = (char *)malloc(len*sizeof(char));
			tmpkvalue->value = (char *)malloc(len);
			if(tmpkvalue->value == NULL)
			{
				printf("malloc value error\n");
				return -1;
			}
			//strncpy(tmpkvalue->value,value,len*sizeof(char));
			strncpy(tmpkvalue->value,value,len);
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
					memset(tmpkvalue->value,0,sizeof(tmpkvalue->value));
					//strncpy(tmpkvalue->value,value,len*sizeof(char));
					strncpy(tmpkvalue->value,value,len);
					return 0;
				}
				tmpkvalue = tmpkvalue->next;
				i++;
				printf("%%%%%%%%%%%%%%[%d]\n",i);
			}
			tmpkvalue = (_keyvalue *)malloc(sizeof(_keyvalue));
			if(tmpkvalue == NULL)
			{
				printf("malloc tmpkvalue error\n");
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
				printf("malloc value error\n");
				return -1;
			}
			//strncpy(tmpkvalue->value,value,len*sizeof(char));
			strncpy(tmpkvalue->value,value,len);
				printf("!!!!!!!!!!!!!!!!!!!!!!!!![%d]\n",i);
			return  0;
		}
	}else
	{
		memset(varnameloop,0,sizeof(varnameloop));
		sprintf(varnameloop,"%s[%d]",varname,loop);
		hash = hashfunc(varnameloop);
		printf("#####################[%d]#######\n",hash);
		tmpkvalue = kvalue+hash;
		if(tmpkvalue->varname[0]==' ')
		{
			printf("$$$$$$$$the first \n");
			strcpy(tmpkvalue->varname,varname);
			//tmpkvalue->value = (char *)malloc(len*sizeof(char));
			tmpkvalue->value = (char *)malloc(len);
			if(tmpkvalue->value == NULL)
			{
				printf("malloc value error\n");
				return -1;
			}
			//strncpy(tmpkvalue->value,value,len*sizeof(char));
			strncpy(tmpkvalue->value,value,len);
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
					memset(tmpkvalue->value,0,sizeof(tmpkvalue->value));
					//strncpy(tmpkvalue->value,value,len*sizeof(char));
					strncpy(tmpkvalue->value,value,len);
					return 0;
				}
				tmpkvalue = tmpkvalue->next;
				i++;
				printf("%%%%%%%%%%%%%%[%d]\n",i);
			}
			tmpkvalue = (_keyvalue *)malloc(sizeof(_keyvalue));
			if(tmpkvalue == NULL)
			{
				printf("malloc tmpkvalue error\n");
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
				printf("malloc value error\n");
				return -1;
			}
			//strncpy(tmpkvalue->value,value,len*sizeof(char));
			strncpy(tmpkvalue->value,value,len);
			printf("!!!!!!!!!!!!!!!!!!!!!!!!![%d]\n",i);
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
		printf("key value not malloc \n");
		return -1;
	}
	if(loop == 0)
	{
		hash = hashfunc(varname);
		printf("**********************[%d]******************\n",hash);
	}else
	{
		memset(varnameloop,0,sizeof(varnameloop));
		sprintf(varnameloop,"%s[%d]",varname,loop);
		hash = hashfunc(varnameloop);
		printf("**********************[%d]******************\n",hash);
	}
	tmpkvalue = kvalue+hash;
	while(tmpkvalue!=NULL)
	{
		printf("@@@@@[%s]\n",tmpkvalue->varname);
		if(!strcmp(tmpkvalue->varname,varname))
		{
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
		printf("start at [%d]\n",i);
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
	memset(inpid,0,sizeof(inpid));

	sprintf(inpid,"2%010ld",innerid);

	size_t shmsize = HASHCNT*BUCKETSCNT*sizeof(_tran);
	int shmid;

	if((key = ftok("/item/ups/etc/mq_1",10))==-1)
	{
		perror("ftok error");
		return -1;
	}
	if((shmid = shmget(key,shmsize,IPC_EXCL))==-1)
	{
		perror("shmget error");
		return -1;
	}
	transhm = (_tran *)shmat(shmid,NULL,0);
	if(transhm ==  (void *)-1)
	{
		perror("shmat error");
		return -1;
	}
	pos = hashfunc(inpid);
	for(i=0;i<BUCKETSCNT;i++)
	{
		if((transhm+pos+i)->innerid == innerid)
		{
			if(intran!=NULL)
			{
				strcpy((transhm+pos+i)->intran,intran);
			}
			if(outtran!=NULL)
			{
				strcpy((transhm+pos+i)->outtran,outtran);
			}
			(transhm+pos+i)->stat[0]='N';
			shmdt(transhm);
			return 0;
		}
	}
	printf("hash bucket full\n");
	shmdt(transhm);
}
