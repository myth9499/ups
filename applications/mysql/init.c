#include"ups.h"
int	init_var(char *xmltype)
{
	char	msghead[133];
	memset(msghead,0x00,sizeof(msghead));
	SysLog(1,"为[%s]初始化变量\n",xmltype);
	put_var_value("V_STAT",4,1,"0000");
	//put_var_value("V_MSGTYPE",20,1,"ccms.990.001.02     ");
	sprintf(msghead,"%s}%s%s","{H:02313513080408  HVPS402871099996  HVPS20141120210121XMLccms.990.001.02     2013090250000008    2013090220000008    3U         ","\r","\n");
	SysLog(1,"为msghead初始化变量[%s]",msghead);
	put_var_value("V_HEADBUF",133,1,msghead);
	return 0;
}
