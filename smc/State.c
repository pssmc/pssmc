#include "State.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int getVarindexFromState(State* st,char* name)
{
	int i;
	for(i = 0;i < st->numvar;i++)
		if(strcmp(st->varname[i],name) == 0)
			break;
	if(i < st->numvar)
		return i;
	else
	{
		return -1;
	}
}

int getVarvalueFromState(State* st,char* name)
{
	int i;
	for(i = 0;i < st->numvar;i++)
		if(strcmp(st->varname[i],name) == 0)
			break;
	if(i < st->numvar)
		return st->varvalue[i];
	else
	{
		printf("ERROR variable name:%s search in state\n",name);	
		for(i = 0;i < st->numvar;i++)
			printf("%d:%s\n",i,st->varname[i]);
		exit(-1);
	}
}
