#ifndef __STATE_H_
#define __STATE_H_

#include "Expr.h"

typedef struct State{
	int numvar;
	int *varvalue;
	char**varname;
}State;

int getVarindexFromState(State* st,char* name);
int getVarvalueFromState(State* st,char* name);

#endif
