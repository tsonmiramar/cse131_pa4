/* File: ast_expr.h
 * ----------------
 * The Expr class and its subclasses are used to represent
 * expressions in the parse tree.  For each expression in the
 * language (add, call, New, etc.) there is a corresponding
 * node class for that construct. 
 *
 * pp3: You will need to extend the Expr classes to implement
 * semantic analysis for rules pertaining to expressions.
 */


#ifndef _H_ast_expr
#define _H_ast_expr

#include "ast.h"
#include "ast_stmt.h"
#include "list.h"
#include "ast_type.h"

void yyerror(const char *msg);

class Expr : public Stmt 
{
  public:
    Expr(yyltype loc) : Stmt(loc) {}
    Expr() : Stmt() {}
    Type* type;
    llvm::Value* llvm_val;

    friend std::ostream& operator<< (std::ostream& stream, Expr * expr) {
        return stream << expr->GetPrintNameForNode();
    }

    virtual void Emit() {}
};

class ExprError : public Expr
{
  public:
    ExprError() : Expr() { yyerror(this->GetPrintNameForNode()); }
    const char *GetPrintNameForNode() { return "ExprError"; }
};

/* This node type is used for those places where an expression is optional.
 * We could use a NULL pointer, but then it adds a lot of checking for
 * NULL. By using a valid, but no-op, node, we save that trouble */
class EmptyExpr : public Expr
{
  public:
    const char *GetPrintNameForNode() { return "Empty"; }
};

class IntConstant : public Expr 
{
  protected:
    int value;
  
  public:
    IntConstant(yyltype loc, int val);
    const char *GetPrintNameForNode() { return "IntConstant"; }
    void PrintChildren(int indentLevel);
    virtual void Emit();
};

class FloatConstant: public Expr 
{
  protected:
    double value;
    
  public:
    FloatConstant(yyltype loc, double val);
    const char *GetPrintNameForNode() { return "FloatConstant"; }
    void PrintChildren(int indentLevel);
    virtual void Emit();
};

class BoolConstant : public Expr 
{
  protected:
    bool value;
    
  public:
    BoolConstant(yyltype loc, bool val);
    const char *GetPrintNameForNode() { return "BoolConstant"; }
    void PrintChildren(int indentLevel);
    virtual void Emit();
};

class VarExpr : public Expr
{
  protected:
    Identifier *id;

  public:
    VarExpr(yyltype loc, Identifier *id);
    const char *GetPrintNameForNode() { return "VarExpr"; }
    void PrintChildren(int indentLevel);
    Identifier *GetIdentifier() {return id;}
    void GetType();
    virtual void Emit();
};

class Operator : public Node 
{
  protected:
    char tokenString[4];
    
  public:
    Operator(yyltype loc, const char *tok);
    const char *GetPrintNameForNode() { return "Operator"; }
    void PrintChildren(int indentLevel);
    friend ostream& operator<<(ostream& out, Operator *o) { return out << o->tokenString; }
    bool IsOp(const char *op) const;
 };
 
class CompoundExpr : public Expr
{
  protected:
    Operator *op;
    Expr *left, *right; // left will be NULL if unary
    
  public:
    CompoundExpr(Expr *lhs, Operator *op, Expr *rhs); // for binary
    CompoundExpr(Operator *op, Expr *rhs);             // for unary
    CompoundExpr(Expr *lhs, Operator *op);             // for unary
    void PrintChildren(int indentLevel);
    virtual void ops_perform(const char* opsTok,bool doPrefix=false) {};
    virtual void Emit() {}
};

class ArithmeticExpr : public CompoundExpr 
{
  public:
    ArithmeticExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
    ArithmeticExpr(Operator *op, Expr *rhs) : CompoundExpr(op,rhs) {}
    const char *GetPrintNameForNode() { return "ArithmeticExpr"; }
    virtual void ops_perform(const char* opsTok, bool doPrefix=false);
    virtual void Emit();
};

class RelationalExpr : public CompoundExpr 
{
  public:
    RelationalExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
    const char *GetPrintNameForNode() { return "RelationalExpr"; }
    virtual void ops_perform(const char* opsTok,bool doPrefix=false);
    virtual void Emit();
};

class EqualityExpr : public CompoundExpr 
{
  public:
    EqualityExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
    const char *GetPrintNameForNode() { return "EqualityExpr"; }
    virtual void ops_perform(const char* opsTok,bool doPrefix=false);
    virtual void Emit();
};

class LogicalExpr : public CompoundExpr 
{
  public:
    LogicalExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
    LogicalExpr(Operator *op, Expr *rhs) : CompoundExpr(op,rhs) {}
    const char *GetPrintNameForNode() { return "LogicalExpr"; }
    virtual void ops_perform(const char* opsTok,bool doPrefix=false);
    virtual void Emit();
};

class AssignExpr : public CompoundExpr 
{
  public:
    AssignExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
    const char *GetPrintNameForNode() { return "AssignExpr"; }
    virtual void ops_perform(const char* opsTok,bool doPrefix=false);
    virtual void Emit();
};

class PostfixExpr : public CompoundExpr
{
  public:
    PostfixExpr(Expr *lhs, Operator *op) : CompoundExpr(lhs,op) { doPrefix = false;}
    const char *GetPrintNameForNode() { return "PostfixExpr"; }
    virtual void ops_perform(const char* opsTok,bool doPrefix=false);
    bool doPrefix;
    virtual void Emit();
};

class ConditionalExpr : public Expr
{
  protected:
    Expr *cond, *trueExpr, *falseExpr;
  public:
    ConditionalExpr(Expr *c, Expr *t, Expr *f);
    void PrintChildren(int indentLevel);
    const char *GetPrintNameForNode() { return "ConditionalExpr"; }
};

class LValue : public Expr 
{
  public:
    LValue(yyltype loc) : Expr(loc) {}
};

class ArrayAccess : public LValue 
{
  protected:
    Expr *base, *subscript;
    
  public:
    ArrayAccess(yyltype loc, Expr *base, Expr *subscript);
    const char *GetPrintNameForNode() { return "ArrayAccess"; }
    void PrintChildren(int indentLevel);
    void GetType();
    llvm::Value* getElemPtrInst;
    virtual void Emit();
};

/* Note that field access is used both for qualified names
 * base.field and just field without qualification. We don't
 * know for sure whether there is an implicit "this." in
 * front until later on, so we use one node type for either
 * and sort it out later. */
class FieldAccess : public LValue 
{
  protected:
    Expr *base;	// will be NULL if no explicit base
    Identifier *field;
  public:
    FieldAccess(Expr *base, Identifier *field); //ok to pass NULL base
    const char *GetPrintNameForNode() { return "FieldAccess"; }
    void PrintChildren(int indentLevel);
    llvm::Value* GetllvmField();
    llvm::Value* InsertllvmElems(llvm::Value* insertVal);
    Expr* GetBase() { return base; }
    static llvm::Value* InsertWithUndef(Type* newType, llvm::Value* insertVal);
    Identifier* GetField() { return field; };
    void GetType();
    virtual void Emit();
};

/* Like field access, call is used both for qualified base.field()
 * and unqualified field().  We won't figure out until later
 * whether we need implicit "this." so we use one node type for either
 * and sort it out later. */
class Call : public Expr 
{
  protected:
    Expr *base;	// will be NULL if no explicit base
    Identifier *field;
    List<Expr*> *actuals;
    
  public:
    Call() : Expr(), base(NULL), field(NULL), actuals(NULL) {}
    Call(yyltype loc, Expr *base, Identifier *field, List<Expr*> *args);
    const char *GetPrintNameForNode() { return "Call"; }
    void PrintChildren(int indentLevel);
    virtual void Emit();
};

class ActualsError : public Call
{
  public:
    ActualsError() : Call() { yyerror(this->GetPrintNameForNode()); }
    const char *GetPrintNameForNode() { return "ActualsError"; }
};

#endif
