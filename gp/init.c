#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "mutation.h"
#include "treerank.h"


double genseq = 0.4;

//input
int initTreeType;		//type of initial tree 1,2,3
int maxDepth;			//max depth of the program
int* maxNumStatements;	//max number of statements in one block in each depth
int numProcess;			//number of process		
int* typeStatements;		//type of statements 
int numVarsInSpec;		//number of variables in specifications
char** nameOfVarsInSpec;	//names of variables in specifications
int* initValueOfVars;	//initial values of variables generated in gp
int* initValueOfVarsInSpec;	//initial values of variables in specifications
int numConditionType;
int numOfBSpec;
double *coefOfBSpec;
int numOfSpec;
char* specification;
void initInputParamenters()
{
	FILE *fp = NULL;
	fp = fopen("../input.txt", "r");
	int input;

	fscanf(fp, "%d", &input);
	initTreeType = input;

	fscanf(fp, "%d", &input);
	maxDepth = input;

	maxNumStatements = (int*)malloc(sizeof(int) * maxDepth);
	int i;
	for(i = 0;i < maxDepth;i++)
	{
		fscanf(fp, "%d", &input);
		maxNumStatements[i] = input;
	}	

	fscanf(fp, "%d", &input);
	numProcess = input;

	typeStatements = (int*)malloc(sizeof(int) * 9);
	for(i = 0;i < 9;i++)
	{
		fscanf(fp, "%d", &input);
		typeStatements[i] = input;
	}	
	
	fscanf(fp, "%d", &input);
	numVarsInSpec = input;

	nameOfVarsInSpec = (char**)malloc(sizeof(char*) * numVarsInSpec);
	for(i = 0;i < numVarsInSpec;i++)
	{
		nameOfVarsInSpec[i] = (char*)malloc(sizeof(char) * 20);
		fscanf(fp, "%s", nameOfVarsInSpec[i]);
	}	

	initValueOfVars = (int*)malloc(sizeof(int) * (numProcess + 1));
	for(i = 0;i < numProcess + 1;i++)
	{
		fscanf(fp, "%d", &input);
		initValueOfVars[i] = input;
	}	

	initValueOfVarsInSpec = (int*)malloc(sizeof(int) * numVarsInSpec);
	for(i = 0;i < numVarsInSpec;i++)
	{
		fscanf(fp, "%d", &input);
		initValueOfVarsInSpec[i] = input;
	}	

	fscanf(fp, "%d", &input);
	numConditionType = input;

	fscanf(fp, "%d", &input);
	numOfBSpec = input;

	coefOfBSpec = (double*)malloc(sizeof(double) * numOfBSpec);
	double input2;
	for(i = 0;i < numOfBSpec;i++)
	{
		fscanf(fp, "%lf", &input2);
		coefOfBSpec[i] = input2;
	}	

	fscanf(fp, "%d", &input);
	numOfSpec = input;

	specification = (char*)malloc(sizeof(char) * 1000);
	fscanf(fp, "%s", specification);

	fclose(fp);
}

exp* createExp(int t,int ind)
{
  	exp* p = (exp*)malloc(sizeof(exp));
   	p->type = t;
  	p->index = ind;
   	return p;
}

sema* createSema(int ind)
{
	sema* s = (sema*)malloc(sizeof(sema));
	s->index = ind;
	return s;
}

cond* createCond(int t,exp* e1,exp* e2,cond* c1,cond* c2)
{
    cond* p = (cond*)malloc(sizeof(cond));
    p->type = t;
    p->exp1 = e1;
    p->exp2 = e2;
	p->cond1 = c1;
	p->cond2 = c2;
    return p;
}

treenode* createTreenode(int t,int ind,cond* c,treenode* t1,treenode* t2,exp* e)
{//printf("createtreenode type:%d",t);
    treenode* p = (treenode*)malloc(sizeof(treenode));
    p->type = t;
    p->index = ind;
    p->cond1 = c;
    p->treenode1 = t1;
    p->treenode2 = t2;
    p->exp1 = e;
    p->goodexamples = 1;
    p->badexamples = 1;
	p->next = NULL;
	p->parent = NULL;
	p->sema1 = NULL;
	p->pc = -1;
    return p;
}

int nextrand(int range)	//generate a random number between 0..range-1 
{
    int number = rand() % range;
   // printf("range=%d;number=%d;\n",range,number);
    return number;
}

int equalExp(exp* e1,exp* e2)
{
	if(e1 == NULL || e2 == NULL)
		return 0;
	if(e1->type == e1->type && e1->index == e2->index)
		return 1;
	return 0;
}

exp* genexp(program* prog,int vartype)
{

    int exptype = nextrand(2);
    if(vartype != -1)
		exptype = vartype;
    int varindex = 0;
    exp* result = NULL;
    switch(exptype)
    {
		case 0:	result = createExp(exptype,nextrand(4));
				break;
		case 1:	varindex = nextrand(1 + numProcess * 2) - numProcess - 1;//2:numofprog	2:me,other
	   			result = createExp(exptype,varindex);break;
    }
    return result;
}

sema* gensema()
{
	return createSema(nextrand(3) - 3);
}

cond* gencond(program* prog,int type)	//type==2:==,!=		type==4:==,!=,&&,||
{
    int condtype = nextrand(type);
    cond* result = NULL;
    switch(condtype)
    {	
		case 0: result = createCond(1,genexp(prog,1),genexp(prog,0),NULL,NULL);
				break;
		case 1: result = createCond(2,genexp(prog,1),genexp(prog,0),NULL,NULL);
				break;	
		case 2: result = createCond(3,NULL,NULL,gencond(prog,3),gencond(prog,3));
				break;	
		case 3: result = createCond(4,NULL,NULL,gencond(prog,3),gencond(prog,3));
				break;	
    }
    return result;
}

//0:IF  1:WHILE  2:SEQ 3:ASGN 4:WAIT 5:SIGNAL 6:cs	7:think 8:eat
treenode* genprog(int depth,program* prog)
{	
    int height = prog->maxdepth + 1 - depth;

	int* typeSet = NULL;
	int numOfStatementType = 0;
    if(height < 2)
	{
		if(typeStatements[3] == 1)
			numOfStatementType++;
		if(typeStatements[4] == 1)
			numOfStatementType++;
		if(typeStatements[5] == 1)
			numOfStatementType++;
		typeSet = (int*)malloc(sizeof(int) * numOfStatementType);
		int i;
		int temp = 0;
		for(i = 3;i <= 5;i++)
			if(typeStatements[i] == 1)
				typeSet[temp++] = i;
	}
    else
    {
		int i;
		for(i = 0;i < 6;i++)
			if(i != 2 && typeStatements[i] == 1)
				numOfStatementType++;

		typeSet = (int*)malloc(sizeof(int) * numOfStatementType);
		int temp = 0;
		for(i = 0;i < 6;i++)
			if(i != 2 && typeStatements[i] == 1)
				typeSet[temp++] = i;
    }
    
    int commandType = typeSet[nextrand(numOfStatementType)];	
    int varindex = 0;    

    treenode* result = NULL;
    switch(commandType)
    {
		case 0: result = createTreenode(0,0,gencond(prog,numConditionType),NULL,NULL,NULL);	
				result->treenode1 = genprog(depth + 1,prog);
				break;
		case 1: result = createTreenode(1,0,gencond(prog,numConditionType),NULL,NULL,NULL);
				result->treenode1 = genprog(depth + 1,prog);
				break;
		case 3: varindex = nextrand(2) - 2;	
			    result = createTreenode(3,varindex,NULL,NULL,NULL,genexp(prog,0));
				break;
		case 4: result = createTreenode(4,0,NULL,NULL,NULL,NULL);
				result->sema1 = gensema(); 
				break;
		case 5:	result = createTreenode(5,0,NULL,NULL,NULL,NULL);
				result->sema1 = gensema(); 
				break;
	}
	
    return result;
}

treenode* genCS(int depth,program* prog,int type)
{
	if(type == 1)
	{
		cond* c = createCond(-1,NULL,NULL,NULL,NULL);
		treenode* t1 = createTreenode(2,0,NULL,NULL,NULL,NULL); 
		treenode* result = createTreenode(1,0,c,t1,NULL,NULL);

		t1->treenode1 = genprog(prog->maxdepth,prog);
		t1->treenode2 = createTreenode(2,0,NULL,NULL,NULL,NULL);
		t1->treenode2->treenode1 = createTreenode(6,0,NULL,NULL,NULL,NULL);
		t1->treenode2->treenode2 = genprog(prog->maxdepth,prog);
		return result;
	}
	else if(type == 2)
	{
		cond* c = createCond(0,NULL,NULL,NULL,NULL);
		treenode* t1 = createTreenode(2,0,NULL,NULL,NULL,NULL); 
		treenode* result = createTreenode(1,0,c,t1,NULL,NULL);

		t1->treenode1 = genprog(prog->maxdepth,prog);
		t1->treenode2 = createTreenode(2,0,NULL,NULL,NULL,NULL);
		t1->treenode2->treenode1 = createTreenode(6,0,NULL,NULL,NULL,NULL);
		t1->treenode2->treenode2 = genprog(prog->maxdepth,prog);
		return result;
	}
	else if(type == 3)
	{
		cond* c = createCond(-1,NULL,NULL,NULL,NULL);
		treenode* t1 = createTreenode(2,0,NULL,NULL,NULL,NULL); 
		treenode* result = createTreenode(1,0,c,t1,NULL,NULL);

		t1->treenode1 = createTreenode(2,0,NULL,NULL,NULL,NULL);
	
		t1->treenode1->treenode1 = createTreenode(7,0,NULL,NULL,NULL,NULL);
		t1->treenode1->treenode2 = genprog(prog->maxdepth,prog);

		t1->treenode2 = createTreenode(2,0,NULL,NULL,NULL,NULL);
		t1->treenode2->treenode1 = createTreenode(8,0,NULL,NULL,NULL,NULL);
		t1->treenode2->treenode2 = genprog(prog->maxdepth,prog);
		return result;
	}
	return NULL;
}

//0:CONST  1:VAR  
void printexp(exp* e,program* prog)
{
    switch(e->type)
    {
		case 0:	if(e->index < 2)
					printf("%d",e->index);
				else if(e->index == 2)
					printf("me");
				else if(e->index == 3)
					printf("other");
				break;
		case 1:	if(e->index > 0)
					printf("v[%d]",e->index);
				else if(e->index == -1)
					printf("v[p]");
				else if(e->index == -2)
					printf("v[me]");
				else if(e->index < -2)
				{
					if(numProcess == 2)
						printf("v[other]");
					else
						printf("v[other%d]",-e->index - 2);
				}
    }
}

 
//0:EQ    1:NEQ   2:AND  3:OR 
void printcond(cond* c,program* prog)
{
	if(c == NULL)
		return;
    switch(c->type)
    {
		case -1:printf("wi");break;
		case 0:printf("true");break;
		case 1:printexp(c->exp1,prog);printf(" == ");printexp(c->exp2,prog);break;
		case 2:printexp(c->exp1,prog);printf(" != ");printexp(c->exp2,prog);break;
		case 3:printf("(");printcond(c->cond1,prog);printf(") && (");printcond(c->cond2,prog);printf(")");break;
		case 4:printf("(");printcond(c->cond1,prog);printf(") || (");printcond(c->cond2,prog);printf(")");break;
    }
}

//0:IF  1:WHILE  2:SEQ  3:ASGN	4:WAIT	5:SIGNAL  6:CS  7:THINK  8:EAT
void printprog(treenode* root,int blank,program* prog)
{	//printf("printprog progtype:%d blank:%d\n",prog->type,blank);
    int i;
    if(root == NULL)	
		return;
    switch(root->type)
    {
		case 0: for(i = 0;i < blank;i++)printf(" ");printf("if(");
				printcond(root->cond1,prog);
				printf(")\n");
				for(i = 0;i < blank;i++)printf(" ");printf("{\n");
				printprog(root->treenode1,blank + 2,prog);
				for(i = 0;i < blank;i++)printf(" ");printf("}\n");
				if(root->treenode2 == NULL)break;
				for(i = 0;i < blank;i++)printf(" ");printf("else\n");
				for(i = 0;i < blank;i++)printf(" ");printf("{\n");
				printprog(root->treenode2,blank + 2,prog);
				for(i = 0;i < blank;i++)printf(" ");printf("}\n");
				break;
		case 1: for(i = 0;i < blank;i++)printf(" ");printf("while(");
				printcond(root->cond1,prog);
				printf(")\n");
				for(i = 0;i < blank;i++)printf(" ");printf("{\n");
				printprog(root->treenode1,blank + 2,prog);
				for(i = 0;i < blank;i++)printf(" ");printf("}\n");
				break;
		case 2: printprog(root->treenode1,blank,prog);
				printprog(root->treenode2,blank,prog);
				break;		
		case 3: for(i = 0;i < blank;i++)printf(" ");
				if(root->index >= 0)
					printf("v[%d] = ",root->index);				
				else if(root->index == -1)
					printf("v[p] = ");
				else if(root->index == -2)	//error
					printf("v[me] = ");
				printexp(root->exp1,prog);
				printf(";\n");
				break;
		case 4: for(i = 0;i < blank;i++)printf(" ");
				if(root->sema1->index == -1)
					printf("wait(mutex);\n");
				else if(root->sema1->index == -2)
					printf("wait(left);\n");
				else if(root->sema1->index == -3)
					printf("wait(right);\n");
				else
					printf("wait(sema[%d]);\n",root->sema1->index);
				break;
		case 5: for(i = 0;i < blank;i++)printf(" ");
				if(root->sema1->index == -1)
					printf("signal(mutex);\n");
				else if(root->sema1->index == -2)
					printf("signal(left);\n");
				else if(root->sema1->index == -3)
					printf("signal(right);\n");
				else
					printf("signal(sema[%d]);\n",root->sema1->index);
				break;
		case 6: for(i = 0;i < blank;i++)printf(" ");
				printf("critical section%d\n",prog->progid);
		case 7: for(i = 0;i < blank;i++)printf(" ");
    			printf("think\n");
    			break;
    	case 8:for(i = 0;i < blank;i++)printf(" ");
    			printf("eat\n");
    			break;
    }
}

int setPc(treenode* t, int pc0)
{	
	if(t == NULL)
		return pc0;

	if(t->type == 0)		//if
	{
		t->pc = ++pc0;
		pc0 = setPc(t->treenode1,pc0);
		return setPc(t->treenode2,pc0);
	}

	if(t->type == 1)		//while
	{
		t->pc = ++pc0;		
		if(t->depth == 1)	//!!not generalized. try		??
			pc0++;
		return setPc(t->treenode1,pc0);
	}

	if(t->type == 2)		//SEQ	
	{
		pc0 = setPc(t->treenode1,pc0);
		return setPc(t->treenode2,pc0);
	}

	if((t->type >= 3 && t->type <= 5) || t->type == 7)		//assign,wait,signal,think
	{
		t->pc = ++pc0;
		return pc0;		
	}

	if(t->type == 6)		//cs
	{
		t->pc = pc0 + 1;
		return (pc0 + 3);
	}

	if(t->type == 8)		//eat
	{
		t->pc = pc0 + 1;
		return (pc0 + 2);	
	}
}

int setNumOfStatementsUp(treenode* t)
{
	if(t == NULL)
		return 0;

	int l = setNumOfStatementsUp(t->treenode1);
	int r = setNumOfStatementsUp(t->treenode2);

	if(t->type != 2)
		t->numofstatements = 1;
	else
		t->numofstatements = l + r;
	return t->numofstatements;
}

void setNumOfStatementsDown(treenode* t,int num)
{
	if(t == NULL)
		return;
	if(t->type == 2)
	{
		if(num > t->numofstatements)
			t->numofstatements = num;

		setNumOfStatementsDown(t->treenode1,t->numofstatements);
		setNumOfStatementsDown(t->treenode2,t->numofstatements);
	}
	else
	{
		t->numofstatements = num;
		setNumOfStatementsDown(t->treenode1,1);
		setNumOfStatementsDown(t->treenode2,1);	
	}
}

void setNumOfStatements(treenode* root)
{
	setNumOfStatementsUp(root);	
	setNumOfStatementsDown(root,1);
}

void setLinesTreenode(treenode* t,int depth)		//set height of nodes
{
    if(t == NULL)
		return;
	//printf("1.0\n");

    t->depth = depth;

	if(t->type == 2)
	{
 	   	setLinesTreenode(t->treenode1,depth);
    	setLinesTreenode(t->treenode2,depth);
	}
	else
	{
 	   	setLinesTreenode(t->treenode1,depth + 1);
    	setLinesTreenode(t->treenode2,depth + 1);
	}
	
	int height = 0;
    if(t->treenode1 != NULL)
		if(t->treenode1->height > height)
	    	height = t->treenode1->height;
    if(t->treenode2 != NULL)
		if(t->treenode2->height > height)
			height = t->treenode2->height;  
 
	if(t->type == 2)
    	t->height = height;
	else
    	t->height = height + 1;
}

int setFixed(treenode* t)
{
    if(t == NULL)
		return 0;

	if(t->fixed == 1)
	{
		setFixed(t->treenode1);
		setFixed(t->treenode2);
		return 1;
	}

    if(t->type == 0)
    {
		int a = setFixed(t->treenode1);
		int b = setFixed(t->treenode2);
		if(a == 0 && b == 0)
			t->fixed = 0;
		else
			t->fixed = 1;
    }
    else if(t->type == 1)
		t->fixed = setFixed(t->treenode1);
    else if(t->type == 2)
    {	if(t->treenode1 == NULL || t->treenode2 == NULL)
			printf("setfixed error\n");
		int a = setFixed(t->treenode1);
		int b = setFixed(t->treenode2);
		if(a == 0 && b == 0)
			t->fixed = 0;
		else
			t->fixed = 1;
    }
	else
		t->fixed = 0;

    return t->fixed;	
}


void setAll(program* prog)
{
	prog->root->next = NULL;
	setNext(prog->root);
 	setParent(prog->root);
	prog->root->parent = NULL;
	setFixed(prog->root);
	setNumOfStatements(prog->root);
	setLinesTreenode(prog->root,1);	
    setPc(prog->root,-1);
}

int getFixed(treenode* t)
{
    if(t == NULL)
		return 0;
    else
		return t->fixed;
}

program** genInitTemplate(int num)
{
	program** inittemplate = (program**)malloc(sizeof(program*) * num);
	int i;
	for(i = 0;i < num;i++)
	{
		inittemplate[i] = (program*)malloc(sizeof(program));
		inittemplate[i]->maxdepth = maxDepth;
		inittemplate[i]->root = genCS(1,inittemplate[i],initTreeType);	
		inittemplate[i]->checkedBySpin = 0;
		setAll(inittemplate[i]);
	}	
    return inittemplate;
}

void setMeOtherExpression(exp* e,int progid)
{
	if(e == NULL)
		return;
	if(e->type == 0)
	{
		if(e->index == 3)
		{
			if(progid <= 1)	
				e->index = progid;
			else
				e->index = 0;
		}
		else if(e->index == 4)
		{
			if(progid <= 1)
				e->index = 1 - progid;
			else
				e->index = 1;
		}
	}
	else
	{
		if(e->index <= -2)
			e->index = (progid - e->index - 2) % numProcess;
	}
}

void setMeOtherSemaphore(sema* s,int progid)
{
		if(s == NULL)
			return;
		if(s->index == -1)
			s->index = numProcess;
		else if(s->index == -2)
			s->index = progid;
		else if(s->index == -3)
			s->index = (progid + 1) % numProcess;
}

void setMeOtherCondition(cond* c,int progid)
{
	if(c == NULL)
		return;
	if(c->type > 0)
	{
		setMeOtherExpression(c->exp1,progid);
		setMeOtherExpression(c->exp2,progid);
		setMeOtherCondition(c->cond1,progid);
		setMeOtherCondition(c->cond2,progid);
	}
}

//0:IF  1:WHILE  2:SEQ 3:ASGN 4:WAIT 5:SIGNAL 6:cs	7:think 8:eat
void setMeOtherTreenode(treenode* t,int progid)
{
	if(t == NULL)
		return;
	if(t->type == 3 && t->index == -2)
		t->index = progid;

	setMeOtherTreenode(t->treenode1,progid);
	setMeOtherTreenode(t->treenode2,progid);
	setMeOtherSemaphore(t->sema1,progid);
	setMeOtherCondition(t->cond1,progid);
	setMeOtherExpression(t->exp1,progid);		
}

program* genProgram(program* templat,int progid)
{
	program* newprog = copyProgram(templat);
	newprog->progid = progid;
	setMeOtherTreenode(newprog->root,progid);

	setAll(newprog);
	return newprog;
}

organism* genOrganism(program* templat)
{
	organism* result = (organism*)malloc(sizeof(organism));
	result->progs = (program**)malloc(sizeof(program*) * numProcess);
	int i;
	for(i = 0;i < numProcess;i++)
		result->progs[i] = genProgram(templat,i);
	return result;
}

void freeOrganism(organism* org)		
{
	if(org == NULL)
		return;	
	int i;
	for(i = 0;i < numProcess;i++)
		freeProgram(org->progs[i]);
	free(org);
}

void freeProgram(program* prog)
{
	if(prog == NULL)
		return;		
	freeTreenode(prog->root);
	free(prog);
}

void freeTreenode(treenode* t)
{
	if(t == NULL)
		return;
	freeTreenode(t->treenode1);
	freeTreenode(t->treenode2);
	freeCondition(t->cond1);
	if(t->exp1 != NULL)
		free(t->exp1);
	if(t->sema1 != NULL)
		free(t->sema1);
	free(t);
}	

void freeCondition(cond* c)
{
	if(c == NULL)
		return;
	freeCondition(c->cond1);
	freeCondition(c->cond2);
	if(c->exp1 != NULL)
		free(c->exp1);
	if(c->exp2 != NULL)
		free(c->exp2);
	free(c);
}

void expToPml(exp* e,FILE* f)
{
	if(e == NULL)
		return;

	if(e->type == 0)
	{
		if(e->index == 0)
			fputs("0",f);
		else if(e->index == 1)
			fputs("1",f);
	}
	else if(e->type == 1)
	{
		if(e->index >= 0)
			fprintf(f,"v%d",e->index);	
		else if(e->index == -1)
			fputs("vp",f);
	}
}

void condToPml(cond* c,FILE* f,int blank, int progid)
{
	if(c == NULL)
		return;
	int i;
	for(i = 0;i < blank;i++)
		fputs("\t",f);
	fputs("::",f);
	
	if(c->type == -1)
	{
		fprintf(f,"wi%d == 0->\n",progid);
		for(i = 0;i < blank + 1;i++)
			fputs("\t",f);
		if(typeStatements[5] == 1)
			fprintf(f,"try%d = 1;\n",progid);		

		for(i = 0;i < blank + 1;i++)
			fputs("\t",f);
		fprintf(f,"select(wi%d:0..1);\n",progid);
	}
	else if(c->type == 0)
		fputs("true->\n",f);
	else if(c->type == 1)
	{		
		expToPml(c->exp1,f);
		fputs(" == ",f);
		expToPml(c->exp2,f);
		fputs("->\n",f);
	}
	else if(c->type == 2)
	{
		expToPml(c->exp1,f);
		fputs(" != ",f);
		expToPml(c->exp2,f);
		fputs("->\n",f);
	}
	else if(c->type == 3)
	{
		if(c->cond1->type == 0)
			fputs("true",f);	
		else if(c->cond1->type == 1)
		{
			expToPml(c->cond1->exp1,f);
			fputs(" == ",f);
			expToPml(c->cond1->exp2,f);
		}
		else if(c->cond1->type == 2)
		{
			expToPml(c->cond1->exp1,f);
			fputs(" != ",f);
			expToPml(c->cond1->exp2,f);
		}

		fputs(" && ",f);
		
		if(c->cond2->type == 0)
			fputs("true",f);	
		else if(c->cond2->type == 1)
		{
			expToPml(c->cond2->exp1,f);
			fputs(" == ",f);
			expToPml(c->cond2->exp2,f);
		}
		else if(c->cond2->type == 2)
		{
			expToPml(c->cond2->exp1,f);
			fputs(" != ",f);
			expToPml(c->cond2->exp2,f);
		}
		fputs("->\n",f);
	}
	else if(c->type == 4)
	{
		if(c->cond1->type == 0)
			fputs("true",f);	
		else if(c->cond1->type == 1)
		{
			expToPml(c->cond1->exp1,f);
			fputs(" == ",f);
			expToPml(c->cond1->exp2,f);
		}
		else if(c->cond1->type == 2)
		{
			expToPml(c->cond1->exp1,f);
			fputs(" != ",f);
			expToPml(c->cond1->exp2,f);
		}

		fputs(" || ",f);
		
		if(c->cond2->type == 0)
			fputs("true",f);	
		else if(c->cond2->type == 1)
		{
			expToPml(c->cond2->exp1,f);
			fputs(" == ",f);
			expToPml(c->cond2->exp2,f);
		}
		else if(c->cond2->type == 2)
		{
			expToPml(c->cond2->exp1,f);
			fputs(" != ",f);
			expToPml(c->cond2->exp2,f);
		}
		fputs("->\n",f);
	}
}

void progToPml(treenode* t,FILE* f,program* prog,int blank)
{
	if(t == NULL)
		return;
	int i;
	if(t->type == 0)
	{
		for(i = 0;i < blank;i++)
			fputs("\t",f);
		fputs("if\n",f);	
		condToPml(t->cond1,f,blank + 1,prog->progid);
		progToPml(t->treenode1,f,prog,blank + 2);

		for(i = 0;i < blank;i++)
			fputs("\t",f);
		fputs("fi\n",f);
	}
	else if(t->type == 1)
	{
		for(i = 0;i < blank;i++)
			fputs("\t",f);
		fputs("do\n",f);	
		condToPml(t->cond1,f,blank + 1,prog->progid);
		progToPml(t->treenode1,f,prog,blank + 2);
		
		for(i = 0;i < blank + 1;i++)
			fputs("\t",f);
		fputs("::else->break;\n",f);

		for(i = 0;i < blank;i++)
			fputs("\t",f);
		fputs("od\n",f);
	}
	else if(t->type == 2)
	{
		progToPml(t->treenode1,f,prog,blank);	
		progToPml(t->treenode2,f,prog,blank);
	}
	else if(t->type == 3)
	{
		for(i = 0;i < blank;i++)
			fputs("\t",f);		
		if(t->index >= 0)
			fprintf(f,"v%d = ",t->index);
		else if(t->index == -1)
			fputs("turn = ",f);

		expToPml(t->exp1,f);
		fputs(";\n",f);
	}
	else if(t->type == 4)
	{
		for(i = 0;i < blank;i++)
			fputs("\t",f);
		fputs("do\n",f);

		for(i = 0;i < blank + 1;i++)
			fputs("\t",f);
		fprintf(f,"::true->atomic{s%d==-1->s%d = %d;break;}\n",t->sema1->index,t->sema1->index,prog->progid);
				
		for(i = 0;i < blank + 1;i++)
			fputs("\t",f);
		fputs("::else->\n",f);

		for(i = 0;i < blank;i++)
			fputs("\t",f);
		fputs("od\n",f);		
	}
	else if(t->type == 5)
	{
		for(i = 0;i < blank;i++)
			fputs("\t",f);
		fprintf(f,"atomic{s%d==%d->s%d = -1;}\n",t->sema1->index,prog->progid,t->sema1->index);
	}
	else if(t->type == 6)
	{
		for(i = 0;i < blank;i++)
			fputs("\t",f);
		fprintf(f,"cs%d++;\n",prog->progid);
		for(i = 0;i < blank;i++)
			fputs("\t",f);
		fprintf(f,"try%d = 0;\n",prog->progid);	
		for(i = 0;i < blank;i++)
			fputs("\t",f);
		fprintf(f,"cs%d--;\n",prog->progid);		
	}
	else if(t->type == 7)
	{
		for(i = 0;i < blank;i++)
			fputs("\t",f);
		fprintf(f,"t%d = 1;\n",prog->progid);
		for(i = 0;i < blank;i++)
			fputs("\t",f);
		fprintf(f,"t%d = 0;\n",prog->progid);
	}
	else if(t->type == 8)
	{
		for(i = 0;i < blank;i++)
			fputs("\t",f);
		fprintf(f,"e%d = 1;\n",prog->progid);
		for(i = 0;i < blank;i++)
			fputs("\t",f);
		fprintf(f,"e%d = 0;\n",prog->progid);

	}
}      

void orgToPml(organism* org,FILE* f)		//??
{
	fprintf(f,"bit vp = %d",initValueOfVars[0]);
	int i;
	for(i = 0;i < numProcess;i++)
		fprintf(f,",v%d = %d",i,initValueOfVars[i + 1]);
	for(i = 0;i < numProcess;i++)
		fprintf(f,",try%d = 0",i);
	for(i = 0;i < numProcess;i++)
		fprintf(f,",wi%d = 0",i);
	fputs(";\n",f);

	fputs("byte ",f);
	for(i = 0;i < numVarsInSpec;i++)
	{
		fprintf(f,"%s = %d",nameOfVarsInSpec[i], initValueOfVars[i]);
		if(i < numVarsInSpec)
			fputs(",",f);
		else
			fputs(";\n",f);
	}	

	for(i = 0;i < numProcess;i++)
	{
		fprintf(f,"active proctype p%d()\n{\n\tselect(wi%d:0..1);\n",i,i);
		progToPml(org->progs[i]->root,f,org->progs[i],1);
		fputs("}\n\n",f);
	}

	fprintf(f,"%s",specification);
	
	fputs("ltl e1{[]mutex}\nltl e2{[]((try0 == 1) -> <>(cs0 == 1))}\nltl e3{[]((try1 == 1) -> <>(cs1 == 1))}",f);
}	
