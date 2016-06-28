#pragma once

#include <memory>
#include <vector>
#include "Value.h"

class CInterpreterContext;

class IExpressionAST;
class IStatementAST;
class IFunctionAST;

using IExpressionASTUniquePtr = std::unique_ptr<IExpressionAST>;
using IStatementASTUniquePtr = std::unique_ptr<IStatementAST>;
using IFunctionASTUniquePtr = std::unique_ptr<IFunctionAST>;
using ExpressionList = std::vector<IExpressionASTUniquePtr>;
using StatementsList = std::vector<IStatementASTUniquePtr>;
using FunctionList = std::vector<IFunctionASTUniquePtr>;

struct SErrorValue {};

class IExpressionAST
{
public:
    virtual ~IExpressionAST() = default;
    virtual CValue Evaluate(CInterpreterContext & context)const = 0;
};

class IStatementAST
{
public:
    virtual ~IStatementAST() = default;
    virtual void Execute(CInterpreterContext & context)const = 0;
};

class IFunctionAST
{
public:
    virtual ~IFunctionAST() = default;
    virtual CValue Call(CInterpreterContext & context, std::vector<CValue> const& arguments)const = 0;
    virtual unsigned GetNameId()const = 0;
};

enum class BinaryOperation
{
    Less,
    Equals,
    And,
    Or,
    Add,
    Substract,
    Multiply,
    Divide,
    Modulo,
};

class CBinaryExpressionAST : public IExpressionAST
{
public:
    CBinaryExpressionAST(IExpressionASTUniquePtr && left, BinaryOperation op, IExpressionASTUniquePtr && right);
    CValue Evaluate(CInterpreterContext & context)const override;

private:
    IExpressionASTUniquePtr m_left;
    const BinaryOperation m_operation;
    IExpressionASTUniquePtr m_right;
};

enum class UnaryOperation
{
    Plus,
    Minus,
    Not,
};

class CUnaryExpressionAST : public IExpressionAST
{
public:
    CUnaryExpressionAST(UnaryOperation op, IExpressionASTUniquePtr && value);
    CValue Evaluate(CInterpreterContext & context)const override;

private:
    const UnaryOperation m_operation;
    IExpressionASTUniquePtr m_expr;
};

class CLiteralAST : public IExpressionAST
{
public:
    CLiteralAST(CValue value);
    CValue Evaluate(CInterpreterContext & context)const override;

private:
    const CValue m_value;
};

class CCallAST : public IExpressionAST
{
public:
    CCallAST(unsigned nameId, ExpressionList &&arguments);
    CValue Evaluate(CInterpreterContext & context)const override;

private:
    const unsigned m_nameId;
    ExpressionList m_arguments;
};

class CVariableRefAST : public IExpressionAST
{
public:
    CVariableRefAST(unsigned nameId);
    CValue Evaluate(CInterpreterContext & context)const override;

private:
    const unsigned m_nameId;
};

class CPrintAST : public IStatementAST
{
public:
    CPrintAST(ExpressionList && expressions);

protected:
    void Execute(CInterpreterContext & context)const override;

private:
    ExpressionList m_expressions;
};

class CAssignAST : public IStatementAST
{
public:
    CAssignAST(unsigned nameId, IExpressionASTUniquePtr && value);

protected:
    void Execute(CInterpreterContext &context)const override;

private:
    const unsigned m_nameId;
    IExpressionASTUniquePtr m_value;
};

class CReturnAST : public IStatementAST
{
public:
    CReturnAST(IExpressionASTUniquePtr && value);

protected:
    void Execute(CInterpreterContext &context)const override;

private:
    IExpressionASTUniquePtr m_value;
};

class CWhileAst : public IStatementAST
{
public:
    CWhileAst(IExpressionASTUniquePtr && condition,
              StatementsList && body = StatementsList());

protected:
    void Execute(CInterpreterContext &context) const override;

private:
    IExpressionASTUniquePtr m_condition;
    StatementsList m_body;
};

class CRepeatAst : public IStatementAST
{
public:
    CRepeatAst(IExpressionASTUniquePtr && condition,
               StatementsList && body = StatementsList());

protected:
    void Execute(CInterpreterContext &context) const override;

private:
    IExpressionASTUniquePtr m_condition;
    StatementsList m_body;
};

class CIfAst : public IStatementAST
{
public:
    CIfAst(IExpressionASTUniquePtr && condition,
           StatementsList && thenBody = StatementsList(),
           StatementsList && elseBody = StatementsList());

protected:
    void Execute(CInterpreterContext &context) const override;

private:
    IExpressionASTUniquePtr m_condition;
    StatementsList m_thenBody;
    StatementsList m_elseBody;
};

class CFunctionAST : public IFunctionAST
{
public:
    CFunctionAST(unsigned nameId, std::vector<unsigned> argumentNames, StatementsList && body);

    unsigned GetNameId()const override;

protected:
    CValue Call(CInterpreterContext &context, const std::vector<CValue> &arguments) const override;

private:
    unsigned m_nameId;
    std::vector<unsigned> m_argumentNames;
    StatementsList m_body;
};

class CProgramAst
{
public:
    CProgramAst(CInterpreterContext &context);
    ~CProgramAst();

    void AddStatement(IStatementASTUniquePtr && stmt);
    void AddFunction(IFunctionASTUniquePtr && function);

private:
    CInterpreterContext &m_context;
    FunctionList m_functions;
};
