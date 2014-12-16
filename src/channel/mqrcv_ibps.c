#include "ups.h"
/* includes for MQI  */
#include <cmqc.h>

int msgidi=0,msgido=0,msgidr=0;
_msgbuf *mbuf=NULL;
_tran *tranbuf=NULL;
char	chnlname[50];
long i=1L;
char     QMName[50];             /* queue manager name            */
char	LQName[60];
char	trancode[30];
char	waitsec[10];

static long testid=1;
int	getchnlcfg(char *chnlname)
{
	FILE *fp;
	char	tmpbuf[60];
	memset(tmpbuf,0,sizeof(tmpbuf));
	char	cfgpath[100];
	memset(cfgpath,0,sizeof(cfgpath));

	if(chnlname == NULL)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE[%d] 传入渠道名称为空\n",__FILE__,__LINE__);
		return -1;
	}
	/** 初始化系统所有渠道的队列区 **/
	sprintf(cfgpath,"%s%s",upshome,"/src/cfg/chnl.cfg");
	fp = fopen(cfgpath,"r");
	if(fp == NULL)
	{
		SysLog(LOG_SYS_ERR,"打开渠道初始化配置文件失败:[%s]\n",strerror(errno));
		return -1;
	}
	while(fgets(tmpbuf,sizeof(tmpbuf),fp)!=NULL)
	{
		tmpbuf[strlen(tmpbuf)-1]='\0';
		if(tmpbuf[0]=='*')
			continue;
		else if(tmpbuf[0]=='@'&&!strcmp(tmpbuf+1,chnlname))
		{
			while(fgets(tmpbuf,sizeof(tmpbuf),fp)!=NULL)
			{
				tmpbuf[strlen(tmpbuf)-1]='\0';
				if(!memcmp(tmpbuf,"队列管理器名称",14))
				{
					strcpy(QMName,strstr(tmpbuf,":")+1);
				}
				if(!memcmp(tmpbuf,"本地队列名称",12))
				{
					strcpy(LQName,strstr(tmpbuf,":")+1);
				}
				if(!memcmp(tmpbuf,"交易名称",8))
				{
					strcpy(trancode,strstr(tmpbuf,":")+1);
					if(trancode[0]=='B')
					{
						SysLog(LOG_SYS_ERR,"静态获取交易代码[%s]\n",trancode+1);
						//strcpy(trancode,trancode+1);
					}
				}
				if(!memcmp(tmpbuf,"无交易等待间隔",14))
				{
					strcpy(waitsec,strstr(tmpbuf,":")+1);
				}
				if(tmpbuf[0]=='@')
					return 0;
			}
		}else
		{
			continue;
		}
	}
}


int	unpack_head_file(char *buffer,char *msgtype,char *xmlfile)
{
	FILE *fp;
	char	msgid[21];
	memset(msgid,0,sizeof(msgid));
	if(buffer == NULL||strlen(buffer)==0)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:传入参数有误\n",__FILE__,__LINE__);
		return -1;
	}
	//printf("buffer is [%s]\n",buffer);
	memcpy(msgtype,buffer+54,20);
	memcpy(msgid,buffer+74,20);

	trim(msgtype);
	trim(msgid);
	sprintf(xmlfile,"%s%s/%s",upshome,"/msg",msgid);
	fp = fopen(xmlfile,"w");
	if(fp == NULL)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:打开文件[%s]失败[%s]\n",__FILE__,__LINE__,xmlfile,strerror(errno));
		return -1;
	}
	if(fwrite(buffer+128,strlen(buffer)-128,1,fp)==-1)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:写文件[%s]失败[%s]\n",__FILE__,__LINE__,xmlfile,strerror(errno));
		fclose(fp);
		return -1;
	}
	fclose(fp);
	return  0;
}

/** 主进程注册信号，当子进程退出时进行后续处理
 * 防止僵尸进程
 **/
void child_exit(int signal)
{
	pid_t   pid;
	int     stat;
	while((pid = waitpid(-1,&stat,WNOHANG))>0)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:子进程[%d]退出，退出状态[%d]\n",__FILE__,__LINE__,pid,stat);
	}
	return ;
}

int chnlprocess(char *buffer)
{
	int ipid = 0,iret =0;
	char rcvbuf[4096],rbuf[4096];
	char tranid[5];
	char tranvalue[4096];
	char errmsg[1024];
	char rtmsg [1024];

	memset(errmsg,0,sizeof(errmsg));
	memset(rtmsg,0,sizeof(rtmsg));
	memset(rcvbuf,0,sizeof(rcvbuf));
	memset(rbuf,0,sizeof(rbuf));

	/** 注册超时信号 **/
	//signal(SIGALRM,timeout);
	//alarm(30);

	/**填充消息队列数据 **/
	/** 利用随机数产生唯一的交易跟踪号 **/
	srand((unsigned)time(NULL));
	//mbuf->innerid =  (long)getpid()+rand()%1000000+rand()%3333333;
	mbuf->innerid=getinnerid();
	testid++;
	strcpy(mbuf->tranbuf.chnlname,chnlname);
	strcpy(mbuf->tranbuf.trancode,trancode+1);
	mbuf->tranbuf.buffsize = strlen(buffer);

	SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:全系统跟踪号为[%ld]\n",__FILE__,__LINE__,mbuf->innerid);
	i++;
	if(shm_hash_insert(mbuf->innerid,buffer,NULL)==-1)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:放置交易报文信息到共享内存hash表中失败\n",__FILE__,__LINE__);
		return -1;
	}
	SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:放置交易报文信息到共享内存hash表中成功,跟踪号：[%ld],报文长度[%d]\n",__FILE__,__LINE__,mbuf->innerid,strlen(rcvbuf));
	if((ipid = getservpid(chnlname))<=0)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:暂无可用服务\n",__FILE__,__LINE__);
		updatestat_foroth(ipid);
		/** 删除消息队列信息，防止堵塞 **/
		if(delete_shm_hash(mbuf->innerid)==-1)
		{
			SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:删除共享内存hash表数据失败\n",__FILE__,__LINE__);
		}
		if(msgrcv(msgidi,mbuf,sizeof(mbuf->tranbuf),mbuf->innerid,IPC_NOWAIT)==-1)
		{
			SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:删除消息队列数据失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		}
		return -1;
	}
	iret = msgsnd(msgido,mbuf,sizeof(mbuf->tranbuf),IPC_NOWAIT);
	if(iret == -1)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:发送控制信息到消息队列失败[%s]\n",__FILE__,__LINE__,strerror(errno));
		updatestat_foroth(ipid);
		if(delete_shm_hash(mbuf->innerid)==-1)
		{
			SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:删除共享内存hash表数据失败\n",__FILE__,__LINE__);
		}
		return -1;
	}
	SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:获取可用服务并发送控制信号\n",__FILE__,__LINE__);
	/** 发送信号到核心服务 **/
	SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:准备发送到的服务进程为 [%ld]\n",__FILE__,__LINE__,ipid);
	if(kill(ipid,SIGUSR2)==0)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:发送控制信号到服务 [%ld]成功\n",__FILE__,__LINE__,ipid);
	}else
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:发送控制信号到服务 [%ld]失败：[%s]\n",__FILE__,__LINE__,ipid,strerror(errno));
		updatestat_foroth(ipid);
		/**还需要删除消息队列信息，防止堵塞 **/
		if(delete_shm_hash(mbuf->innerid)==-1)
		{
			SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:删除共享内存hash表数据失败\n",__FILE__,__LINE__);
		}
		if(msgrcv(msgidi,mbuf,sizeof(mbuf->tranbuf),mbuf->innerid,0)==-1)
		{
			SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:删除消息队列数据失败 ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		}
		return -1;
	}
	return  0;
}


int main(int argc, char **argv)
{

	if(argc<2)
	{
		printf("USAGE:appname+chnlname\n");
		return -1;
	}
	/** 初始化全局共享内存前，先获取ups根路径 **/
	if(setupshome()==-1)
	{
		printf("设置全局变量upshome错误,请检查UPSHOME环境变量是否设置\n");
		return -1;
	}


	char    startcmd[200];
	int     i=0;
	memset(startcmd,0x00,sizeof(startcmd));

	/*   Declare MQI structures needed                                */
	MQOD     od = {MQOD_DEFAULT};    /* Object Descriptor             */
	MQMD     md = {MQMD_DEFAULT};    /* Message Descriptor            */
	MQGMO   gmo = {MQGMO_DEFAULT};   /* get message options           */
	/** note, sample uses defaults where it can **/

	MQHCONN  Hcon;                   /* connection handle             */
	MQHOBJ   Hobj;                   /* object handle                 */
	MQLONG   O_options;              /* MQOPEN options                */
	MQLONG   C_options;              /* MQCLOSE options               */
	MQLONG   CompCode;               /* completion code               */
	MQLONG   OpenCode;               /* MQOPEN completion code        */
	MQLONG   Reason;                 /* reason code                   */
	MQLONG   CReason;                /* reason code for MQCONN        */
	MQBYTE   buffer[65536];          /* message buffer                */
	MQLONG   buflen;                 /* buffer length                 */
	MQLONG   messlen;                /* message length received       */




	char	xmlfile[60];
	char	sendbuf[90];
	char	msgtype[90];
	char	headbuf[1024];

	memset(QMName,0,sizeof(QMName));
	memset(LQName,0,sizeof(LQName));
	memset(trancode,0,sizeof(trancode));
	memset(chnlname,0,sizeof(chnlname));
	memset(xmlfile,0,sizeof(xmlfile));
	memset(sendbuf,0,sizeof(sendbuf));
	memset(msgtype,0,sizeof(msgtype));

	strcpy(chnlname,argv[1]);
	if(getchnlcfg(chnlname)!=0)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d] 获取渠道[%s]配置失败\n",__FILE__,__LINE__,chnlname);
		return -1;
	}

	mbuf = (_msgbuf *)malloc(sizeof(_msgbuf));
	if(mbuf == (void *)-1)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:MALLOC MSGBUF ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}

	tranbuf = (_tran *)malloc(sizeof(_tran));
	if(tranbuf == (void *)-1)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:MALLOC TRANBUF ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		free(mbuf);
		return -1;
	}

	if(getmsgid(chnlname,&msgidi,&msgido,&msgidr)==-1)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:GET CHANNEL[%s] MSGID ERROR[%s]\n",__FILE__,__LINE__,chnlname,strerror(errno));
		free(mbuf);
		free(tranbuf);
		return -1;
	}

	/** 设置忽略SIGPIPE信号，防止因socket写的时候客户端关闭导致的SIGPIPE信号 **/
	signal(SIGPIPE,SIG_IGN);
	signal(SIGCHLD,child_exit);

	/******************************************************************/
	/*                                                                */
	/*   Create object descriptor for subject queue                   */
	/*                                                                */
	/******************************************************************/
	/**
	  strncpy(od.ObjectName, argv[1], MQ_Q_NAME_LENGTH);
	  QMName[0] = 0;    default 
	  if (argc > 2)
	  strncpy(QMName, argv[2], MQ_Q_MGR_NAME_LENGTH);
	 **/

	/******************************************************************/
	/*                                                                */
	/*   Connect to queue manager                                     */
	/*                                                                */
	/******************************************************************/
	MQCONN(QMName,                  /* queue manager                  */
			&Hcon,                   /* connection handle              */
			&CompCode,               /* completion code                */
			&CReason);               /* reason code                    */

	/* report reason and stop if it failed     */
	if (CompCode == MQCC_FAILED)
	{
		SysLog(LOG_SYS_ERR,"MQCONN ended with reason code %d\n", CReason);
		free(mbuf);
		free(tranbuf);
		exit( (int)CReason );
	}

	/******************************************************************/
	/*                                                                */
	/*   Open the named message queue for input; exclusive or shared  */
	/*   use of the queue is controlled by the queue definition here  */
	/*                                                                */
	/******************************************************************/

	if (argc > 3)
	{
		O_options = atoi( argv[3] );
		SysLog(LOG_SYS_ERR,"open  options are %d\n", O_options);
	}
	else
	{
		O_options = MQOO_INPUT_AS_Q_DEF    /* open queue for input      */
			| MQOO_FAIL_IF_QUIESCING /* but not if MQM stopping   */
			;                        /* = 0x2001 = 8193 decimal   */
	}

	/** the local queue name **/
	//strcpy(od.ObjectName,"TEST");
	strcpy(od.ObjectName,LQName);

	MQOPEN(Hcon,                      /* connection handle            */
			&od,                       /* object descriptor for queue  */
			O_options,                 /* open options                 */
			&Hobj,                     /* object handle                */
			&OpenCode,                 /* completion code              */
			&Reason);                  /* reason code                  */

	/* report reason, if any; stop if failed      */
	if (Reason != MQRC_NONE)
	{
		SysLog(LOG_SYS_ERR,"MQOPEN ended with reason code %d\n", Reason);
	}

	if (OpenCode == MQCC_FAILED)
	{
		SysLog(LOG_SYS_ERR,"unable to open queue for input\n");
		return -1;
	}

	/******************************************************************/
	/*                                                                */
	/*   Get messages from the message queue                          */
	/*   Loop until there is a failure                                */
	/*                                                                */
	/******************************************************************/
	CompCode = OpenCode;       /* use MQOPEN result for initial test  */

	/******************************************************************/
	/* Use these options when connecting to Queue Managers that also  */
	/* support them, see the Application Programming Reference for    */
	/* details.                                                       */
	/* These options cause the MsgId and CorrelId to be replaced, so  */
	/* that there is no need to reset them before each MQGET          */
	/******************************************************************/
	/*gmo.Version = MQGMO_VERSION_2;*/ /* Avoid need to reset Message */
	/*gmo.MatchOptions = MQMO_NONE; */ /* ID and Correlation ID after */
	/* every MQGET                 */
	gmo.Options = MQGMO_WAIT           /* wait for new messages       */
		| MQGMO_NO_SYNCPOINT   /* no transaction              */
		| MQGMO_CONVERT;       /* convert if necessary        */
	//gmo.WaitInterval = MQWI_UNLIMITED;          /* 15 second limit for waiting */
	gmo.WaitInterval = 2000;          /* 15 second limit for waiting */
	gmo.MatchOptions=MQMO_NONE;
	gmo.Version=MQGMO_VERSION_2;

	signal(SIGUSR1,SIG_IGN);
	for(i=0;i<argc;i++)
	{
		if(strlen(argv[i])==0)
			break;
		strcat(startcmd," ");
		strcat(startcmd,argv[i]);
	}

	strcat(startcmd," ");
	strcat(startcmd,"&");
	if(insert_chnlreg(startcmd,chnlname)!=0)
	{
		SysLog(LOG_SYS_ERR,"FILE [%s] LINE [%d]:添加渠道到监控内存失败\n",__FILE__,__LINE__);
		return -1;
	}

	//while (CompCode != MQCC_FAILED)
	while (1)
	{
		buflen = sizeof(buffer) - 1; /* buffer size available for GET   */

		/****************************************************************/
		/* The following two statements are not required if the MQGMO   */
		/* version is set to MQGMO_VERSION_2 and and gmo.MatchOptions   */
		/* is set to MQGMO_NONE                                         */
		/****************************************************************/
		/*                                                              */
		/*   In order to read the messages in sequence, MsgId and       */
		/*   CorrelID must have the default value.  MQGET sets them     */
		/*   to the values in for message it returns, so re-initialise  */
		/*   them before every call                                     */
		/*                                                              */
		/****************************************************************/
		memcpy(md.MsgId, MQMI_NONE, sizeof(md.MsgId));
		memcpy(md.CorrelId, MQCI_NONE, sizeof(md.CorrelId));

		/****************************************************************/
		/*                                                              */
		/*   MQGET sets Encoding and CodedCharSetId to the values in    */
		/*   the message returned, so these fields should be reset to   */
		/*   the default values before every call, as MQGMO_CONVERT is  */
		/*   specified.                                                 */
		/*                                                              */
		/****************************************************************/

		md.Encoding       = MQENC_NATIVE;
		md.CodedCharSetId = MQCCSI_Q_MGR;

		MQGET(Hcon,                /* connection handle                 */
				Hobj,                /* object handle                     */
				&md,                 /* message descriptor                */
				&gmo,                /* get message options               */
				buflen,              /* buffer length                     */
				buffer,              /* message buffer                    */
				&messlen,            /* message length                    */
				&CompCode,           /* completion code                   */
				&Reason);            /* reason code                       */

		/* report reason, if any     */
		if (Reason != MQRC_NONE)
		{
			if (Reason == MQRC_NO_MSG_AVAILABLE)
			{                         /* special report for normal end    */
				SysLog(LOG_SYS_ERR,"no more messages\n");
				sleep(atoi(waitsec));
				continue;
			}
			else                      /* general report for other reasons */
			{
				SysLog(LOG_SYS_ERR,"MQGET ended with reason code %d\n", Reason);

				/*   treat truncated message as a failure for this sample   */
				if (Reason == MQRC_TRUNCATED_MSG_FAILED)
				{
					CompCode = MQCC_FAILED;
				}
				return -1;
			}
		}

		/****************************************************************/
		/*   Display each message received                              */
		/****************************************************************/
		if (CompCode != MQCC_FAILED)
		{
			buffer[messlen] = '\0';            /* add terminator          */
			SysLog(LOG_SYS_ERR,"message <%s>\n", buffer);
			memset(xmlfile,0,sizeof(xmlfile));
			memset(msgtype,0,sizeof(msgtype));
			/** 解报文头并生成文件 **/
			if(unpack_head_file(buffer,msgtype,xmlfile)!=0)
			{
				SysLog(LOG_SYS_ERR,"解报文头生成文件失败\n");
			}else
			{
				memset(sendbuf,0,sizeof(sendbuf));
				memset(headbuf,0,sizeof(headbuf));
				memcpy(headbuf,buffer,128);
				sprintf(sendbuf,"%s|%s|%s",msgtype,xmlfile,headbuf);
				SysLog(LOG_SYS_ERR,"传入hash表参数[%s]\n",sendbuf);
				if(chnlprocess(sendbuf)==0)
				{
					SysLog(LOG_SYS_ERR,"渠道处理成功\n");
				}else
				{
					SysLog(LOG_SYS_ERR,"渠道处理失败\n");
				}
			}

		}
	}

	/******************************************************************/
	/*                                                                */
	/*   Close the source queue (if it was opened)                    */
	/*                                                                */
	/******************************************************************/
	if (OpenCode != MQCC_FAILED)
	{
		if (argc > 4)
		{
			C_options = atoi( argv[4] );
			SysLog(LOG_SYS_ERR,"close options are %d\n", C_options);
		}
		else
		{
			C_options = MQCO_NONE;        /* no close options             */
		}

		MQCLOSE(Hcon,                    /* connection handle           */
				&Hobj,                   /* object handle               */
				C_options,
				&CompCode,               /* completion code             */
				&Reason);                /* reason code                 */

		/* report reason, if any     */
		if (Reason != MQRC_NONE)
		{
			SysLog(LOG_SYS_ERR,"MQCLOSE ended with reason code %d\n", Reason);
		}
	}

	/******************************************************************/
	/*                                                                */
	/*   Disconnect from MQM if not already connected                 */
	/*                                                                */
	/******************************************************************/
	if (CReason != MQRC_ALREADY_CONNECTED )
	{
		MQDISC(&Hcon,                     /* connection handle          */
				&CompCode,                 /* completion code            */
				&Reason);                  /* reason code                */

		/* report reason, if any     */
		if (Reason != MQRC_NONE)
		{
			SysLog(LOG_SYS_ERR,"MQDISC ended with reason code %d\n", Reason);
		}
	}

	/******************************************************************/
	/*                                                                */
	/* END OF AMQSGET0                                                */
	/*                                                                */
	/******************************************************************/
	SysLog(LOG_SYS_ERR,"Sample AMQSGET0 end\n");
	free(mbuf);
	free(tranbuf);
	return(0);
}
