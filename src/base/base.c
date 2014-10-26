#include  "ups.h"
#include  "pool.h"
#include  <libxml/tree.h>
#include  <libxml/parser.h>

/** 获取当前内存占用 **/

void prtusage()
{
        FILE    *fp ;
        char    filepath[100];
        char    str[100];
        memset(filepath,0,sizeof(filepath));
        memset(str,0,sizeof(str));

        sprintf(filepath,"/proc/%ld/status",getpid());
        fp = fopen(filepath,"r");
        if(fp == NULL)
        {
                fprintf(stderr,"获取进程信息失败\n");
                return  ;
        }
        while(fgets(str,sizeof(str),fp)!=NULL)
        {
                if(memcmp(str,"VmRSS",5)==0)
                {
                        str[strlen(str)-1]='\0';
                        fprintf(stderr,"当前占用内存:[%s]\n",str+5);
                        break;
                }
                memset(str,0,sizeof(str));
        }
        fclose(fp);
        return ;
}

/** 调用函数动态库，执行函数 **/
int do_so(char *so_name,char *func_name,char *par1)
{
	if(so_name==NULL||func_name==NULL)
	{
		SysLog(1,"FILE [%s] LINE [%d]:调用动态库参数错误\n",__FILE__,__LINE__);
		return -1;
	}
	void *handle;
	int (*func)(char *par1);
	/**
	handle = dlopen(so_name,RTLD_NOLOAD);
	if(handle == NULL)
	{
	**/
	handle = dlopen(so_name,RTLD_LAZY);
	if(handle == NULL)
	{
		SysLog(1,"FILE [%s] LINE [%d]:打开动态库[%s]失败:%s\n",__FILE__,__LINE__,so_name,dlerror());
		return -1;
	}
	//}
	func = (int(*)(char *par1))dlsym(handle,func_name);
	if(func == NULL)
	{
		SysLog(1,"FILE [%s] LINE [%d]:打开函数[%s]失败:%s\n",__FILE__,__LINE__,func_name,dlerror());
		dlclose(handle);
		return -1;
	}
		SysLog(1,"@@@@@@@@@@@FILE [%s] LINE [%d]:执行函数[%s]参数:%s\n",__FILE__,__LINE__,func_name,par1);
	if(func(par1)==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:执行函数[%s]失败:%s\n",__FILE__,__LINE__,func_name,strerror(errno));
		dlclose(handle);
		return -1;
	}
	dlclose(handle);
	SysLog(1,"FILE [%s] LINE [%d]:执行函数[%s]成功\n",__FILE__,__LINE__,func_name);
	return 0;
}


int	getmsgid(char *msgname,int *msgidi,int *msgido,int	*msgidr)
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
	/** 3 for 应答核心 msg queue **/
	if((key=ftok(ftokpath,3))==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取主键[%s]失败:%s\n",__FILE__,__LINE__,ftokpath,strerror(errno));
		return -1;
	}
	if((*msgidr = msgget(key,IPC_EXCL))==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取消息队列[%s]失败:%s\n",__FILE__,__LINE__,msgname,strerror(errno));
		return -1;
	}
	SysLog(1,"FILE [%s] LINE [%d]:获取消息队列[%s]成功:进入队列[%d]外出队列[%d]应答核心队列[%d]\n",__FILE__,__LINE__,msgname,*msgidi,*msgido,*msgidr);
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
	_tran	*tran = NULL;

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
	for(i=0;i<MAXSERVREG;i++)
	{
		sem_init(&(sreg+i)->sem1,1,1);
		sem_init(&(sreg+i)->sem2,1,1);
	}
	shmdt(sreg);
	/** 初始化tran的sem **/
	shmsize = HASHCNT*BUCKETSCNT*sizeof(_tran);
	/** init tran shm **/
	if((shmid = getshmid(10,shmsize))==-1)
	{
		SysLog(1,"获取交易hash桶共享内存ID失败\n");
		return -1;
	}
	if((tran = shmat(shmid,NULL,0))==NULL) 
	{
		SysLog(1,"FILE [%s] LINE [%d]:链接共享内存失败:%s\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	for(i=0;i<HASHCNT*BUCKETSCNT;i++)
	{
		sem_init(&(tran+i)->sem1,1,1);
		sem_init(&(tran+i)->sem2,1,1);
	}
	shmdt(tran);
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
	if(put_var_value("V_ERRCODE",strlen(errcode)+1,1,errcode)==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:设置V_ERRCODE为:%s失败\n",__FILE__,__LINE__,errcode);
		return -1;
	}
	if(put_var_value("V_ERRMSG",strlen(errmsg)+1,1,errmsg)==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:设置V_ERRMSG为:%s失败\n",__FILE__,__LINE__,errcode);
		return -1;
	}
	return 0;
}

/** 获取交易属性 **/
int gettranmap(_tranmap *tmap,char *trancode)
{
	int iret =-1;
	int shmid ;
	int shmsize = MAXTRANMAP*(sizeof(_tranmap));

	if((tmap == NULL)||(trancode == NULL))
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取交易码为[%s]的交易属性参数有误\n",__FILE__,__LINE__,trancode);
		return -1;
	}
	_tranmap *ttmap,*tstmap = NULL;
	if((shmid = getshmid(5,shmsize))==-1)
	{         
		SysLog(1,"FILE [%s] LINE [%d]:获取交易码为[%s]时获取共享内存失败\n",__FILE__,__LINE__,trancode);
		return -1;
	}
	tstmap  = shmat(shmid,NULL,0);
	if(tstmap == (void *)-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取交易码为[%s]时链接共享内存失败\n",__FILE__,__LINE__,trancode);
		return -1;
	}
	ttmap = tstmap;
	while(strcmp(ttmap->trancode,"END"))
	{
		if(!strcmp(ttmap->trancode,trancode))
		{
			memcpy(tmap,ttmap,sizeof(_tranmap));
			iret = 0;
			break;
		}
		ttmap++;
	}
	shmdt(tstmap);
	return iret;
}
void Trim (char *str)
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

#define        ISSPACE(x)        ((x)==' '||(x)=='\r'||(x)=='\n'||(x)=='\f'||(x)=='\b'||(x)=='\t')

void trim( char *String )
{
        char        *Tail, *Head;

        for ( Tail = String + strlen( String ) - 1; Tail >= String; Tail -- )
                if ( !ISSPACE( *Tail ) )
                        break;
        Tail[1] = 0;
        for ( Head = String; Head <= Tail; Head ++ )
                if ( !ISSPACE( *Head ) )
                        break;
        if ( Head != String )
                memcpy( String, Head, ( Tail - Head + 2 ) * sizeof( char ) );
}
/** 获取可用服务 **/
pid_t getservpid(char *chnl_name)
{
	pid_t ret = 0;
	int err;
	int shmid = 0,i=0;
	_servreg *sreg = NULL;
	int	servpos=0;//serv 偏移
	int shmsize = MAXSERVREG*sizeof(_servreg);
	if((shmid = getshmid(7,shmsize))==-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取服务登记表失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	if((sreg = shmat(shmid,NULL,0))==NULL)
	{
		SysLog(1,"FILE [%s] LINE [%d]:连接服务登记表失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	/** 信号量控制 **/
	/** 随机数控制 **/
	servpos = MAXSERVREG/2;
	for(i=0;i<MAXSERVREG;i++)
	{
		//SysLog(1,"#####FILE[%s] LINE[%d]@@@@pid  [%ld] 尝试[%d]个\n",__FILE__,__LINE__,getpid(),i);
		err=sem_trywait(&((sreg+i)->sem1));
		if(err!=0&&errno==EAGAIN)
		{
			SysLog(1,"FILE[%s] LINE[%d]pid[%ld]当前正在占用，尝试下一个[%d]\n",__FILE__,__LINE__,getpid(),i);
			// 为了最大限度保证每次查询都可以查询的到，尝试得不到的时候直接加一半再找 
			//i+=servpos;
			continue;
			//servpos = servpos/2;
		}else if(err == 0)
		{
			if((sreg+i)->stat[0]=='N'&&!strcmp((sreg+i)->chnlname,chnl_name)&&!strcmp((sreg+i)->type,"S"))
			{
				SysLog(1,"FILE[%s]LINE[%d]开始修改服务[%ld]状态\n",__FILE__,__LINE__,(sreg+i)->servpid);
				(sreg+i)->stat[0]='L';
				ret = (sreg+i)->servpid ;
				sem_post(&((sreg+i)->sem1));
				SysLog(1,"FILE[%s]LINE[%d]结束修改服务[%ld]状态\n",__FILE__,__LINE__,(sreg+i)->servpid);
				break;
			}else
			{
				sem_post(&((sreg+i)->sem1));
			}
		}else
		{
			SysLog(1,"FILE[%s]LINE[%d]加锁进程[%ld]状态失败:%s\n",__FILE__,__LINE__,(sreg+i)->servpid,strerror(errno));
			break;
		}
	}
	shmdt(sreg);
	return ret;
}
int insert_chnlreg(char	*startcmd,char *chnlname )
{
	int shmid = 0,i=0;
	_servreg *sreg = NULL;
	int shmsize = MAXSERVREG*sizeof(_servreg);
	if((shmid = getshmid(7,shmsize))==-1)
	{
		SysLog(1,"get serv shm id error\n");
		return -1;
	}
	SysLog(1,"shmid is[%d]\n",shmid);
	if((sreg = shmat(shmid,NULL,0))==NULL)
	{
		SysLog(1,"shmat sreg error\n");
		return -1;
	}
	for(i=0;i<MAXSERVREG;i++)
	{
		//SysLog(1,"i[[[[]]]]]%d servpid [%d]\n",i,(sreg+i)->servpid);
		sem_wait(&((sreg+i)->sem2));
		if((sreg+i)->servpid==0)
		{
			(sreg+i)->servpid = getpid();
			strcpy((sreg+i)->chnlname,chnlname);
			strcpy((sreg+i)->startcmd,startcmd);
			(sreg+i)->stat[0]='N';
			(sreg+i)->type[0]='C';
			sem_post(&((sreg+i)->sem2));
			shmdt(sreg);
			return 0;
		}else if((kill((sreg+i)->servpid,SIGUSR1)==-1)&&(errno == ESRCH))
		{
			(sreg+i)->servpid = getpid();
			strcpy((sreg+i)->chnlname,chnlname);
			strcpy((sreg+i)->startcmd,startcmd);
			(sreg+i)->stat[0]='N';
			sem_post(&((sreg+i)->sem2));
			shmdt(sreg);
			return 0;
		}
		sem_post(&((sreg+i)->sem2));
	}
	shmdt(sreg);
	return -1;
}


int updatestat_foroth(pid_t	pid)
{
	int ret = 0;
	int shmid = 0,i=0,semid = 0;
	_servreg *sreg = NULL;
	int shmsize = MAXSERVREG*sizeof(_servreg);
	if((shmid = getshmid(7,shmsize))==-1)
	{
		SysLog(1,"get serv shm id error\n");
		return -1;
	}
	SysLog(1,"shmid is[%d]\n",shmid);
	if((sreg = shmat(shmid,NULL,0))==NULL)
	{
		SysLog(1,"shmat sreg error\n");
		return -1;
	}
	for(i=0;i<MAXSERVREG;i++)
	{
		//SysLog(1,"i[[[[]]]]]%d servpid [%d][%c]\n",i,(sreg+i)->servpid,(sreg+i)->stat[0]);
		if((sreg+i)->servpid==pid)
		{
			sem_wait(&((sreg+i)->sem1));
			(sreg+i)->stat[0]='N';
			ret = 0;
			sem_post(&((sreg+i)->sem1));
			break;
		}
		//sem_post(&((sreg+i)->sem1));

	}
	shmdt(sreg);
	SysLog(1,"解除信号量成功\n");
	return ret;
}
/** 获取交易属性 **/
int get_vardef(char	*varname,_vardef	*vardef)
{
	int iret =-1;
	int shmid ;
	int shmsize = MAXVARDEF*(sizeof(_vardef));

	if((varname == NULL)||(vardef == NULL))
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取变量为[%s]的配置失败\n",__FILE__,__LINE__,varname);
		return -1;
	}
	_vardef *tvardef,*tstvardef = NULL;
	if((shmid = getshmid(4,shmsize))==-1)
	{         
		SysLog(1,"FILE [%s] LINE [%d]:获取变量为[%s]时获取共享内存失败\n",__FILE__,__LINE__,varname);
		return -1;
	}
	tstvardef  = shmat(shmid,NULL,0);
	if(tstvardef == (void *)-1)
	{
		SysLog(1,"FILE [%s] LINE [%d]:获取变量为[%s]时链接共享内存失败\n",__FILE__,__LINE__,varname);
		return -1;
	}
	tvardef = tstvardef;
	while(strcmp(tvardef->varname,"END"))
	{
		if(!strcmp(tvardef->varname,varname))
		{
			memcpy(vardef,tvardef,sizeof(_vardef));
			iret = 0;
			break;
		}
		tvardef++;
	}
	shmdt(tstvardef);
	return iret;
}
