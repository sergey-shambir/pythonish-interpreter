#include "AST.h"
#include "InterpreterContext.h"
#include "StringPool.h"
#include "VariablesScope.h"
#include <iostream>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/algorithm.hpp>
#include <cmath>

namespace
{

template <class TFunction>
CValue ExecuteSafely(TFunction && fn)
{
    try
    {
        return fn();
    }
    catch (std::exception const&)
    {
        return CValue::FromError(std::current_exception());
    }
}

class CSinFunction : public IFunctionAST
{
public:
    CValue Call(CInterpreterContext &context, const std::vector<CValue> &arguments) const override
    {
        (void)context;
        return ExecuteSafely([&] {
            double radians = arguments.at(0).AsDouble();
            return CValue::FromDouble(std::sin(radians));
        });
    }

    unsigned GetNameId() const override
    {
        return 0;
    }
};

class CRandFunction : public IFunctionAST
{
public:
    CValue Call(CInterpreterContext &context, const std::vector<CValue> &arguments) const override
    {
        (void)context;
        return ExecuteSafely([&] {
            double minimum = arguments.at(0).AsDouble();
            double maximum = arguments.at(1).AsDouble();
            if (minimum > maximum)
            {
                return CValue::FromErrorMessage("invalid arguments for rand - range maximum is lesser than minimum.");
            }
            double rand0to1 = double(std::rand()) / std::numeric_limits<unsigned>::max();

            return CValue::FromDouble((maximum - minimum) * rand0to1 + minimum);
        });
    }

    unsigned GetNameId() const override
    {
        return 0;
    }
};

}

CInterpreterContext::CInterpreterContext(std::ostream &output, std::ostream &errors, CStringPool &pool)
    : m_pool(pool)
    , m_output(output)
    , m_errors(errors)
{
    AddBuiltin("sin", std::unique_ptr<IFunctionAST>(new CSinFunction));
    AddBuiltin("rand", std::unique_ptr<IFunctionAST>(new CRandFunction));
}

CInterpreterContext::~CInterpreterContext()
{
}

void CInterpreterContext::DefineVariable(unsigned nameId, const CValue &value)
{
    if (ValidateValue(value))
    {
        m_scopes.back()->AssignVariable(nameId, value);
    }
}

void CInterpreterContext::AssignVariable(unsigned nameId, const CValue &value)
{
    if (ValidateValue(value))
    {
        if (CVariablesScope *pScope = FindScopeWithVariable(nameId))
        {
            pScope->AssignVariable(nameId, value);
        }
        else
        {
            DefineVariable(nameId, value);
        }
    }
}

CValue CInterpreterContext::GetVariableValue(unsigned nameId) const
{
    if (CVariablesScope *pScope = FindScopeWithVariable(nameId))
    {
        return *pScope->GetVariableValue(nameId);
    }
    return CValue::FromErrorMessage("unknown variable " + m_pool.GetString(nameId));
}

void CInterpreterContext::PushScope(std::unique_ptr<CVariablesScope> &&scope)
{
    m_scopes.emplace_back(std::move(scope));
}

std::unique_ptr<CVariablesScope> CInterpreterContext::PopScope()
{
    std::unique_ptr<CVariablesScope> ret(m_scopes.back().release());
    m_scopes.pop_back();
    return ret;
}

size_t CInterpreterContext::GetScopesCount() const
{
    return m_scopes.size();
}

IFunctionAST *CInterpreterContext::GetFunction(unsigned nameId) const
{
    try
    {
        return m_functions.at(nameId);
    }
    catch (std::exception const&)
    {
        return nullptr;
    }
}

void CInterpreterContext::AddFunction(unsigned nameId, IFunctionAST *function)
{
    if (function)
    {
        m_functions[nameId] = function;
    }
}

std::string CInterpreterContext::GetStringLiteral(unsigned stringId) const
{
    return m_pool.GetString(stringId);
}

void CInterpreterContext::PrintResults(const std::vector<CValue> &values)
{
    m_output << "  ";
    for (const auto &value : values)
    {
        if (ValidateValue(value))
        {
            m_output << value.ToString();
        }
    }
    m_output << std::endl;
}

void CInterpreterContext::PrintError(const std::string &message)
{
    m_errors << "  Error: " << message << std::endl;
}

bool CInterpreterContext::ValidateValue(const CValue &value)
{
    try
    {
        value.RethrowIfException();
        return true;
    }
    catch (std::exception const& ex)
    {
        PrintError(ex.what());
        return false;
    }
}

void CInterpreterContext::SetReturnValue(boost::optional<CValue> const& valueOpt)
{
    m_returnValueOpt = valueOpt;
}

boost::optional<CValue> CInterpreterContext::GetReturnValue() const
{
    return m_returnValueOpt;
}

CVariablesScope *CInterpreterContext::FindScopeWithVariable(unsigned nameId) const
{
    auto range = boost::adaptors::reverse(m_scopes);
    auto it = boost::find_if(range, [=](const auto &pScope) {
        return pScope->HasVariable(nameId);
    });
    if (it != range.end())
    {
        return it->get();
    }
    return nullptr;
}

void CInterpreterContext::AddBuiltin(const std::string &name, std::unique_ptr<IFunctionAST> &&function)
{
    m_builtins.emplace_back(std::move(function));
    unsigned nameRand = m_pool.Insert(name);
    m_functions[nameRand] = m_builtins.back().get();
}
