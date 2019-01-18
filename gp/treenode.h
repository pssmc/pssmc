#ifndef __TREENODE_H_
#define __TREENODE_H_

/*
int numprivatevars = 3;
int numpublicvars = 5;
int progid = 5;
int maxconst = 2;
int expdepth = 2;
int condepth = 4;
int progdepth = 10;
*/
#include <stdbool.h>
#include <stdio.h>



extern int maxDepth;			//max depth of the program
extern int* maxNumStatements;	//max number of statements in one block in each depth
extern int numProcess;			//number of process		
extern int* typeStatements;		//type of statements 
extern int numVarsInSpec;		//number of variables in specifications
extern char** nameOfVarsInSpec;	//names of variables in specifications
extern int* initValueOfVars;	//initial values of variables generated in gp
extern int* initValueOfVarsInSpec;	//initial values of variables in specifications

typedef struct expression
{
    int type;	//0:CONST  1:VAR 		 	
    int index;	//0:value of CONST  1:index of VAR 
				//CONST	0:0		1:1		2:me	3:other
				//VAR: -1:public variables	<=-2:me, other1,other2...	
}exp;

typedef struct semaphore
{
	int index;	//-1:mutex	-2:left -3:right 
}sema;

typedef struct condition
{
    int type;	
	//	-1:wi;	0:TRUE	1:EQ;	2:NEQ 	3:AND	4:OR

    exp* exp1;
    exp* exp2; 	
	struct condition* cond1;
	struct condition* cond2;
}cond;


typedef struct treenode
{
    int type;//0:IF  1:WHILE  2:SEQ 3:ASGN 4:WAIT 5:SIGNAL 6:cs	7:think 8:eat
    int index;//ASSIGN left VAR index	-1:v[p]  -2:v[me]
    cond* cond1;
    struct treenode* treenode1;
    struct treenode* treenode2;
    struct treenode* next;
	struct treenode* parent;
    exp* exp1;
	sema* sema1;
	int goodexamples;
	int badexamples;
	int pc;
	int depth;
	int height;
	int numofstatements;
	int fixed;
}treenode;

typedef struct program
{
	treenode* root;
	int maxdepth;
	int progid;
	double fitness;
	int numofevent[7];			//???
	double propertyfit[20];		//???
	int checkedBySpin;
}program;

typedef struct organism
{
	program** progs;
	double fitness;
}organism;

void initInputParamenters();

//bool checkIfFixed(treenode* t);
exp* createExp(int t,int ind);
sema* createSema(int ind);
cond* createCond(int t,exp* e1,exp* e2,cond* c1,cond* c2);
treenode* createTreenode(int t,int ind,cond* c,treenode* t1,treenode* t2,exp* e);

int nextrand(int range);
int equalExp(exp* e1,exp* e2);
void printprog(treenode* root,int blank,program* prog);
void orgToPml(organism* org,FILE* f);
exp* genexp(program* prog,int type);
cond* gencond(program* prog,int type);
treenode* genprog(int depth,program* prog);
int* getPublicVarUsed(treenode* prog,int num);
treenode* genCS(int depth,program* prog,int type);
void addCS(program* prog);

void setNumOfStatements(treenode* root);
void setLinesTreenode(treenode* t,int depth);
int setFixed(treenode* t);
int getFixed(treenode* t);
void setAll(program* prog);

exp* copyExp(exp* e);
cond* copyCond(cond* c);
treenode* copyTreenode(treenode* t);
program* copyProgram(program* prog);

program* genProgram(program* templat,int progid);
organism* genOrganism(program* template);
program** genInitTemplate(int num);

void freeOrganism(organism* org);
void freeProgram(program* prog);
void freeTreenode(treenode* t);
void freeCondition(cond* c);
void freeExpression(exp* e);
#endif

