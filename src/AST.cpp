#include "AST.h"
#include "VariablesScope.h"
#include "InterpreterContext.h"
#include <limits>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <cassert>

namespace
{

void ExecuteAll(StatementsList const& list, CInterpreterContext & context)
{
    for (auto const& stmt : list)
    {
        stmt->Execute(context);
    }
}

class CScopedVariableScope
{
public:
    CScopedVariableScope(CInterpreterContext &context)
        : m_context(context)
    {
        m_context.PushScope(std::make_unique<CVariablesScope>());
    }

    ~CScopedVariableScope()
    {
        m_context.PopScope();
    }

private:
    CInterpreterContext &m_context;
};

class CVariableScopeCleaner
{
public:
    CVariableScopeCleaner(CInterpreterContext &context)
        : m_context(context)
    {
        // Убираем все области видимости, кроме верхней.
        while (m_context.GetScopesCount() > 1)
        {
            m_scopes.emplace_back(m_context.PopScope());
        }
    }

    ~CVariableScopeCleaner()
    {
        for (std::unique_ptr<CVariablesScope> &pScope : m_scopes)
        {
            m_context.PushScope(std::unique_ptr<CVariablesScope>(pScope.release()));
        }
    }

private:
    CInterpreterContext &m_context;
    std::vector<std::unique_ptr<CVariablesScope>> m_scopes;
};

}

CPrintAST::CPrintAST(ExpressionList &&expressions)
    : m_expressions(std::move(expressions))
{
}

void CPrintAST::Execute(CInterpreterContext &context)const
{
    std::vector<CValue> values;
    for (const auto &pExpr : m_expressions)
    {
        values.push_back(pExpr->Evaluate(context));
    }
    context.PrintResults(values);
}

CAssignAST::CAssignAST(unsigned nameId, IExpressionASTUniquePtr &&value)
    : m_nameId(nameId)
    , m_value(std::move(value))
{
}

void CAssignAST::Execute(CInterpreterContext &context)const
{
    const CValue value = m_value->Evaluate(context);
    context.AssignVariable(m_nameId, value);
}

CBinaryExpressionAST::CBinaryExpressionAST(IExpressionASTUniquePtr &&left, BinaryOperation op, IExpressionASTUniquePtr &&right)
    : m_left(std::move(left))
    , m_operation(op)
    , m_right(std::move(right))
{
}

CValue CBinaryExpressionAST::Evaluate(CInterpreterContext &context) const
{
    const CValue a = m_left->Evaluate(context);
    const CValue b = m_right->Evaluate(context);
    switch (m_operation)
    {
    case BinaryOperation::Less:
        return a < b;
    case BinaryOperation::Equals:
        return a == b;
    case BinaryOperation::And:
        return a && b;
    case BinaryOperation::Or:
        return a || b;
    case BinaryOperation::Add:
        return a + b;
    case BinaryOperation::Substract:
        return a - b;
    case BinaryOperation::Multiply:
        return a * b;
    case BinaryOperation::Divide:
        return a / b;
    case BinaryOperation::Modulo:
        return a % b;
    }
    return CValue::FromErrorMessage("binary operation not implemented");
}

CUnaryExpressionAST::CUnaryExpressionAST(UnaryOperation op, IExpressionASTUniquePtr &&value)
    : m_operation(op)
    , m_expr(std::move(value))
{
}

CValue CUnaryExpressionAST::Evaluate(CInterpreterContext &context) const
{
    const CValue value = m_expr->Evaluate(context);
    switch (m_operation)
    {
    case UnaryOperation::Plus:
        return +value;
    case UnaryOperation::Minus:
        return -value;
    case UnaryOperation::Not:
        return !value;
    }
    return CValue::FromErrorMessage("unary operation not implemented");
}

CLiteralAST::CLiteralAST(CValue value)
    : m_value(value)
{
}

CValue CLiteralAST::Evaluate(CInterpreterContext &context) const
{
    (void)context;
    return m_value;
}

CVariableRefAST::CVariableRefAST(unsigned nameId)
    : m_nameId(nameId)
{
}

CValue CVariableRefAST::Evaluate(CInterpreterContext &context) const
{
    return context.GetVariableValue(m_nameId);
}

CIfAst::CIfAst(IExpressionASTUniquePtr &&condition, StatementsList &&thenBody, StatementsList &&elseBody)
    : m_condition(std::move(condition))
    , m_thenBody(std::move(thenBody))
    , m_elseBody(std::move(elseBody))
{
}

void CIfAst::Execute(CInterpreterContext &context) const
{
    CValue result = m_condition->Evaluate(context);
    if (bool(result))
    {
        ExecuteAll(m_thenBody, context);
    }
    else
    {
        ExecuteAll(m_elseBody, context);
    }
}

CProgramAst::CProgramAst(CInterpreterContext &context)
    : m_context(context)
{
    m_context.PushScope(std::make_unique<CVariablesScope>());
}

CProgramAst::~CProgramAst()
{
    m_context.PopScope();
}

void CProgramAst::AddStatement(IStatementASTUniquePtr &&stmt)
{
    stmt->Execute(m_context);
}

void CProgramAst::AddFunction(IFunctionASTUniquePtr &&function)
{
    unsigned nameId = function->GetNameId();
    m_context.AddFunction(nameId, function.get());
    m_functions.emplace_back(std::move(function));
}

CWhileAst::CWhileAst(IExpressionASTUniquePtr &&condition, StatementsList &&body)
    : m_condition(std::move(condition))
    , m_body(std::move(body))
{
}

void CWhileAst::Execute(CInterpreterContext &context) const
{
    while (bool(m_condition->Evaluate(context)))
    {
        ExecuteAll(m_body, context);
    }
}

CRepeatAst::CRepeatAst(IExpressionASTUniquePtr &&condition, StatementsList &&body)
    : m_condition(std::move(condition))
    , m_body(std::move(body))
{
}

void CRepeatAst::Execute(CInterpreterContext &context) const
{
    do
    {
        ExecuteAll(m_body, context);
    }
    while (bool(m_condition->Evaluate(context)));
}

CCallAST::CCallAST(unsigned nameId, ExpressionList && arguments)
    : m_nameId(nameId)
    , m_arguments(std::move(arguments))
{
}

CValue CCallAST::Evaluate(CInterpreterContext &context) const
{
    if (IFunctionAST *func = context.GetFunction(m_nameId))
    {
        std::vector<CValue> args(m_arguments.size());
        std::transform(m_arguments.begin(), m_arguments.end(), args.begin(), [&](IExpressionASTUniquePtr const& ast) {
            return ast->Evaluate(context);
        });
        return func->Call(context, args);
    }
    return CValue::FromErrorMessage("Attempt to call unknown function.");
}

CFunctionAST::CFunctionAST(unsigned nameId, std::vector<unsigned> argumentNames, StatementsList && body)
    : m_nameId(nameId)
    , m_argumentNames(argumentNames)
    , m_body(std::move(body))
{
}

unsigned CFunctionAST::GetNameId() const
{
    return m_nameId;
}

CValue CFunctionAST::Call(CInterpreterContext &context, const std::vector<CValue> &arguments) const
{
    if (arguments.size() != m_argumentNames.size())
    {
        return CValue::FromErrorMessage("arguments and parameters count mismatch");
    }

    CVariableScopeCleaner cleaner(context);
    CScopedVariableScope scopedScope(context);

    auto argumentIt = arguments.begin();
    for (unsigned nameId : m_argumentNames)
    {
        context.DefineVariable(nameId, *argumentIt);
        ++argumentIt;
    }

    boost::optional<CValue> returnedValue;
    for (IStatementASTUniquePtr const& stmt : m_body)
    {
        stmt->Execute(context);
        returnedValue = context.GetReturnValue();
        if (returnedValue)
        {
            context.SetReturnValue(boost::none);
            break;
        }
    }

    if (returnedValue.is_initialized())
    {
        return *returnedValue;
    }

    return CValue::FromErrorMessage("Function returned no value");
}

CReturnAST::CReturnAST(IExpressionASTUniquePtr &&value)
    : m_value(std::move(value))
{
}

void CReturnAST::Execute(CInterpreterContext &context) const
{
    CValue result = m_value->Evaluate(context);
    context.SetReturnValue(result);
}
