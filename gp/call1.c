#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc,char *argv[])
{
	int result = 1,count = 0,i;
	int target = 1;
	if(argc > 1)
		target = atoi(argv[1]);
	if(target < 1)
	{
		printf("参数1错误，请输入希望生成的解决方案个数\n");
		return 1;
	}
	while(count < target)
	{	
		char a[50] = "./a.out "; 
		char b[5];
		sprintf(b,"%d",count);
		
		char c[20] = " > output/output";
		char d[5]; 
		sprintf(d,"%d",count);
		char e[5] = ".c";
		strcat(a,b);
		strcat(a,c);
		strcat(a,d);
		strcat(a,e);

		result = system(a);
		//printf("result = %d\n",WEXITSTATUS(result));

		if(WEXITSTATUS(result) == 1)
			printf("generate solution %d\n",count++);
		else 
			printf("No correct solution generated.\n");
	}
}

/*

*/

