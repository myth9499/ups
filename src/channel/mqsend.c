#include "ups.h"
/* includes for MQI */
#include <cmqc.h>



int msgidi=0,msgido=0,msgidr;
_msgbuf *mbuf=NULL;
_tran *tranbuf=NULL;
char    chnlname[50];
char     buffer[65535];          /* message buffer                */
char     tmpbuffer[65000];          /* message buffer                */
char     headbuf[1024];          /* message buffer                */
char     QMName[50];             /* queue manager name            */
char     RQName[50];             /* remote queue name            */

/** 主进程注册信号，当子进程退出时进行后续处理
 *  * 防止僵尸进程
 *   **/
void child_exit(int signal)
{
	pid_t   pid;
	int     stat;
	while((pid = waitpid(-1,&stat,WNOHANG))>0)
	{
		printf("child %d ternimated\n",pid);
	}
	return ;
}

int	getchnlcfg(char *chnlname)
{
	FILE *fp;
	char	tmpbuf[60];
	memset(tmpbuf,0,sizeof(tmpbuf));
	char	cfgpath[100];
	memset(cfgpath,0,sizeof(cfgpath));

	if(chnlname == NULL)
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE[%d] 传入渠道名称为空\n",__FILE__,__LINE__);
		return -1;
	}
	/** 初始化系统所有渠道的队列区 **/
	sprintf(cfgpath,"%s%s",upshome,"/cfg/chnl.cfg");
	fp = fopen(cfgpath,"r");
	if(fp == NULL)
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE[%d]打开渠道初始化配置文件失败:[%s]\n",__FILE__,__LINE__,strerror(errno));
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
				if(!memcmp(tmpbuf,"远程队列名称",12))
				{
					strcpy(RQName,strstr(tmpbuf,":")+1);
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
int sendprocess(long inerid,MQLONG	*messlen)
{
	FILE	*fp = NULL;
	char	tmpfilename[60];

	memset(tmpfilename,0,sizeof(tmpfilename));
	memset(buffer,0,sizeof(buffer));
	memset(tmpbuffer,0,sizeof(tmpbuffer));
	memset(headbuf,0,sizeof(headbuf));

	SysLog(LOG_CHNL_SHOW,"&&&&&&&&&&&&&&&&&FILE [%s] LINE[%d] 开始处理[%ld]\n",__FILE__,__LINE__,inerid);
	/**get the shm **/ 
	if((get_shm_hash(inerid,tranbuf))!=-1)
	{
		/**
		  SysLog(LOG_CHNL_ERR,"交易跟踪号[%ld]\t传入交易信息[%s]\n",mbuf->innerid,tranbuf->outtran);
		  if(unpack(mbuf->tranbuf.chnlname,tranbuf->outtran)==-1)
		  {
		  SysLog(LOG_CHNL_ERR,"解[%s]包失败\t传入交易信息[%s]\n",mbuf->tranbuf.chnlname,tranbuf->outtran);
		  seterr("EEEEEEEE","解包失败");
		  }else
		  {
		 **/
		SysLog(LOG_CHNL_DEBUG,"FILE [%s] LINE[%d] 交易跟踪号[%ld]\t传入交易信息[%s][%s]\n",__FILE__,__LINE__,inerid,tranbuf->outtran,tranbuf->intran);
		if(strcmp(tranbuf->outtran,""))
		{
			strtok(tranbuf->outtran,"|");
			strcpy(tmpfilename,strtok(NULL,"|"));
			strcpy(headbuf,strtok(NULL,"|"));
			fp = fopen(tmpfilename,"r");
			if(fp==NULL)	
			{
				SysLog(LOG_CHNL_ERR,"FILE [%s] LINE[%d] 读取文件[%s]失败:%s\n",__FILE__,__LINE__,tmpfilename,strerror(errno));
				return -1;
			}
			if(fread(tmpbuffer, sizeof(tmpbuffer), 1,fp) != -1)
			{
				sprintf(buffer,"%s%s%s%s",headbuf,"\r","\n",tmpbuffer);
				*messlen = (MQLONG)strlen(buffer); /* length without null      */
				fclose(fp);
				return 0;
			}
		}else
		{
			SysLog(LOG_CHNL_ERR,"FILE [%s] LINE[%d] 传入数据为空:%s\n",__FILE__,__LINE__);
			return -1;
		}
		//}
	}else
	{
		SysLog(LOG_CHNL_ERR,"FILE[%s] LINE[%d]获取传入交易信息失败\n"__FILE__,__LINE__);
		return -1;
	}
}


int main(int argc, char **argv)
{
	/** 初始化全局共享内存前，先获取ups根路径 **/
	if(setupshome()==-1)
	{
		printf("设置全局变量upshome错误,请检查UPSHOME环境变量是否设置\n");
		return -1;
	}

	char    startcmd[200];
	int     i=0;
	memset(startcmd,0x00,sizeof(startcmd));
	/*  Declare file and character for sample input                   */

	/*   Declare MQI structures needed                                */
	MQOD     od = {MQOD_DEFAULT};    /* Object Descriptor             */
	MQMD     md = {MQMD_DEFAULT};    /* Message Descriptor            */
	MQPMO   pmo = {MQPMO_DEFAULT};   /* put message options           */
	/** note, sample uses defaults where it can **/

	MQHCONN  Hcon;                   /* connection handle             */
	MQHOBJ   Hobj;                   /* object handle                 */
	MQLONG   O_options;              /* MQOPEN options                */
	MQLONG   C_options;              /* MQCLOSE options               */
	MQLONG   CompCode;               /* completion code               */
	MQLONG   OpenCode;               /* MQOPEN completion code        */
	MQLONG   Reason;                 /* reason code                   */
	MQLONG   CReason;                /* reason code for MQCONN        */
	MQLONG   messlen;                /* message length                */


	int iret = 0;
	//pid_t	pid;


	memset(chnlname,0,sizeof(chnlname));


	strcpy(chnlname,"MQ发送渠道");
	if(getchnlcfg(chnlname)!=0)
	{
		SysLog(LOG_CHNL_ERR,"FILE[%s] LINE[%d] 获取渠道[%s]配置失败\n",__FILE__,__LINE__,chnlname);
		return -1;
	}
	SysLog(LOG_CHNL_SHOW,"FILE[%s] LINE[%d] 远程队列管理器[%s]\n",__FILE__,__LINE__,QMName);
	SysLog(LOG_CHNL_SHOW,"FILE[%s] LINE[%d] 远程队列[%s]\n",__FILE__,__LINE__,RQName);

	mbuf = (_msgbuf *)malloc(sizeof(_msgbuf));
	if(mbuf == (void *)-1)
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:MALLOC MSGBUF ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}

	tranbuf = (_tran *)malloc(sizeof(_tran));
	if(tranbuf == (void *)-1)
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:MALLOC TRANBUF ERROR[%s]\n",__FILE__,__LINE__,strerror(errno));
		free(mbuf);
		return -1;
	}

	if(getmsgid(chnlname,&msgidi,&msgido,&msgidr)==-1)
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:GET CHANNEL[%s] MSGID ERROR[%s]\n",__FILE__,__LINE__,chnlname,strerror(errno));
		return -1;
	}

	/** 设置忽略SIGPIPE信号，防止因socket写的时候客户端关闭导致的SIGPIPE信号 **/
	signal(SIGPIPE,SIG_IGN);
	signal(35,child_exit);
	signal(SIGCHLD,child_exit);

	//signal(SIGCHLD,child_exit);

	/******************************************************************/
	/*                                                                */
	/*   Connect to queue manager                                     */
	/*                                                                */
	/******************************************************************/
	//QMName[0] = 0;    /* default */
	//strncpy(QMName, "dev01", (size_t)MQ_Q_MGR_NAME_LENGTH);
	MQCONN(QMName,                  /* queue manager                  */
			&Hcon,                   /* connection handle              */
			&CompCode,               /* completion code                */
			&CReason);               /* reason code                    */

	/* report reason and stop if it failed     */
	if (CompCode == MQCC_FAILED)
	{
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE[%d] MQCONN MQ链接失败，错误码%d\n",__FILE__,__LINE__,CReason);
		exit( (int)CReason );
	}

	/******************************************************************/
	/*                                                                */
	/*   Use parameter as the name of the target queue                */
	/*                                                                */
	/******************************************************************/
	//strncpy(od.ObjectName, "RTEST", (size_t)MQ_Q_NAME_LENGTH);
	strncpy(od.ObjectName, RQName, (size_t)MQ_Q_NAME_LENGTH);
	SysLog(LOG_CHNL_SHOW,"FILE[%s] LINE[%d] target queue is %s\n",__FILE__,__LINE__, od.ObjectName);

	//strncpy(od.ObjectQMgrName, argv[5], (size_t) MQ_Q_MGR_NAME_LENGTH);
	//SysLog(LOG_CHNL_ERR,"target queue manager is %s\n", od.ObjectQMgrName);

	//strncpy(od.DynamicQName, argv[6], (size_t) MQ_Q_NAME_LENGTH);
	//SysLog(LOG_CHNL_ERR,"dynamic queue name is %s\n", od.DynamicQName);

	/******************************************************************/
	/*                                                                */
	/*   Open the target message queue for output                     */
	/*                                                                */
	/******************************************************************/
	if (argc > 3)
	{
		O_options = atoi( argv[3] );
		SysLog(LOG_CHNL_ERR,"FILE[%s] LINE[%d]open  options are %d\n",__FILE__,__LINE__, O_options);
	}
	else
	{
		O_options = MQOO_OUTPUT            /* open queue for output     */
			| MQOO_FAIL_IF_QUIESCING /* but not if MQM stopping   */
			;                        /* = 0x2010 = 8208 decimal   */
	}

	MQOPEN(Hcon,                      /* connection handle            */
			&od,                       /* object descriptor for queue  */
			O_options,                 /* open options                 */
			&Hobj,                     /* object handle                */
			&OpenCode,                 /* MQOPEN completion code       */
			&Reason);                  /* reason code                  */

	/* report reason, if any; stop if failed      */
	if (Reason != MQRC_NONE)
	{
		SysLog(LOG_CHNL_ERR,"FILE[%s] LINE[%d]MQOPEN ended with reason code %d\n",__FILE__,__LINE__, Reason);
	}

	if (OpenCode == MQCC_FAILED)
	{
		SysLog(LOG_CHNL_ERR,"FILE[%s] LINE[%d]unable to open queue for output\n",__FILE__,__LINE__);
	}

	/******************************************************************/
	/*                                                                */
	/*   Read lines from the file and put them to the message queue   */
	/*   Loop until null line or end of file, or there is a failure   */
	/*                                                                */
	/******************************************************************/
	CompCode = OpenCode;        /* use MQOPEN result for initial test */

	memcpy(md.Format,           /* character string format            */
			MQFMT_STRING, (size_t)MQ_FORMAT_LENGTH);

	pmo.Options = MQPMO_NO_SYNCPOINT
		| MQPMO_FAIL_IF_QUIESCING;

	/******************************************************************/
	/* Use these options when connecting to Queue Managers that also  */
	/* support them, see the Application Programming Reference for    */
	/* details.                                                       */
	/* The MQPMO_NEW_MSG_ID option causes the MsgId to be replaced,   */
	/* so that there is no need to reset it before each MQPUT.        */
	/* The MQPMO_NEW_CORREL_ID option causes the CorrelId to be       */
	/* replaced.                                                      */
	/******************************************************************/
	/* pmo.Options |= MQPMO_NEW_MSG_ID;                               */
	/* pmo.Options |= MQPMO_NEW_CORREL_ID;                            */

	signal(SIGUSR1,SIG_IGN);
	/** 添加该进程到进程控制表中，方便做统一处理 **/
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
		SysLog(LOG_CHNL_ERR,"FILE [%s] LINE [%d]:添加渠道[%s]到监控内存失败\n",__FILE__,__LINE__,chnlname);
		return -1;
	}
	/** 从消息队列读取数据，进行后续处理 **/
	while(1)
	{
		SysLog(LOG_CHNL_DEBUG,"FILE [%s] LINE [%d]:渠道[%s]开始服务\n",__FILE__,__LINE__,chnlname);
		memset(mbuf,0,sizeof(mbuf));
		if(msgrcv(msgidi,mbuf,sizeof(mbuf->tranbuf),0,0)==-1)
		{
			if(errno == EINTR)
			{
				continue;
			}else
			{
				SysLog(LOG_CHNL_ERR,"FILE[%s] LINE[%d] 获取渠道[%s]消息队列消息失败[%s]\n",__FILE__,__LINE__,chnlname,strerror(errno));
				sleep (5);
				continue;
			}
		}
		SysLog(LOG_CHNL_SHOW,"FILE[%s] LINE[%d] 渠道[%s]获取到跟踪号[%ld]\n",__FILE__,__LINE__,chnlname,mbuf->innerid);

		if(sendprocess(mbuf->innerid,&messlen)!=0)
		{
			SysLog(LOG_CHNL_ERR,"FILE[%s] LINE[%d] 处理失败\n",__FILE__,__LINE__);
			/** 返回交易信息到服务端**/
			iret = shm_hash_update(mbuf->innerid,"EEEEEEE|发送失败",NULL);
			if(iret == -1)
			{
				SysLog(LOG_CHNL_ERR,"FILE[%s] LINE[%d] 放置打包信息到共享内存失败 \n",__FILE__,__LINE__);
				msgsnd(msgidr,mbuf,sizeof(mbuf->tranbuf),IPC_NOWAIT);
			}else
			{
				msgsnd(msgidr,mbuf,sizeof(mbuf->tranbuf),IPC_NOWAIT);
			}
		}else
		{

			/****************************************************************/
			/*                                                              */
			/*   Put each buffer to the message queue                       */
			/*                                                              */
			/****************************************************************/
			SysLog(LOG_CHNL_SHOW,"FILE[%s] LINE[%d]buffer is [%s] len is [%d]\n",__FILE__,__LINE__,buffer,messlen);
			if (messlen > 0)
			{
				/**************************************************************/
				/* The following statement is not required if the             */
				/* MQPMO_NEW_MSG_ID option is used.                           */
				/**************************************************************/
				memcpy(md.MsgId,           /* reset MsgId to get a new one    */
						MQMI_NONE, sizeof(md.MsgId) );

				MQPUT(Hcon,                /* connection handle               */
						Hobj,                /* object handle                   */
						&md,                 /* message descriptor              */
						&pmo,                /* default options (datagram)      */
						messlen,             /* message length                  */
						buffer,              /* message buffer                  */
						&CompCode,           /* completion code                 */
						&Reason);            /* reason code                     */

				/* report reason, if any */
				if (Reason != MQRC_NONE)
				{
					SysLog(LOG_CHNL_ERR,"FILE[%s] LINE[%d]MQPUT ended with reason code %d\n",__FILE__,__LINE__, Reason);
					/** 返回交易信息到服务端**/
					iret = shm_hash_update(mbuf->innerid,"EEEEEEE|发送失败",NULL);
					if(iret == -1)
					{
						SysLog(LOG_CHNL_ERR,"FILE[%s] LINE[%d]放置打包信息到共享内存失败 \n",__FILE__,__LINE__);
						msgsnd(msgidr,mbuf,sizeof(mbuf->tranbuf),IPC_NOWAIT);
					}else
					{
						msgsnd(msgido,mbuf,sizeof(mbuf->tranbuf),IPC_NOWAIT);
					}
				}
				/** 返回交易信息到服务端**/
				iret = shm_hash_update(mbuf->innerid,"AAAAAAA|发送成功",NULL);
				if(iret == -1)
				{
					SysLog(LOG_CHNL_ERR,"FILE[%s] LINE[%d]放置打包信息到共享内存失败 \n",__FILE__,__LINE__);
					msgsnd(msgidr,mbuf,sizeof(mbuf->tranbuf),IPC_NOWAIT);
				}else
				{
					msgsnd(msgidr,mbuf,sizeof(mbuf->tranbuf),IPC_NOWAIT);
				}
			}
			else   /* satisfy end condition when empty line is read */
				CompCode = MQCC_FAILED;

		}
	}
	/******************************************************************/
	/*                                                                */
	/*   Close the target queue (if it was opened)                    */
	/*                                                                */
	/******************************************************************/
	if (OpenCode != MQCC_FAILED)
	{
		if (argc > 4)
		{
			C_options = atoi( argv[4] );
			SysLog(LOG_CHNL_ERR,"FILE[%s] LINE[%d]close options are %d\n",__FILE__,__LINE__, C_options);
		}
		else
		{
			C_options = MQCO_NONE;        /* no close options             */
		}

		MQCLOSE(Hcon,                   /* connection handle            */
				&Hobj,                  /* object handle                */
				C_options,
				&CompCode,              /* completion code              */
				&Reason);               /* reason code                  */

		/* report reason, if any     */
		if (Reason != MQRC_NONE)
		{
			SysLog(LOG_CHNL_ERR,"FILE[%s] LINE[%d]MQCLOSE ended with reason code %d\n",__FILE__,__LINE__,Reason);
		}
	}

	/******************************************************************/
	/*                                                                */
	/*   Disconnect from MQM if not already connected                 */
	/*                                                                */
	/******************************************************************/
	if (CReason != MQRC_ALREADY_CONNECTED)
	{
		MQDISC(&Hcon,                   /* connection handle            */
				&CompCode,               /* completion code              */
				&Reason);                /* reason code                  */

		/* report reason, if any     */
		if (Reason != MQRC_NONE)
		{
			SysLog(LOG_CHNL_ERR,"FILE[%s] LINE[%d]MQDISC ended with reason code %d\n",__FILE__,__LINE__, Reason);
		}
	}

	/******************************************************************/
	/*                                                                */
	/* END OF AMQSPUT0                                                */
	/*                                                                */
	/******************************************************************/
	SysLog(LOG_CHNL_ERR,"FILE[%s] LINE[%d]Sample AMQSPUT0 end\n",__FILE__,__LINE__);
	return(0);
}
