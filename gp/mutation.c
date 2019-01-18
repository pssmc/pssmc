#include "treenode.h"
#include "trace.h"
#include "mutation.h"
#include "treerank.h"
#include <stdio.h>
#include <stdlib.h>

extern int numConditionType;
sema* copySema(sema* s)
{
	if(s == NULL)
		return NULL;
	sema* result = (sema*)malloc(sizeof(sema));
	result->index = s->index;
	return result;
}

exp* copyExp(exp* e)
{
	if(e == NULL)
		return NULL;
	exp* result = (exp*)malloc(sizeof(exp));
	result->type = e->type;
	result->index = e->index;
	return result;
}

cond* copyCond(cond* c)
{
	if(c == NULL)
		return NULL;
	cond* result = (cond*)malloc(sizeof(cond));
	result->type = c->type;
	result->exp1 = copyExp(c->exp1);
	result->exp2 = copyExp(c->exp2);
	return result;
}

treenode* copyTreenode(treenode* t)
{
	if(t == NULL)
		return NULL;
	treenode* result = (treenode*)malloc(sizeof(treenode));
	result->type = t->type;
	result->index = t->index;
	result->cond1 = copyCond(t->cond1);		
	result->treenode1 = copyTreenode(t->treenode1);
	result->treenode2 = copyTreenode(t->treenode2);
	result->exp1 = copyExp(t->exp1);
	result->goodexamples = t->goodexamples;
	result->badexamples = t->badexamples;
	result->depth = t->depth;
	result->height = t->height;
	result->fixed = t->fixed;
	result->numofstatements = t->numofstatements;		//!!
	setParent(result);
	setNext(result);
	return result;
}

program* copyProgram(program* prog)
{
	program* result = (program*)malloc(sizeof(program));
	result->root = copyTreenode(prog->root);
	result->maxdepth = prog->maxdepth;
	result->progid = prog->progid;
	result->checkedBySpin = prog->checkedBySpin;
	int i;
	for(i = 0;i < 7;i++)
	result->propertyfit[i] = prog->propertyfit[i];
	return result;
}


int satisfyMutationReduction(treenode* t)		//!!!
{
	if(t->treenode1 == NULL && t->treenode2 == NULL)
		return 0;
	if(t->type == 0)
	{
		if(getFixed(t->treenode1) == 1 || getFixed(t->treenode2) == 1)
			printf("if child fixed error!!\n");
		return 1;
	}
	else if(t->type == 1)
	{
		if(t->fixed == 0 && t->treenode1 != NULL)
			return 1;
	}
	else if(t->type == 2)
	{	
		if(getFixed(t->treenode1) == 0 && getFixed(t->treenode2) == 0)	//
			return 1;
	}
	return 0;
}


void mutationCond(cond* root,program* prog,int type)	//type == 1:can add		type == 0:can't add
{	
	if(root == NULL)
		return;	
	if(root->type == -1)
		return;
	if(root->type == 0)		
	{	
		cond* new = gencond(prog,2);
		while(new->type == 0)
		{
			free(new);
			new = gencond(prog,2);
		}	
		root->type = new->type;
		root->exp1 = new->exp1;
		root->exp2 = new->exp2;
		root->cond1 = new->cond1;
		root->cond2 = new->cond2;
		free(new);
	}
	else if(root->type == 1 || root->type == 2)	
	{	
		int t = nextrand(4 + type);//printf("a%d",t);
		if(t == 0)		//change left 
			root->exp1 = genexp(prog,1);
		else if(t == 1)		//change right
			root->exp2 = genexp(prog,0);
		else if(t == 2)		//change == / !=
			root->type = 3 - root->type;
		else if(t == 3)				//change entirely
		{	
			free(root->exp1);
			free(root->exp2);
			free(root->cond1);
			free(root->cond2);
			
			cond* new;
			if(type == 1)
				new = gencond(prog,4);
			else
				new = gencond(prog,2);
				
			root->type = new->type;
			root->exp1 = new->exp1;
			root->exp2 = new->exp2;
			root->cond1 = new->cond1;
			root->cond2 = new->cond2;	
			free(new);		
		}
		else 		//change to &&/||
		{
			cond* c1 = copyCond(root);
			free(root->exp1);
			root->exp1 = NULL;
			free(root->exp2);
			root->exp2 = NULL;
			root->type = nextrand(2) + 3;
			root->cond1 = c1;
			root->cond2 = gencond(prog,2);
		}
		
	}
	else if(root->type == 3 || root->type == 4)
	{
		int t = nextrand(6);//printf("b%d",t);
		if(t == 0)		//change left
			mutationCond(root->cond1,prog,0);
		else if(t == 1)	//change && / ||
			root->type = 7 - root->type;
		else if(t == 2)	//change right
			mutationCond(root->cond2,prog,0);
		else if(t == 3)	//reduction  left
		{
			root->type = root->cond1->type;
			root->exp1 = root->cond1->exp1;
			root->exp2 = root->cond1->exp2;
			free(root->cond1);
			root->cond1 = NULL;
			free(root->cond2);
			root->cond2 = NULL;
		}
		else if(t == 4)	//reduction right
		{
			root->type = root->cond2->type;
			root->exp1 = root->cond2->exp1;
			root->exp2 = root->cond2->exp2;
			free(root->cond1);
			root->cond1 = NULL;
			free(root->cond2);
			root->cond2 = NULL;		
		}
		else if(t == 5)				//change entirely
		{	
			free(root->exp1);
			free(root->exp2);
			free(root->cond1);
			free(root->cond2);
			
			cond* new = gencond(prog,4);
				
			root->type = new->type;
			root->exp1 = new->exp1;
			root->exp2 = new->exp2;
			root->cond1 = new->cond1;
			root->cond2 = new->cond2;		
			free(new);	
		}
	}
}

treenode* getStatement(treenode* seq, int t)	//t=0:first	t=1:last
{
	if(seq == NULL || seq->type != 2)
		return seq;

	if(t == 0)
		return getStatement(seq->treenode1,0);
	else
		return getStatement(seq->treenode2,1);
}

program* mutation(program* parent)
{	
	program* newprog = copyProgram(parent);
	newprog->checkedBySpin = 0;
	treenode* new = newprog->root;

	//printf("mutationtype = %d\n",mutationtype);
	int mutationtype;
	mutationNode* chnode;
	
	do
	{
		treeRank* tr = (treeRank*)malloc(sizeof(treeRank));
		tr->candidate = (mutationNode**)malloc(sizeof(mutationNode*) * 10);
		tr->numcandidate = 0;
		tr->maxnumcandidate = 10;
		tr->ranksum = 0;
		mutationtype = nextrand(4) + 1;				//5
		searchNode(new,tr,mutationtype,newprog->maxdepth);		
		chnode = chooseNode(tr);
	}while(chnode == NULL);

	if(mutationtype == 1)							//Replacement Mutation type
	{
		treenode* mnode = chnode->node; 
		free(chnode);
		if(mnode->fixed == 0 && (mnode->cond1 == NULL && mnode->exp1 == NULL || nextrand(2) == 0))
		{
			treenode* newnode;
			newnode = genprog(mnode->depth,newprog);
	
			if(mnode->parent == NULL)	
				new = newnode;
			else
			{
				if(mnode->parent->treenode1 == mnode)
					mnode->parent->treenode1 = newnode;
				else
					mnode->parent->treenode2 = newnode;
				newnode->parent = mnode->parent;
				free(mnode);	
			}
		}
		else
		{
			if(mnode->type <= 1)
				mutationCond(mnode->cond1,newprog,1);
			else if(mnode->type == 3)
			{
				if(nextrand(2) == 0)
				{
					exp* e;
					do
					{
						e = genexp(newprog,0);
					}while(equalExp(e,mnode->exp1) == 1);		
					free(mnode->exp1);
					mnode->exp1 = e;		
					
				}
				else
				{
					mnode->index = -3 - mnode->index;	//-3 = v[p] + v[me]
				}
			}
			else if(mnode->type == 4 || mnode->type == 5)
			{
				int temp;
				do				
				{
					temp = nextrand(3) - 3;
				}while(mnode->sema1->index == temp);
				mnode->sema1->index = temp;
			}		
		}
	}
	else if(mutationtype == 2)						//Insert Mutation types
	{	
		treenode* mnode = chnode->node; 
		free(chnode);
		
		int t;
		if(mnode->depth + mnode->height == newprog->maxdepth + 1)
			t = 2;
		else if(mnode->depth == 2 && mnode->numofstatements >= 8 || mnode->depth == 3 && mnode->numofstatements >= 4)
			t = nextrand(2);
		else
			t = nextrand(3);
		treenode* newnode = createTreenode(t,0,NULL,NULL,NULL,NULL);
		
		if(t == 0 || t == 1)	//if,while
		{	
			newnode->cond1 = gencond(newprog,numConditionType);
			newnode->treenode1 = mnode;
			if(mnode->parent == NULL)
			{	//printf("first\n");
				mnode->parent = newnode;
				new = newnode;
			}
			else
			{	treenode* pp =  mnode->parent;
				//printf("second:parent type:%d\n",mnode->parent->type);
				if(mnode->parent->treenode1 == mnode)
					mnode->parent->treenode1 = newnode;
				else if(mnode->parent->treenode2 == mnode)
					mnode->parent->treenode2 = newnode;
				else
					printf("mutation type2 error!\n");
				newnode->parent = mnode->parent;
				mnode->parent = newnode;
			}
		}
		else
		{
			int p = nextrand(2);
			if(p == 0)
			{
				newnode->treenode1 = mnode;
				newnode->treenode2 = genprog(mnode->depth,newprog);
			}
			else
			{
				newnode->treenode2 = mnode;
				newnode->treenode1 = genprog(mnode->depth,newprog);
			}
			
			if(mnode->parent == NULL)
			{
				mnode->parent = newnode;
				new = newnode;
			}
			else
			{
				if(mnode->parent->treenode1 == mnode)
					mnode->parent->treenode1 = newnode;
				else
					mnode->parent->treenode2 = newnode;
				newnode->parent = mnode->parent;
				mnode->parent = newnode;
			}
		}
	}
	else if(mutationtype == 3)						//Reduction Mutation type
	{
		treenode* mnode = chnode->node;
		free(chnode);
		int child = nextrand(2);
		if(mnode->treenode1->fixed == 1 || mnode->treenode2 == NULL || child == 0 && mnode->treenode2->fixed == 0)//treenode1 replace mnode
		{
			if(mnode->parent == NULL)	//mnode == new
			{
				new = mnode->treenode1;
				new->parent = NULL;
			}
			else 						//mnode != new
			{
				mnode->treenode1->parent = mnode->parent;
				if(mnode->parent->treenode1 == mnode)
					mnode->parent->treenode1 = mnode->treenode1;
				else
					mnode->parent->treenode2 = mnode->treenode1;
			}
			free(mnode);	
		}
		else
		{
			if(mnode->parent == NULL)	//mnode == new
			{
				new = mnode->treenode2;
				new->parent = NULL;
			}
			else 						//mnode != new
			{
				mnode->treenode2->parent = mnode->parent;
				if(mnode->parent->treenode1 == mnode)
					mnode->parent->treenode1 = mnode->treenode2;
				else
					mnode->parent->treenode2 = mnode->treenode2;
			}
			free(mnode);				
		}
	}
	else if(mutationtype == 4)						//Deletion mutation type
	{	
		treenode* mnode = chnode->node; 
		free(chnode);
		if(mnode->treenode1 == NULL)
			mnode->treenode1 = genprog(mnode->depth + 1,newprog);
		else
		{
			free(mnode->treenode1);
			mnode->treenode1 = NULL;
		}	
	}
	newprog->root = new;
	return newprog;
}

program** genNewCandidate(int numofcandidate,program** candidate,int numofmutation)
{
	int* selected = (int*)malloc(sizeof(int) * numofcandidate);
	program** result = (program**)malloc(sizeof(program*) * numofmutation);
	int i;
	for(i = 0;i < numofcandidate;i++)
		selected[i] = 0;

	int count = 0;
	while(count < numofmutation)
	{
		int index;
		do
		{
			index = nextrand(numofcandidate);
		}while(selected[index] == 1);
		
		selected[index] = 1;
		count++;
	}
	
	count = 0;
	for(i = 0;i < numofcandidate;i++)
	{
		if(selected[i] == 1)
		{	
			if(candidate[i] == NULL)printf("gennewcandidate error candidate null\n");			
			result[count] = mutation(candidate[i]);//printf("after mutation\n");		
			setAll(result[count]);
			count++;
		}
	}
	free(selected);
	return result;
}


program** selectNewCandidate(int numofcandidate,program** candidate,int numofmutation, program** newcandidate)
{
	program** output = (program**)malloc(sizeof(program*) * numofcandidate);
    double* f = (double*)malloc(sizeof(double) * (numofcandidate + numofmutation));
	int *chosen1 = (int*)malloc(sizeof(int) * numofcandidate);
	int *chosen2 = (int*)malloc(sizeof(int) * numofmutation);
	int i,j;
	for(i = 0;i < numofcandidate;i++)
	{
		f[i] = candidate[i]->fitness;
		chosen1[i] = 0;
	}
	for(i = 0;i < numofmutation;i++)
	{
		f[i + numofcandidate] = newcandidate[i]->fitness;
		chosen2[i] = 0;
	}
	for(i = 0;i < numofcandidate + numofmutation - 1;i++)
		for(j = 0;j < numofcandidate + numofmutation - 1 - i;j++)
			if(f[j] < f[j + 1])
			{
				f[j] = f[j] + f[j + 1];
				f[j + 1] = f[j] - f[j + 1];
				f[j] = f[j] - f[j + 1];
			}
	double border = f[numofcandidate - 1];
	int count = 0;
	for(i = 0;i < numofcandidate;i++)
		if(candidate[i]->fitness > border + 0.0001)
		{	
			output[count] = candidate[i];	
			count++;
			chosen1[i] = 1;
		}
	for(i = 0;i < numofmutation;i++)
		if(newcandidate[i]->fitness > border + 0.0001)
		{	
			output[count] = newcandidate[i];	
			count++;
			chosen2[i] = 1;
		}
	
	for(i = 0;i < numofcandidate && count < numofcandidate;i++)
		if(candidate[i]->fitness > border - 0.0001 && candidate[i]->fitness < border + 0.0001)
		{	
			output[count] = candidate[i];	
			count++;
			chosen1[i] = 1;
		}
	for(i = 0;i < numofmutation && count < numofcandidate;i++)
		if(newcandidate[i]->fitness > border - 0.0001 && newcandidate[i]->fitness < border + 0.0001)
		{	
			output[count] = newcandidate[i];	
			count++;
			chosen2[i] = 1;
		}
	
	for(i = 0;i < numofcandidate;i++)
		if(chosen1[i] == 0)
			freeProgram(candidate[i]);
	for(i = 0;i < numofmutation;i++)
		if(chosen2[i] == 0)
			freeProgram(newcandidate[i]);
	free(f);
	free(chosen1);
	free(chosen2);
	return output;
}

program** genNewCandidateWithCoefficient1(int numofcandidate,program** candidate,int numofmutation,double coef)//no repeat
{
	int* selected = (int*)malloc(sizeof(int) * numofcandidate);
	program** result = (program**)malloc(sizeof(program*) * numofmutation);
	int i;
	for(i = 0;i < numofcandidate;i++)
		selected[i] = 0;

	int count = 0;
	while(count < numofmutation)
	{
		int index;
		do
		{
			index = nextrand(numofcandidate);
		}while(selected[index] == 1);
		
		selected[index] = 1;
		count++;
	}

	count = 0;
	for(i = 0;i < numofcandidate;i++)
	{
		if(selected[i] == 1)
		{			
			
			if(candidate[i] == NULL)printf("gennewcandidate error candidate null\n");
			
			if(nextrand(100) < coef * 100)
				result[count] = mutation(candidate[i]);
			else
				result[count] = copyProgram(candidate[i]);	

			setAll(result[count]);
			count++;
		}
	}
	free(selected);
	
	return result;
}

program** genNewCandidateWithCoefficient2(int numofcandidate,program** candidate,int numofmutation,double coef)//with repeat
{

	program** result = (program**)malloc(sizeof(program*) * numofmutation);
	int count = 0;

	while(count < numofmutation)
	{
		int index = nextrand(numofcandidate);
			
		if(nextrand(100) < coef * 100)
			result[count] = mutation(candidate[index]);
		else
			result[count] = copyProgram(candidate[index]);	

		setAll(result[count]);
		count++;
	}
	
	return result;
}

program** selectNewCandidateWithFitness(int numofcandidate,program** candidate,int numofmutation, program** newcandidate)
{
	
	program** output = (program**)malloc(sizeof(program*) * numofcandidate);
    int* f = (int*)malloc(sizeof(double) * (numofcandidate + numofmutation));
	int *chosen = (int*)malloc(sizeof(int) * (numofcandidate + numofmutation));

	int addfitness = 0;
	int i,j;
	for(i = 0;i < numofcandidate;i++)
	{
		f[i] = candidate[i]->fitness + 40;
		chosen[i] = 0;
		addfitness += f[i];
	}
	for(i = 0;i < numofmutation;i++)
	{
		f[i + numofcandidate] = newcandidate[i]->fitness + 40;
		chosen[i + numofcandidate] = 0;
		addfitness += f[i + numofcandidate];
	}

	int* roulette = (int*)malloc(sizeof(int) * addfitness);
	int tempfit = 0;
	for(i = 0;i < numofcandidate;i++)
		for(j = 0;j < f[i];j++)
		{
			roulette[tempfit++] = i;
		}
	for(i = 0;i < numofmutation;i++)
	{
		for(j = 0;j < f[i + numofcandidate];j++)
		{
			roulette[tempfit++] = i + numofcandidate;
		}
	}

	if(tempfit != addfitness)
		printf("ERROR:TEMPFIT%d != ADDFITESS%d\n",tempfit,addfitness);

	int count = 0;
	while(count < numofcandidate)
	{
		int p = nextrand(addfitness);
		if(chosen[roulette[p]] == 0)
		{
			chosen[roulette[p]] = 1;
			count++;
		}
	}
	
	count = 0;
	for(i = 0;i < numofcandidate;i++)
	{
		if(chosen[i] == 1)
		{
			output[count] = candidate[i];
			count++;
		}
		else
			freeProgram(candidate[i]);	
	}
	for(i = 0;i < numofmutation;i++)
	{
		if(chosen[i + numofcandidate] == 1)
		{
			output[count] = newcandidate[i];
			count++;
		}
		else
			freeProgram(newcandidate[i]);
	}	

	free(f);
	free(chosen);
	free(roulette);
	
	return output;
}


