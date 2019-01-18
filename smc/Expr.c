#include "Expr.h"
#include "../gp/trace.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Expr* createExpr(ExprType t,UnOp u,BinOp b,int bound,char* n,int v,Expr* child,Expr* left,Expr* right)
{
	Expr* result = (Expr*)malloc(sizeof(Expr));
	result->type = t;
	result->uop = u;
	result->bop = b;

	result->stepbound = bound;
	//strcpy(result->name,n);	
	result->name = n;	
	result->value = v;

	result->child = child;
	result->left = left;
	result->right = right;
	
	return result;
}

Expr* createExprConstant(int value){
	return createExpr(Constant,0,0,0,NULL,value,NULL,NULL,NULL);
}

Expr* createExprVar(char* name){
	return createExpr(Variable,0,0,0,name,0,NULL,NULL,NULL);
}

Expr* createExprExp1(UnOp op,Expr* child){
	return createExpr(Expr1,op,0,0,NULL,0,child,NULL,NULL);
}

Expr* createExprStepb(UnOp op,Expr* child,int stepbound){
	return createExpr(StepBoundExpr1,op,0,stepbound,NULL,0,child,NULL,NULL);
}

Expr* createExprExpr2(BinOp op,Expr* left,Expr* right){
	return createExpr(Expr2,0,op,0,NULL,0,NULL,left,right);
}

BinOp getBinOp(char* buf,int index)
{
	BinOp op;
	switch(buf[index])
	{
		case '-':if(buf[index + 1] == '>')
				 	op = Imp;
				 else
					op = Min;
				break;
		case '&':op = And;break;
		case '|':op = Or;break;
		case '=':op = Eq;break;
		case '!':if(buf[index + 1] == '=')
				 {
					op = Neq;
					break;
				 }
				 else
				 {
					printf("Error:property BinOp !\n");
					exit(-1);
				 }
		case '<':if(buf[index + 1] == '=')
				 	op = Le;
				 else
					op = Lt;
				break;
		case '>':if(buf[index + 1] == '=')
				 	op = Ge;
				 else
					op = Gt;
				break;
		case '+':op = Add;break;
		case '*':op = Mul;break;
		default:printf("Error:property BinOp\n");
				exit(-1);
	}	
	return op;
}

int getBinOpLength(BinOp op)
{
	if(op == Neq || op == Le || op == Ge || op == Imp)
		return 2;
	else
		return 1;
}

Expr* generateStepExpr(char* buf, int leftindex, int rightindex)
{	//printf("%s,%d,%d\n",buf,leftindex,rightindex);
	if(buf[leftindex] == '(')
	{	
		int index = leftindex,count = 0;
		while(index <= rightindex)
		{
			if(buf[index] == '(')
				count++;
			else if(buf[index] == ')')
				count--;
			if(count == 0)
				break;
			index++;
		}
		if(count != 0)
		{
			printf("Error:property parentheses not match!\n");
			exit(-1);
		}

		if(index == rightindex)
			return generateStepExpr(buf, leftindex + 1, rightindex - 1);
		else
		{
			Expr* left = generateStepExpr(buf, leftindex + 1, index - 1);
			BinOp op = getBinOp(buf,index + 1);
			Expr* right = generateStepExpr(buf, index + 1 + getBinOpLength(op),rightindex);
			return createExprExpr2(op,left,right);
		}
	}
	
	if(buf[leftindex] == 'G' || buf[leftindex] == 'F' || buf[leftindex] == 'X')
	{
		UnOp op;
		if(buf[leftindex] == 'G')
			op = Globally;
		else if(buf[leftindex] == 'F')
			op = Future;
		else
			op = Next;
		int index = leftindex + 4;
		int stepbound = 0;
		while(index <= rightindex && buf[index] <= '9' && buf[index] >= '0')
		{
			stepbound *= 10;
			stepbound += buf[index++] - '0';
		}	
		
		Expr* child = generateStepExpr(buf,index,rightindex);
		Expr* result = createExprStepb(op,child,stepbound);
		
		return result;
	}
	else if(buf[leftindex] <= '9' && buf[leftindex] >= '0')
	{
		int value = 0;
		while(leftindex <= rightindex && buf[leftindex] <= '9' && buf[leftindex] >= '0')
		{
			value *= 10;
			value += buf[leftindex++] - '0';
		}
		return createExprConstant(value);

	}
	else if(buf[leftindex] <= 'z' && buf[leftindex] >= 'a')
	{
		int index = leftindex;
		while(index <= rightindex && (buf[index] <= '9' && buf[index] >= '0' || buf[index] <= 'z' && buf[index] >= 'a'))
			index++;
		char* name = (char*)malloc(sizeof(char) * (index - leftindex + 1));
		strncpy(name,&(buf[leftindex]),index - leftindex);
		name[index - leftindex] = '\0';
		Expr* left = createExprVar(name);
		if(index >= rightindex)
			return left;
		else
		{
			BinOp op = getBinOp(buf,index);
			Expr* right = generateStepExpr(buf,index + getBinOpLength(op),rightindex);
			return createExprExpr2(op,left,right);
		}
		/*while(index < rightindex)
		{
			BinOp op = getBinOp(buf,index);
			Expr* right = generateStepExpr(buf,index + getBinOpLength(op),rightindex);
			 createExprExpr2(op,left,right);
		}*/
	}
	else
	{
		printf("Error:property!\n");
		exit(-1);
	}
}

int getStepBound(Expr* exp)
{
	if(exp == NULL)
		return 0;

	int left,right;
	switch(exp->type)
	{
		case Constant:return 0;
		case Variable:return 0;
		case Expr1:return getStepBound(exp->child);
		case StepBoundExpr1:return (exp->stepbound + getStepBound(exp->child));
		case Expr2:	left = getStepBound(exp->left);
					right = getStepBound(exp->right);
					return (left>right?left:right);
	}
}

void printUnOp(UnOp op)
{
	switch(op)
	{
		case Neg:printf("~");break;
		case Not:printf("!");break;
		case Future:printf("F");break;
		case Globally:printf("G");break;
		case Next:printf("X");break;
	}
}

void printBinOp(BinOp op)
{
	switch(op)
	{
		case Imp:printf("->");break;
		case And:printf("&");break;
		case Or:printf("|");break;
		case Eq:printf("=");break;
		case Neq:printf("!=");break;
		case Lt:printf("<");break;
		case Le:printf("<=");break;
		case Gt:printf(">");break;
		case Ge:printf(">=");break;
		case Add:printf("+");break;
		case Min:printf("-");break;
		case Mul:printf("*");break;
		case Div:printf("/");break;
	}
}

void printExpr(Expr* exp)
{
	if(exp == NULL)
		return;

	switch(exp->type)
	{
		case Constant:printf("%d",exp->value);break;
		case Variable:printf("%s",exp->name);break;
		case Expr1:printUnOp(exp->uop);printExpr(exp->child);break;
		case StepBoundExpr1:printf("(");printUnOp(exp->uop);printf("<=#%d",exp->stepbound);printExpr(exp->child);printf(")");break;
		case Expr2:printf("(");printExpr(exp->left);printBinOp(exp->bop);printExpr(exp->right);printf(")");break;
	}
}

int getExprValue(Expr* exp,trace* t,int currentStep)
{
	//printf("get expr value:");
	//printExpr(exp);
	//printf("\n");
	if(exp->type == Constant)
		return exp->value;
	else if(exp->type == Variable)
		return getVarvalueFromState(t->states[currentStep],exp->name);
	else if(exp->type == Expr1)
	{
		if(exp->uop == Neg)
		{
			return (-1 * getExprValue(exp->child,t,currentStep));
		}
		else if(exp->uop == Not)
		{
			if(getExprValue(exp->child,t,currentStep) == 0)
				return 1;
			else
				return 0;
		}
		else
		{
			printf("ERROR:Expr1 uop type invalid!\n");
			exit(-1);
		}
	}
	else if(exp->type == StepBoundExpr1)
	{
		
		if(exp->uop == Future)
		{
			int i;
			for(i = currentStep;i <= currentStep + exp->stepbound;i++)			//important
				if(getExprValue(exp->child,t,i) == 1)
					return 1;
			return 0;
		}
		else if(exp->uop == Globally)
		{
			int i;
			for(i = currentStep;i <= currentStep + exp->stepbound;i++)			//important
				if(getExprValue(exp->child,t,i) == 0)
					return 0;
			return 1;
		}
		else if(exp->uop == Next)
		{
			if(getExprValue(exp->child,t,currentStep + exp->stepbound) == 1)
				return 1;
			else
				return 0;
		}
		else
		{
			printf("ERROR:StepBoundExpr1 uop type invalid!\n");
			exit(-1);
		}
	}
	else if(exp->type == Expr2)
	{
		int left = getExprValue(exp->left,t,currentStep);
		int right; 
		switch(exp->bop)
		{
			case Imp:	if(left > 0 && getExprValue(exp->right,t,currentStep) == 0)
							return  0;
						else
							return 1;
			case And:	if(left > 0 && getExprValue(exp->right,t,currentStep) > 0)
							return 1; 
						else
							return 0;
			case Or:	if(left > 0 || getExprValue(exp->right,t,currentStep) > 0)
							return  1;
						else
							return 0;
			case Eq:	if(left == getExprValue(exp->right,t,currentStep))
							return  1;
						else
							return 0;
			case Neq:	if(left != getExprValue(exp->right,t,currentStep))
							return  1;
						else
							return 0;
			case Lt:	if(left < getExprValue(exp->right,t,currentStep))
							return  1;
						else
							return 0;
			case Le:	if(left <= getExprValue(exp->right,t,currentStep))
							return  1;
						else
							return 0;
			case Gt:	if(left > getExprValue(exp->right,t,currentStep))
							return  1;
						else
							return 0;
			case Ge:	if(left >= getExprValue(exp->right,t,currentStep))
							return  1;
						else
							return 0;
			case Add:	return (left + getExprValue(exp->right,t,currentStep));
			case Min:	return (left - getExprValue(exp->right,t,currentStep));
			case Mul:	return (left * getExprValue(exp->right,t,currentStep));
			case Div:	right = getExprValue(exp->right,t,currentStep);	
						if(right != 0)
							return (left / right);
						else 
							return 0;
			default:	printf("ERROR:getExprValue Expr2 BinOp type invalid!\n");
		}
	}
	else 
	{
		printf("Error:Expr type invalid!\n");
		exit(-1);
	}
}

/*int main()
{
	/*FILE* fp = NULL;
	char buf[255];
	fp = fopen("../property/B1.bltl","r");
	fgets(buf, 255, fp);
	printf("%s\n%d\n",buf,strlen(buf));
	int index = 0;
	int i = 0;
	while(buf[i] != '\0')
	{
		if(buf[i] != ' ' && buf[i] != '\n')
		{
			buf[index] = buf[i];
			index++;
		}
		i++;
	}
	buf[index] = '\0';
	printf("%s\n%d\n",buf,strlen(buf));

	char* buf = "(G<=#200((cs0+cs1)<=1))&(X<=#5(tracetype=0))&(X<=#250((enter0>1)&(enter1>1)))";printf("%s\n%d\n",buf,strlen(buf));
	Expr* requirement = generateStepExpr(buf,0,strlen(buf) - 1);
	printExpr(requirement);
	printf("\n");
}*/

