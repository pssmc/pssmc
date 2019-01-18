#include <stdio.h>
#include <stdlib.h>
#include "treerank.h"

extern int* maxNumStatements;

void addNode(treeRank* tr,treenode* node)
{
	if(node == NULL)
		return;

	if(tr->numcandidate == tr->maxnumcandidate)
	{
		tr->maxnumcandidate *= 2;
		mutationNode** new = (mutationNode**)malloc(sizeof(mutationNode*) * tr->maxnumcandidate);
		int i;
		for(i = 0;i < tr->numcandidate;i++)
		{
			new[i] = tr->candidate[i];
		}
		free(tr->candidate);
		tr->candidate = new;
	}

	mutationNode* p = (mutationNode*)malloc(sizeof(mutationNode));
	p->node = node;

	tr->ranksum += node->badexamples;
	p->ranksum = tr->ranksum;
	tr->candidate[tr->numcandidate] = p;
	tr->numcandidate++;
}

void searchNode(treenode* root,treeRank* tr,int type,int maxdepth)
{

	if(root == NULL)
		return;
	if(type == 1)
	{	
		if(root->fixed == 0)	
			addNode(tr,root);		
	}
	else if(type == 2)	
	{
		if(root->fixed == 0 && root->depth < maxdepth || root->numofstatements < maxNumStatements[root->depth - 1])
			addNode(tr,root);
	}
	else if(type == 3)
	{
		if(satisfyMutationReduction(root) == 1)
			addNode(tr,root);
	}
	else if(type == 4)
	{
		if(root->type == 1 || root->type == 2)
		{
			if(root->treenode1 != NULL && root->treenode1->fixed == 0 || root->treenode1 == NULL)
				addNode(tr,root);
		}
	}
	else
	{
	    if(root->depth + 2 <= maxdepth)			//!!
			addNode(tr,root);
	}

	searchNode(root->treenode1,tr,type,maxdepth);
	searchNode(root->treenode2,tr,type,maxdepth);
}	

	

mutationNode* chooseNode(treeRank* tr)
{
	if(tr->ranksum == 0)
		return NULL;
	int p = nextrand(tr->ranksum);
	int i;
	mutationNode* result;
	for(i = 0;i < tr->numcandidate;i++)
	{
		result = tr->candidate[i];
		if(result == NULL)printf("result NULL!!\n");
		if(result->ranksum > p)
			break;	
	}
	
	for(i = 0;i < tr->numcandidate;i++)
		if(tr->candidate[i] != result)
			free(tr->candidate[i]);
	free(tr);

	return result;
}


