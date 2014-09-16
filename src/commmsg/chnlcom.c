#include "ups.h"

/** 发送报文到渠道**/
int send_to_channel(char *chnlname)
{
	SysLog(1,"开始发送报文到渠道[%s]\n",chnlname);
	_msgbuf	*mbuf ;
	int	msgidi,msgido,msgidr;
	int iret ;

	mbuf = (_msgbuf *)malloc(sizeof(_msgbuf));
	if(mbuf == NULL)
	{
		printf("get mbuf error\n");
		return -1;
	}
	memset(mbuf,0,sizeof(mbuf));
	mbuf->innerid = innerid;
	memset(mbuf->tranbuf.trancode,0,sizeof(mbuf->tranbuf.trancode));
	strcpy(mbuf->tranbuf.trancode,"核心服务");
	memset(mbuf->tranbuf.chnlname,0,sizeof(mbuf->tranbuf.chnlname));
	mbuf->tranbuf.buffsize=123;
	if(getmsgid(chnlname,&msgidi,&msgido,&msgidr)!=0)
	{
		SysLog(1,"获取渠道[%s]消息队列失败\n",chnlname);
		free(mbuf);
		return -1;
	}
	iret = msgsnd(msgidi,mbuf,sizeof(mbuf->tranbuf),IPC_NOWAIT);
	if(iret == -1)
	{
		SysLog(1,"发送到渠道[%s]失败\n",chnlname);
		free(mbuf);
		return -1;
	}
	free(mbuf);
	SysLog(1,"发送到渠道[%s]成功\n",chnlname);
	return 0;
}
int recv_from_channel(char *chnlname)
{
	SysLog(1,"开始接收渠道[%s]返回报文\n",chnlname);
	_msgbuf	*mbuf ;
	int	msgidi,msgido,msgidr;
	int iret ;
	_tran *tranbuf;

	mbuf = (_msgbuf *)malloc(sizeof(_msgbuf));
	if(mbuf == NULL)
	{
		printf("get mbuf error\n");
		return -1;
	}
	memset(mbuf,0,sizeof(mbuf));
	if(getmsgid(chnlname,&msgidi,&msgido,&msgidr)!=0)
	{
		SysLog(1,"获取渠道[%s]消息队列失败\n",chnlname);
		free(mbuf);
		return -1;
	}
	iret = msgrcv(msgidr,mbuf,sizeof(mbuf->tranbuf),innerid,0);
	if(iret == -1)
	{
		SysLog(1,"接收渠道[%s]返回报文失败:%s\n",chnlname,strerror(errno));
		free(mbuf);
		return -1;
	}
	SysLog(1,"接收渠道[%s]返回报文成功\n",chnlname);

	/** 开始解析返回报文**/
	tranbuf = (_tran *)malloc(sizeof(_tran));
	if(tranbuf == NULL)
	{
		SysLog(1,"申请交易数据内存失败[%s]\n",strerror(errno));
		return -1;
	}
	memset(mbuf->tranbuf.chnlname,0,sizeof(mbuf->tranbuf.chnlname));
	sprintf(mbuf->tranbuf.chnlname,"%s返回",chnlname);
	if((get_shm_hash(mbuf->innerid,tranbuf))!=-1)
	{
		SysLog(1,"交易跟踪号[%ld]\t[%s]渠道返回信息信息[%s]\n",mbuf->innerid,chnlname,tranbuf->intran);
		if(unpack(mbuf->tranbuf.chnlname,tranbuf->intran,"|")==-1)
		{
			SysLog(1,"解[%s]包失败\t传入交易信息[%s]\n",mbuf->tranbuf.chnlname,tranbuf->intran);
			seterr("EEEEEEEE","解包失败");
		}
	}
	free(tranbuf);
	free(mbuf);
	return 0;
}
