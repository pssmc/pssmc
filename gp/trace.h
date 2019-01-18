#ifndef __TRACE_H_
#define __TRACE_H_

#include "treenode.h"
#include "mutation.h"
#include "../smc/State.h"

typedef struct trace
{
    int steplength;
    int numofprog;
    int numofvar;
    program** root;				//numofprog * program*
   
	int** valueofsema;			//steplength * numofsema * int
	int* executeprogid;			//steplength * int
   	treenode** executenode;		//steplength* treenode*
    int satisfied;
	State** states;				//(steplength + 1) * State*
}trace;

void setbadexamples(trace** t,int num);
void setNext(treenode* root);
void setParent(treenode* root);

double calculateFitness(organism* prog,Expr** exp,int numexp,double* coef);

void initTraceGlobalVar(int steplength);
void freeTrace(trace* t);
#endif
