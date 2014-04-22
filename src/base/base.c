#include  "ups.h"
#include  "pool.h"
#include  <libxml/tree.h>
#include  <libxml/parser.h>

/** 调用函数动态库，执行函数 **/
int do_so(char *so_name,char *func_name,char *par1,char *par2,char *par3)
{
	if(so_name==NULL||func_name==NULL)
	{
		printf("invalid so name of func name\n");
		return -1;
	}
	void *handle = NULL;
	int (*func)(char *par1,char *par2,char *par3);
	handle = dlopen(so_name,RTLD_NOW);
	if(handle == NULL)
	{
		printf("open [%s] error[%s]\n",so_name,dlerror());
		return -1;
	}
	func = (int(*)(char *par1,char *par2,char *par3))dlsym(handle,func_name);
	if(func == NULL)
	{
		printf("find func [%s] error[%s]\n",so_name,dlerror());
		return -1;
	}
	if(func(par1,par2,par3)!=0)
	{
		printf("do the func error\n");
		return -1;
	}
	dlclose(handle);
	printf("执行函数[%s]成功\n",func_name);
	return 0;
}


int	getmsgid(char *msgname,int *msgidi,int *msgido)
{
	key_t key;
	char ftokpath[30];

	memset(ftokpath,0,sizeof(ftokpath));

	sprintf(ftokpath,"%s/%s","/item/ups/etc",msgname);
	/** 1 for in msg queue **/
	if((key=ftok(ftokpath,1))==-1)
	{
		printf("ftok error[%s]",strerror(errno));
		return -1;
	}
	if((*msgidi = msgget(key,IPC_EXCL))==-1)
	{
		perror("msgget error");
		return -1;
	}
	printf("msg get ok,msgid [%d]\n",*msgidi);

	/** 2 for out msg queue **/
	if((key=ftok(ftokpath,2))==-1)
	{
		printf("ftok error[%s]",strerror(errno));
		return -1;
	}
	if((*msgido = msgget(key,IPC_EXCL))==-1)
	{
		perror("msgget error");
		return -1;
	}
	printf("msg get ok,msgid [%d]\n",*msgido);
	return 0;
}
int getshm(int procid,size_t shmsize)
{
	int shmid;
	key_t   key;
	if((key = ftok("/item/ups/etc/mq_1",procid))==-1)
	{
		perror("ftok error");
		return -1;
	}
	if((shmid = shmget(key,shmsize,IPC_CREAT|IPC_EXCL|00666))==-1)
	{
		perror("shmget error");
		return -1;
	}
	printf("shmget ok[%d]\n",shmid);
	return 0;
}

/** init posix sem  **/
int initservregsem()
{
	pid_t ret = 0;
	int shmid = 0,i=0;
	_servreg *sreg = NULL;
	int shmsize = MAXSERVREG*sizeof(_servreg);
	if((shmid = getshmid(7,shmsize))==-1)
	{         
		printf("get serv shm id error\n");
		return -1;
	}
	printf("shmid is[%d]\n",shmid);
	if((sreg = shmat(shmid,NULL,0))==NULL) 
	{
		printf("shmat sreg error\n");
		return -1;
	}
	for(i=0;i<1024;i++)
	{
		sem_init(&(sreg+i)->sem1,1,1);
		sem_init(&(sreg+i)->sem2,1,1);
	}
	shmdt(sreg);
	return  0;
}

/** 获取XML节点全路径 **/
int getNodePath(char *path,xmlNodePtr cur)
{
	xmlNodePtr	curNode ;
	curNode = cur;
	char	tmppath[256];
	memset(tmppath,0,sizeof(tmppath));

	while(curNode!=NULL)
	{
		//printf("curNode parent path is [%s]\n",curNode->name);
		if(curNode->name!=NULL&&strcmp(curNode->name,"text"))
		{
			sprintf(path,"%s.%s",curNode->name,tmppath);
			strcpy(tmppath,path);
		}
		curNode = curNode->parent;
	}
	path[strlen(path)-1]='\0';
}

/** 设置错误代码函数 **/
int	seterr(char *errcode,char *errmsg)
{
	if(put_var_value("V_ERRCODE",10,1,errcode)==-1)
	{
		printf("set err code error\n");
		return -1;
	}
	if(put_var_value("V_ERRMSG",60,1,errmsg)==-1)
	{
		printf("set err msg error\n");
		return -1;
	}
	printf("set err ok\n");
	return 0;
}

/** 获取交易属性 **/
int gettranmap(_tranmap *tmap,char *trancode)
{
	if((tmap == NULL)||(trancode == NULL))
	{
		printf("传入参数错误\n");
		return -1;
	}
	_tranmap *ttmap = NULL;
	int shmid ;
	int shmsize = MAXTRANMAP*(sizeof(_tranmap));
	if((shmid = getshmid(5,shmsize))==-1)
	{         
		printf("get tranmap shm id error\n");
		return -1;
	}
	printf("shmid is[%d]\n",shmid);
	ttmap  = shmat(shmid,NULL,0);
	if(ttmap == (void *)-1)
	{
		printf("tranmap shmat error\n");
		return -1;
	}
	while(strcmp(ttmap->trancode,"END"))
	{
		if(!strcmp(ttmap->trancode,trancode))
		{
			//printf("find it\n");
			memcpy(tmap,ttmap,sizeof(_tranmap));
			shmdt(ttmap);
			return  0;
		}
		ttmap++;
	}
	return -1;
}
void trim (char *str)
{
	char    *p = str;
	char    *p1;
	if(p)
	{
		p1 = p+strlen(str)-1;
		while(*p&&isspace(*p))
		{
			p++;
		}
		while(p1>p&&isspace(*p1))
			*p1--='\0';
	}
	printf("!!![%s]\n",p);
	strcpy(str,p);
}
