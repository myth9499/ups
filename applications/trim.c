#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
void rtrim(char *s)
{
	char	*str ;
	str = s;
	int  j = strlen(str)-1;
	while(1)
	{
		printf("!!!!!!!\t [%c]\n",str[j]);
		if(str[j]==' ')
		{
			str[j+1]='\0';
			j--;
		} else
		{
			printf("go out\n");
			break;
		}
	}
	printf("str is [%s]\n",str);
}
char *ltrim(char *s)
{
	int  i =0;
	char	*p;
	p=s;
	while(1)
	{
		if((p[i]==' ')||(p[i]=='\t'))
		{
			printf("123");
			s++;
			i++;
	printf("s !!!!!!!!is [%s]\n",s);
		}else
		{
			break;
		}
	}

	printf("s is [%s]\n",s);
	return s;
}

char *trim (char *str)
{
	char	*p = str;
	char	*p1;
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
	printf("!!![%s]\n",p);
	strcpy(str,p);
}
char *trim2 (char *str)
{
	char	*p1;
		p1 = str+strlen(str)-1;
	if(str)
	{
		while(*str&&isspace(*str))
		{
			str++;
		}
		while(p1>str&&isspace(*p1))
			*p1--='\0';
	}
	printf("@@@[%s]\n",str);
	//strcpy(str,p);
}
int main(int argc,char *argv[])
{
	char buff[100];
	strcpy(buff,"              hello            ");
	trim(buff);
	printf("buff is [%s]\n",buff);
	return 0;
}
