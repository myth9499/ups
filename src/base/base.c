#include  "ups.h"
#include  "pool.h"
#include  <libxml/tree.h>
#include  <libxml/parser.h>

/** 调用函数动态库，执行函数 **/
int do_so(char *so_name,char *func_name,char *par1)
{
	if(so_name==NULL||func_name==NULL)
	{
		SysLog(1,"FILE [%s] LINE [%d]:调用动态库参数错误\n",__FILE__,__LINE__);
		return -1;
	}
	void *handle = NULL;
	int (*func)(char *par1);
	handle = dlopen(so_name,RTLD_LAZY);
	if(handle == NULL)
	{
		SysLog(1,"FILE [%s] LINE [%d]:打开动态库[%s]失败:%s\n",__FILE__,__LINE__,so_name,strerror(errno));
		return -1;
	}
	func = (int(*)(char *par1))dlsym(handle,func_name);
	if(func == NULL)
	{
		SysLog(1,"FILE [%s] LINE [%d]:打开函数[%s]失败:%s\n",__FILE__,__LINE__,func_name,strerror(errno));
		return -1;
	}
		SysLog(1,"@@@@@@@@@@@FILE [%s] LINE [%d]:执行函数[%s]参数:%s\n",__FILE__,__LINE__,func_name,par1);
	if(func(par1)==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:执行函数[%s]失败:%s\n",__FILE__,__LINE__,func_name,strerror(errno));
		return -1;
	}
	dlclose(handle);
	SysLog(1,"FILE [%s] LINE [%d]:执行函数[%s]成功\n",__FILE__,__LINE__,func_name);
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
		SysLog(1,"FILE [%s] LINE [%d]:获取主键[%s]失败:%s\n",__FILE__,__LINE__,ftokpath,strerror(errno));
		return -1;
	}
	if((*msgidi = msgget(key,IPC_EXCL))==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取消息队列[%s]失败:%s\n",__FILE__,__LINE__,msgname,strerror(errno));
		return -1;
	}
	/** 2 for out msg queue **/
	if((key=ftok(ftokpath,2))==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取主键[%s]失败:%s\n",__FILE__,__LINE__,ftokpath,strerror(errno));
		return -1;
	}
	if((*msgido = msgget(key,IPC_EXCL))==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取消息队列[%s]失败:%s\n",__FILE__,__LINE__,msgname,strerror(errno));
		return -1;
	}
	SysLog(1,"FILE [%s] LINE [%d]:获取消息队列[%s]成功:进入队列[%d]外出队列[%d]\n",__FILE__,__LINE__,msgname,*msgidi,*msgido);
	return 0;
}
int getshm(int procid,size_t shmsize)
{
	int shmid;
	key_t   key;
	if((key = ftok("/item/ups/etc/mq_1",procid))==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取主键失败:%s\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	if((shmid = shmget(key,shmsize,IPC_CREAT|IPC_EXCL|00666))==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取共享内存失败:%s\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	SysLog(1,"FILE [%s] LINE [%d]:获取共享内存成功:%d\n",__FILE__,__LINE__,shmid);
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
		SysLog(1,"FILE [%s] LINE [%d]:获取共享内存失败:%s\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	printf("shmid is[%d]\n",shmid);
	if((sreg = shmat(shmid,NULL,0))==NULL) 
	{
		SysLog(1,"FILE [%s] LINE [%d]:链接共享内存失败:%s\n",__FILE__,__LINE__,strerror(errno));
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
	int loop = 0 ;

	while(curNode!=NULL)
	{
		if(curNode->name!=NULL&&strcmp(curNode->name,"text"))
		{
			sprintf(path,"%s/%s",curNode->name,tmppath);
			strcpy(tmppath,path);
		}
		curNode = curNode->parent;
		loop++;
	}
	path[strlen(path)-1]='\0';
	return loop;
}

/** 设置错误代码函数 **/
int	seterr(char *errcode,char *errmsg)
{
	if(put_var_value("V_ERRCODE",10,1,errcode)==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:设置V_ERRCODE为:%s失败\n",__FILE__,__LINE__,errcode);
		return -1;
	}
	if(put_var_value("V_ERRMSG",60,1,errmsg)==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:设置V_ERRMSG为:%s失败\n",__FILE__,__LINE__,errcode);
		return -1;
	}
	return 0;
}

/** 获取交易属性 **/
int gettranmap(_tranmap *tmap,char *trancode)
{
	if((tmap == NULL)||(trancode == NULL))
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取交易码为[%s]的交易属性参数有误\n",__FILE__,__LINE__,trancode);
		return -1;
	}
	_tranmap *ttmap = NULL;
	int shmid ;
	int shmsize = MAXTRANMAP*(sizeof(_tranmap));
	if((shmid = getshmid(5,shmsize))==-1)
	{         
		SysLog(1,"FILE [%s] LINE [%d]:获取交易码为[%s]时获取共享内存失败\n",__FILE__,__LINE__,trancode);
		return -1;
	}
	printf("shmid is[%d]\n",shmid);
	ttmap  = shmat(shmid,NULL,0);
	if(ttmap == (void *)-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取交易码为[%s]时链接共享内存失败\n",__FILE__,__LINE__,trancode);
		return -1;
	}
	while(strcmp(ttmap->trancode,"END"))
	{
		if(!strcmp(ttmap->trancode,trancode))
		{
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
	strcpy(str,p);
}
