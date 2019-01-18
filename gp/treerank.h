#ifndef __TREERANK_H_
#define __TREERANK_H_

#include "trace.h"

typedef struct treeRank         
{
    mutationNode** candidate;
	int numcandidate;
	int maxnumcandidate;
	int ranksum;
}treeRank;

void addNode(treeRank* tr,treenode* node);
void searchNode(treenode* root,treeRank* tr,int type,int maxdepth);
mutationNode* chooseNode(treeRank* tr);

#endif
