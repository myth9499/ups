/** the public include file **/
#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<string.h>
#include<unistd.h>
#include<time.h>
#include<signal.h>
#include<assert.h>
#include<pthread.h>
#include<errno.h>
#include<signal.h>
#include<dlfcn.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <fcntl.h>
#include <semaphore.h>
#include <bits/pthreadtypes.h>

/** system define errno **/
#define SYS_ERR -1
#define SYS_OK  0

/** func define **/

static int BUCKETSCNT=100; /** HASH最大值 **/
static int HASHCNT=13;	   /** HASH桶最大值 **/
static int VARCNT=1024;    /** 最大支持变量个数**/
static int MAXCOMMMSG=1024;/** 通信区最大支持传输报文个数**/
static int MAXFLOW=1024;   /** 最大支持的流程个数 **/
static int MAXSERVREG=1024;/** 最大支持的服务个数 **/
static int MAXXMLCFG=1024; /** 最大支持的XML报文配置个数**/
static int MAXTRANMAP=100; /** 最大支持的内外部交易映射个数 **/

/**main tran buf **/
typedef struct TRAN
{
	long	innerid;	/** 系统跟踪号 **/
	char	intran[1048576];/** 各渠道传输到核心的报文**/
	char	outtran[1048576];/** 服务传输或者返回到渠道的报文**/
	time_t	stime;			/** 开始执行时间 **/
	time_t	etime;			/** 结束执行时间 **/
	char	stat[2];		/** 当前服务状态 **/
	sem_t	sem1;			/** 信号量  **/
	sem_t	sem2;			/** 信号量 **/
}_tran;

/** main msg buf 
 * 消息队列传输消息，进行精简，只传输渠道名称+交易代码+报文长度
 **/
typedef struct MSGTRANBUF
{
	char	chnlname[20];	/** 渠道名称 **/
	char	trancode[10];	/** 交易码 **/
	long	buffsize;		/** 报文长度 **/
}_msgtranbuf;

typedef struct MSGBUF
{
	long	innerid;	/** 系统跟踪号 **/
	/**char	tranbuf[42];  chnlname(20)+Trancode(10)buffsize(10)+**/
	_msgtranbuf	tranbuf; /** chnlname(20)+Trancode(10)buffsize(10)+**/
}_msgbuf;

/** key-value hash **/
typedef struct KEYVALUE
{
	struct KEYVALUE *head;/** 链表头指针**/
	char varname[20];
	void *value;		  /** 设置为void指针，可以存放全部值 **/
	struct KEYVALUE *pre; /** 链表前指针 **/
	struct KEYVALUE *next; /** 链表后指针 **/
	struct KEYVALUE *end;  /** 链表尾指针 **/
}_keyvalue;

/** inner comm msg type **/
typedef struct COMMMSG
{
	char	commname[100];	/** 内部解包匹配渠道名称 **/
	int	len;				/** 内容长度 **/
	char	commvar[10];	/** 放置变量名 **/
	char	commmark[1024]; /** 备注 **/
}_commmsg;

/** server flow cfg **/
typedef struct FLOW
{
	char	flowname[60];	/** 流程名称 **/
	char	flowmark[256];	/** 流程备注 **/
	char	flowso[20];		/** 调用动态库名称 **/
	char	flowfunc[40];	/** 调用函数名称 **/
	char	funcpar1[1024];	/** 传入函数参数 **/
	char	errflow[60];	/** 错误跳转流程 **/
}_flow;

/** 核心服务登记表 **/
typedef struct SERVREG
{
	pid_t	servpid;	/** 核心服务进程号 **/
	char	stat[2];	/** 核心服务状态  **/
	char	chnlname[20];/** 核心/渠道名称 **/
	char	type[2];//S代表服务,C代表渠道
	sem_t	sem1;	/** 信号量 **/
	sem_t	sem2;	/** 信号量 **/
}_servreg;


/** XML格式配置 **/
typedef struct XMLCFG
{
	char	xmlname[20];	/** XML格式名称 **/
	char	mark[60];		/** XML说明 **/
	char	fullpath[128];	/** XML配置全路径 **/
	char	varname[10];	/** XML解包变量 **/
	char	isneed[2];		/** 是否必须 **/
	char	sign[2];		/** 是否签名 **/
	char	loop[2];		/** 是否循环 **/
	int		depth;			/** 深度 **/
}_xmlcfg;

/** 信号量 **/
union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

/** 交易码映射表 **/
typedef struct TRANMAP
{
	char	trancode[10];	/** 交易码 **/
	char	tranname[60];	/** 交易名称 **/
	char	tranflow[60];	/** 对应流程名称 **/
	int		timeout;		/** 交易超时时间 **/
}_tranmap;
static _keyvalue *kvalue = NULL;
long	innerid;//跟踪号，全局使用
/** hash shm buckets **/
int shm_hash_insert(long ,char *,char *);
int  get_shm_hash(long,_tran *);
