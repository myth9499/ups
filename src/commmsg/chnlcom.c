#include "ups.h"

/** 发送报文到渠道**/
int send_to_channel(char *chnlname,char *trancode,char *buffsize)
{
	printf("into send to ch 1\n");
	_msgbuf	*mbuf ;
	int	msgidi,msgido;
	int iret ;

	mbuf = (_msgbuf *)malloc(sizeof(_msgbuf));
	if(mbuf == NULL)
	{
		printf("get mbuf error\n");
		return -1;
	}
	//memset(mbuf,0,sizeof(mbuf));
	mbuf->innerid = innerid;
	printf("into send to ch 2\n");
	//strcpy(mbuf.trancode,trancode);
	strcpy(mbuf->tranbuf.trancode,"test");
	mbuf->tranbuf.buffsize=123;
	if(getmsgid(chnlname,&msgidi,&msgido)!=0)
	{
		printf("get msgid error\n");
		free(mbuf);
		return -1;
	}
	printf("get channel[%s] msgid ok innerid[%ld]\n",chnlname,mbuf->innerid);
	iret = msgsnd(msgidi,mbuf,sizeof(mbuf->tranbuf),IPC_NOWAIT);
	if(iret == -1)
	{
		printf("send_to_chnl error");
		free(mbuf);
		return -1;
	}
	/** 同步接收渠道回应**/
	iret = msgrcv(msgido,mbuf,sizeof(mbuf->tranbuf),mbuf->innerid,0);
	if(iret == -1)
	{
		printf("recv from chnl error");
		free(mbuf);
		return -1;
	}
	free(mbuf);
	printf("send_to_chnl ok\n");
	return 0;
}
