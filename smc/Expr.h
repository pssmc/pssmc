#ifndef __EXPR_H_
#define __EXPR_H_


typedef enum ExprType{Constant,Variable,Expr1,StepBoundExpr1,Expr2}ExprType;
typedef enum UnOp{Neg,Not,Future,Globally,Next}UnOp;
typedef enum BinOp{Imp,And,Or,Eq,Neq,Lt,Le,Gt,Ge,Add,Min,Mul,Div}BinOp;	//,Until,WeakUntil

typedef struct Expr
{
	ExprType type;
	UnOp uop;
	BinOp bop;

	int stepbound;
	char* name;	
	int value;

	struct Expr* child;
	struct Expr* left;
	struct Expr* right;
}Expr;

Expr* createExprConstant(int value);
Expr* createExprVar(char* name);
Expr* createExprExp1(UnOp op,Expr* child);
Expr* createExprStepb(UnOp op,Expr* child,int stepbound);
Expr* createExprExpr2(BinOp op,Expr* left,Expr* right);

Expr* generateStepExpr(char* buf, int leftindex, int rightindex);

int getStepBound(Expr* exp);
#endif
