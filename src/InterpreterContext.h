#pragma once

#include <unordered_map>
#include <memory>
#include <boost/optional.hpp>
#include "Value.h"

class CStringPool;
class CVariablesScope;
class IFunctionAST;

class CInterpreterContext
{
public:
    CInterpreterContext(std::ostream &output, std::ostream &errors, CStringPool & pool);
    ~CInterpreterContext();

    void DefineVariable(unsigned nameId, const CValue &value);
    void AssignVariable(unsigned nameId, const CValue &value);
    CValue GetVariableValue(unsigned nameId)const;

    void PushScope(std::unique_ptr<CVariablesScope> && scope);
    std::unique_ptr<CVariablesScope> PopScope();
    size_t GetScopesCount()const;

    IFunctionAST *GetFunction(unsigned nameId)const;
    void AddFunction(unsigned nameId, IFunctionAST *function);

    std::string GetStringLiteral(unsigned stringId)const;

    void SetReturnValue(const boost::optional<CValue> &valueOpt);
    boost::optional<CValue> GetReturnValue()const;

    void PrintResults(const std::vector<CValue> &values);
    void PrintError(std::string const& message);

private:
    CVariablesScope *FindScopeWithVariable(unsigned nameId)const;
    bool ValidateValue(const CValue &value);
    void AddBuiltin(const std::string &name, std::unique_ptr<IFunctionAST> && function);

    std::unordered_map<unsigned, IFunctionAST*> m_functions;
    CStringPool & m_pool;
    boost::optional<CValue> m_returnValueOpt;
    std::vector<std::unique_ptr<IFunctionAST>> m_builtins;
    std::vector<std::unique_ptr<CVariablesScope>> m_scopes;
    std::ostream &m_output;
    std::ostream &m_errors;
};
