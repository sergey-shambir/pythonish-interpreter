#pragma once

#include <cctype>
#include <string>
#include <map>
#include <functional>
#include <boost/utility/string_ref.hpp>
#include "Token.h"
#include "StringPool.h"

class CLexer
{
public:
    using ErrorHandler = std::function<void(std::string const& message)>;

    CLexer(unsigned lineNo, std::string const& line, CStringPool & pool, ErrorHandler const &handler);

    // Возвращает следующий токен (лексему) либо 0, если входной файл кончился.
    // Токены объявлены в Grammar.h
    int Scan(SToken &data);

private:
    double ParseDouble();
    std::string ParseIdentifier();
    void SkipSpaces();
    bool ParseString(SToken &data);
    int AcceptIdOrKeyword(SToken &data, std::string && id);
    void OnError(const char message[], SToken &data);

    const unsigned m_lineNo;
    const std::string m_sources;
    boost::string_ref m_peep;
    CStringPool & m_stringPool;
    ErrorHandler m_onError;
    const std::map<std::string, int> m_keywords;
};
