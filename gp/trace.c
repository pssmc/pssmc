#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "treenode.h"
#include "trace.h"
#include "mutation.h"

int *varvalue = NULL;
int *semavalue = NULL;
treenode** nextstep = NULL;
treenode** currentstep = NULL;
trace* gtrace = NULL;

int numofcheck = 300;

extern int numProcess;
extern int numVarsInSpec;
extern char** nameOfVarsInSpec;
extern int* initValueOfVars;
extern int* initValueOfVarsInSpec;
int condnull = 0;

void freeTrace(trace* t)
{
	int i;
	for(i = 0;i < t->steplength;i++)
		free(t->valueofsema[i]);
	free(t->valueofsema);
	//for(i = 0;i < t->steplength;i++)
	//	free(t->valueofvar[i]);
	//free(t->valueofvar);
	free(t->executeprogid);
	free(t->executenode);
	free(t);
}

void setbadexamples(trace** traces,int num)
{
	
	int i,j;
	for(i = 0;i < num;i++)
	{
		for(j = 0;j < traces[i]->steplength;j++)
		{
			if(traces[i]->satisfied)
				traces[i]->executenode[j]->goodexamples++;
			else
				traces[i]->executenode[j]->badexamples++;
		}

	}
}

void setNext(treenode* root)
{
	if(root == NULL)
		return;

	if(root->type == 0)
	{
		if(root->treenode1 != NULL)	
			root->treenode1->next = root->next;
		if(root->treenode2 != NULL)
			root->treenode2->next = root->next;
	}
	else if(root->type == 1)
	{
		if(root->treenode1 != NULL)
			root->treenode1->next = root;
	}
	else if(root->type == 2)
	{
		root->treenode1->next = root->treenode2;
		root->treenode2->next = root->next;
	}
	setNext(root->treenode1);
	setNext(root->treenode2);
}

void setParent(treenode* root)
{
	if(root != NULL)
	{
		if(root->treenode1 != NULL)
		{
			root->treenode1->parent = root;
			setParent(root->treenode1);
		}			
		if(root->treenode2 != NULL)
		{
			root->treenode2->parent = root;	
			setParent(root->treenode2);
		}
	}
}

int getvarvalue(exp* e)
{
    if(e == NULL)
    {
		printf("getvarvalue null!\n");
		return -1;
    }
  
    if(e->type == 0)
	{
		if(e->index == 0)
			return 0;
		else if(e->index == 1)
			return 1;
		else
			printf("ERROR CONSTANT INDEX:%d\n",e->index);
	}
    if(e->type == 1)
	{
		if(e->index >= -1)
			return varvalue[e->index + 1];
		else
		{
			printf("ERROR Getvarvalue variable index < -1 :%d\n",e->index);
			return 0;
		}
	}
}

int getsemavalue(sema* s)
{
    if(s == NULL)
    {
		printf("getsemavalue null!\n");
		return -2;
    }	
	//printf("getsemavalue.index=%d,value=%d\n",s->index,semavalue[s->index]);
	return semavalue[s->index];
}

bool getcondvalue(cond* c)
{
    if(c == NULL)
    {
		printf("getcondvalue null!\n");
		condnull = 1;
		return false;
    }

    switch(c->type)
    {	
		case -1:if(nextrand(10) < 8) return true;else return false;			//??
		case 0:return true;
		case 1:return (getvarvalue(c->exp1) == getvarvalue(c->exp2));
		case 2:return (getvarvalue(c->exp1) != getvarvalue(c->exp2));
		case 3:return (getcondvalue(c->cond1) && getcondvalue(c->cond2));
		case 4:return (getcondvalue(c->cond1) || getcondvalue(c->cond2));
		default:printf("getcondvalue type error:%d\n",c->type);return false;
    }
}

void printvarvalue()
{
    
    int numofvars = 1 + numProcess;
    int i;
    for(i = 0;i < 1;i++)
		printf("v%d=%d  ",i,varvalue[i]);	

	printf("\n");
    for(i = 0;i < numProcess;i++)
    {
	    printf("p%d.v=%d",i,varvalue[1 + i]);
	    if(i < 10)
	    	printf("   ");
	    else
	    	printf(" ");
		printf("\n");
    }
    printf("\n");
}

void printTrace(trace* t,int length)
{
	int i;
	printf("\nprinttrace,length:%d\n",t->steplength);
	for(i = 0;i < length && i < t->steplength;i++)
	{
		printf("executeprogid:%d, type:%d, varvalue:",t->executeprogid[i],t->executenode[i]->type);
		if(t->executenode[i]->next != NULL)
			printf("next type:%d",t->executenode[i]->next->type);
		printf("\n");
	}

}

trace* gettrace(organism* org,int num)
{
    gtrace->root = org->progs;
    int i,j,k;
    int step = gtrace->steplength;			//???

 	for(i = 0;i < numProcess;i++)
	{
		nextstep[i] = gtrace->root[i]->root;
		currentstep[i] = NULL;
	}
	for(i = 0;i < numProcess + 1;i++)
		semavalue[i] = -1;
	for(i = 0;i < gtrace->numofvar;i++)
		varvalue[i] = initValueOfVars[i];


   	for(i = 0;i < step;i++)
    {
		bool allprogdone = true;
    	for(j = 0;j < numProcess;j++)
  		{
		    if(nextstep[j] != NULL)
	    	{
				allprogdone = false;
	    	}	
		}   
  		if(allprogdone)
		{
			gtrace->steplength = i;		//!!	
		    break;
		}

		int executeprogid;
	
		do
		{
			executeprogid = nextrand(numProcess);
		}while(nextstep[executeprogid] == NULL);
	
		while(nextstep[executeprogid]->type == 2)
			nextstep[executeprogid] = nextstep[executeprogid]->treenode1;

		currentstep[executeprogid] = nextstep[executeprogid];
		gtrace->executeprogid[i] = executeprogid;
		gtrace->executenode[i] = currentstep[executeprogid];	
		
		bool condition;
		switch(nextstep[executeprogid]->type)
		{
		   	case 0:	condition = getcondvalue(nextstep[executeprogid]->cond1);
		   			if(condition)
						nextstep[executeprogid] = nextstep[executeprogid]->treenode1;
		   			else
						nextstep[executeprogid] = nextstep[executeprogid]->next;
		   			break;
	    	case 1:	condition = getcondvalue(nextstep[executeprogid]->cond1);
		   			if(condition)
					{
						if(nextstep[executeprogid]->treenode1 != NULL)	
							nextstep[executeprogid] = nextstep[executeprogid]->treenode1;
					}
		  		 	else
						nextstep[executeprogid] = nextstep[executeprogid]->next;
		   			break;
	    	case 3:	varvalue[nextstep[executeprogid]->index + 1] = getvarvalue(nextstep[executeprogid]->exp1);
		   			nextstep[executeprogid] = nextstep[executeprogid]->next;
		   			break;
			case 4: if(getsemavalue(nextstep[executeprogid]->sema1) == -1)		//wait
					{	
						semavalue[nextstep[executeprogid]->sema1->index] = executeprogid;
						nextstep[executeprogid] = nextstep[executeprogid]->next;
					}
		   			break;
			case 5: if(getsemavalue(nextstep[executeprogid]->sema1) == executeprogid)//signal	
						semavalue[nextstep[executeprogid]->sema1->index] = -1;
					nextstep[executeprogid] = nextstep[executeprogid]->next;			  			
		   			break;
	    	case 6: nextstep[executeprogid] = nextstep[executeprogid]->next;
		   			break;
	    	case 7: nextstep[executeprogid] = nextstep[executeprogid]->next;
		   			break;
			case 8: nextstep[executeprogid] = nextstep[executeprogid]->next;
		   			break;
		}
		for(k = 0;k < numProcess + 1;k++)
		{
			gtrace->valueofsema[i][k] = semavalue[k];
		}
    }	
	return gtrace;
}

void setTraceStates(trace* t)
{
	int i,j;
	for(i = 0;i < t->states[0]->numvar;i++)
		t->states[0]->varvalue[i] = initValueOfVarsInSpec[i];
	
	int lastnodeincs = numProcess;
	for(i = 0;i < t->steplength;i++)
	{
		for(j = 0;j < t->states[0]->numvar;j++)
			t->states[i + 1]->varvalue[j] = t->states[i]->varvalue[j];

		int id = t->executeprogid[i];

		int lastindex = getVarindexFromState(t->states[0],"last");
		if(lastindex > -1)
			t->states[i + 1]->varvalue[lastindex] = lastnodeincs;	


		char temp1[10] = "cs";
		char idString[10];
		sprintf(idString,"%d",id);
		strcat(temp1,idString);
		int csindex = getVarindexFromState(t->states[0],temp1);
		char temp2[10] = "enter";
		strcat(temp2,idString);
		int enterindex = getVarindexFromState(t->states[0],temp2);

		if(csindex > -1 && enterindex > -1)
		{
			if(t->executenode[i]->type == 6)		
			{	
				t->states[i + 1]->varvalue[csindex] = 1;
				t->states[i + 1]->varvalue[enterindex]++;
				lastnodeincs = id;
			}
			else 
				t->states[i + 1]->varvalue[csindex] = 0;
		}

		char temp3[10] = "eating";
		strcat(temp3,idString);
		int eatingindex = getVarindexFromState(t->states[0],temp3);
		char temp4[10] = "eat";
		strcat(temp4,idString);
		int eatindex = getVarindexFromState(t->states[0],temp4);
		if(eatingindex > -1 && eatindex > -1)
		{
			if(t->executenode[i]->type == 7)		
			{	
				t->states[i + 1]->varvalue[eatingindex] = 1;
				t->states[i + 1]->varvalue[eatindex]++;
			}
			else
				t->states[i + 1]->varvalue[eatingindex] = 0;	
		}	

		for(j = 0;j < numProcess + 1;j++)
		{
			char temp5[10] = "s";
			char jString[10];
			sprintf(jString,"%d",j);
			strcat(temp5,jString);
			int sindex = getVarindexFromState(t->states[0],temp5);
			if(sindex > -1)
				t->states[i + 1]->varvalue[sindex] = t->valueofsema[i][j];
		}
	}
}

double calculateFitness(organism* prog,Expr** exp,int numexp,double* coef)
{
	int i;

	double* result = (double*)malloc(sizeof(double) * numexp);
	for(i = 0;i < numexp;i++)
		result[i] = 0;

	int count = 0;
	for(i = 0;i < numofcheck;i++)
	{
		trace* t = gettrace(prog,i);	
		setTraceStates(t);	
		int j;
		for(j = 0;j < numexp;j++)
		{
			double value = getExprValue(exp[j],t,0);
			result[j] += value;
		}
	}

	for(i = 0;i < numexp;i++)
	{
		result[i] = result[i] / (double)numofcheck;
	}
	
    double fitness = 0;
    for(i = 0;i < numexp;i++)
    {
		fitness += coef[i] * result[i];
     	prog->progs[0]->propertyfit[i] = result[i];   
	}

	return (fitness * 100);
}

void initTraceGlobalVar(int steplength)
{	
	gtrace = (trace*)malloc(sizeof(trace));
    gtrace->steplength = steplength;
    gtrace->numofprog = numProcess;
	gtrace->numofvar = 1 + numProcess;
    
	varvalue = (int*)malloc(sizeof(int) * gtrace->numofvar);
	nextstep = (treenode**)malloc(sizeof(treenode*) * gtrace->numofprog);
	currentstep = (treenode**)malloc(sizeof(treenode*) * gtrace->numofprog);

	//gtrace->valueofvar = (int**)malloc(sizeof(int*) * steplength);
	gtrace->executeprogid = (int*)malloc(sizeof(int) * steplength);
	if(gtrace->executeprogid == NULL)
	{
		printf("failed allocaating!\n");
		perror("do we have an error\n");
	}
	gtrace->executenode = (treenode**)malloc(sizeof(treenode*) * steplength);
    gtrace->satisfied = nextrand(2) == 0 ? true : false;

	gtrace->states = (State**)malloc(sizeof(State*) * (steplength + 1));
	int i;
	for(i = 0;i < steplength + 1;i++)
	{
		gtrace->states[i] = (State*)malloc(sizeof(State));
		gtrace->states[i]->numvar = numVarsInSpec;
		gtrace->states[i]->varvalue = (int*)malloc(sizeof(int) * gtrace->states[i]->numvar);
		gtrace->states[i]->varname = (char**)malloc(sizeof(char*) * gtrace->states[i]->numvar);
		int j;
		for(j = 0;j < gtrace->states[i]->numvar;j++)
			gtrace->states[i]->varname[j] = nameOfVarsInSpec[j];
	}
}


