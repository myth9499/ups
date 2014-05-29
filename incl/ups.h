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

static int BUCKETSCNT=100;
static int HASHCNT=13;
static int VARCNT=1024;
static int MAXCOMMMSG=1024;
static int MAXFLOW=1024;
static int MAXSERVREG=1024;
static int MAXXMLCFG=1024;
static int MAXTRANMAP=100;

/**main tran buf **/
typedef struct TRAN
{
	long	innerid;
	char	intran[1048576];
	char	outtran[1048576];
	time_t	stime;
	time_t	etime;
	char	stat[2];
	sem_t	sem1;
	sem_t	sem2;
}_tran;

/** main msg buf 
 * 消息队列传输消息，进行精简，只传输渠道名称+交易代码+报文长度
 **/
typedef struct MSGTRANBUF
{
	char	chnlname[20];
	char	trancode[10];
	long	buffsize;
}_msgtranbuf;

typedef struct MSGBUF
{
	long	innerid;
	/**char	tranbuf[42];  chnlname(20)+Trancode(10)buffsize(10)+**/
	_msgtranbuf	tranbuf; /** chnlname(20)+Trancode(10)buffsize(10)+**/
}_msgbuf;

/** key-value hash **/
typedef struct KEYVALUE
{
	struct KEYVALUE *pre;
	char varname[20];
	char *value;
	struct KEYVALUE *next;
	struct KEYVALUE *end;
}_keyvalue;

/** inner comm msg type **/
typedef struct COMMMSG
{
	char	commname[100];
	int	len;
	char	commvar[10];
	char	commmark[1024];
}_commmsg;

/** server flow cfg **/
typedef struct FLOW
{
	char	flowname[60];
	char	flowmark[256];
	char	flowso[20];
	char	flowfunc[40];
	char	funcpar1[1024];
	char	errflow[60];
}_flow;

/** 核心服务登记表 **/
typedef struct SERVREG
{
	pid_t	servpid;
	char	stat[2];
	char	chnlname[20];
	char	type[2];//S代表服务,C代表渠道
	sem_t	sem1;
	sem_t	sem2;
}_servreg;


/** XML格式配置 **/
typedef struct XMLCFG
{
	char	xmlname[20];
	char	mark[60];
	char	fullpath[128];
	char	varname[10];
	char	isneed[2];
	char	sign[2];
	char	loop[2];
	int		depth;
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
	char	trancode[10];
	char	tranname[60];
	char	tranflow[60];
	int		timeout;
}_tranmap;
static _keyvalue *kvalue = NULL;
long	innerid;//跟踪号，全局使用
/** hash shm buckets **/
int shm_hash_insert(long ,char *,char *);
int  get_shm_hash(long,_tran *);
