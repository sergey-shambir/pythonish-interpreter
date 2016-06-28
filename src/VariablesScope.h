#pragma once

#include <unordered_map>
#include <boost/optional.hpp>
#include "Value.h"

class CVariablesScope
{
public:
    CVariablesScope();
    ~CVariablesScope();

    bool HasVariable(unsigned nameId)const;
    void AssignVariable(unsigned nameId, const CValue &value);
    boost::optional<CValue> GetVariableValue(unsigned nameId)const;

private:
    std::unordered_map<unsigned, CValue> m_variables;
};
