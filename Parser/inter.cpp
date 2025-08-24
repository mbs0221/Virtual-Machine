#include "inter.h"

Value * Stmt::Codegen()
{
}

Value * Block::Codegen()
{
	for (Stmt *blk : block) {
		blk->Codegen();
	}
}

Value * BinaryExprAST::Codegen()
{
	lhs->Codegen();
	rhs->Codegen();
}

Value * UnaryExprAST::Codegen()
{
	rhs->Codegen();
}

Value * VariableExprAST::Codegen()
{
}

Value * AssignExprAST::Codegen()
{
	rhs->Codegen();
}

Value * ConstantExprAST::Codegen()
{
}

Value * AccessExprAST::Codegen()
{
	return nullptr;
}

Value * MemberExpr::Codegen()
{
	return nullptr;
}

Value * CallExprAST::Codegen()
{
	vector<ExprAST*>::iterator iter;
	for (ExprAST* expr : args) {
		expr->Codegen();
	}
	// std::reverse(args.begin(), args.end());  // 暂时注释掉
	for (ExprAST* expr : args) {

	}
}

Value * Decl::Codegen()
{
}

Value * IfElse::Codegen()
{
	cond->Codegen();
	body_t->Codegen();
	if (body_f) {
		body_f->Codegen();
	}
}

Value * WhileDo::Codegen()
{
	cond->Codegen();
	body->Codegen();
}

Value * DoWhile::Codegen()
{
	body->Codegen();
	cond->Codegen();
}

Value * For::Codegen()
{
	init->Codegen();
	cond->Codegen();
	body->Codegen();
	step->Codegen();
}

Value * Switch::Codegen()
{
	expr->Codegen();
	for (Case cs : cases) {
		cs.second->Codegen();
	}
}

Value * Break::Codegen()
{
}

Value * Continue::Codegen()
{
}

Value * Throw::Codegen()
{
}

Value * TryCatch::Codegen()
{
	pTry->Codegen();
	pCatch->Codegen();
	pFinally->Codegen();
}

Value * FunctionAST::Codegen()
{
}

Value * PrototypeAST::Codegen()
{
}

Value * ParameterAST::Codegen()
{
	return nullptr;
}
