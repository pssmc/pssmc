#ifndef __MUTATION_H_
#define __MUTATION_H_

#include "trace.h"

typedef struct mutationNode
{
	treenode* node;
	cond* cond;
	exp* exp;
	int ranksum;
	int type;
}mutationNode;

int satisfyMutationReduction(treenode* t);
void mutationCond(cond* root,program* prog, int type);
//void mutationExp(exp* root,program* prog);
program* mutation(program* parent);

program** genNewCandidate(int numofcandidate,program** candidate,int numofmutation);
program** genNewCandidateWithCoefficient1(int numofcandidate,program** candidate,int numofmutation,double coef);
program** genNewCandidateWithCoefficient2(int numofcandidate,program** candidate,int numofmutation,double coef);

program** selectNewCandidate(int numofcandidate,program** candidate,int numofmutation, program** newcandidate);
program** selectNewCandidateWithFitness(int numofcandidate,program** candidate,int numofmutation, program** newcandidate);

#endif
