#include"ups.h"
int	init_var(char *xmltype)
{
	SysLog(1,"为[%s]初始化变量\n",xmltype);
	put_var_value("V_STAT",4,1,"0000");
	return 0;
}
