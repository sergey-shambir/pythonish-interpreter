#include "Lexer.h"
#include "Grammar.h"
#include <iostream>
#include <sstream>
#include <cmath>

CLexer::CLexer(unsigned lineNo, std::string const& line, CStringPool &pool, const ErrorHandler &handler)
    : m_lineNo(lineNo)
    , m_sources(line)
    , m_peep(m_sources)
    , m_stringPool(pool)
    , m_onError(handler)
    , m_keywords({
        { "do",     TK_DO },
        { "if",     TK_IF },
        { "end",    TK_END },
        { "else",   TK_ELSE },
        { "while",  TK_WHILE },
        { "print",  TK_PRINT },
        { "return", TK_RETURN },
        { "function",    TK_FUNCTION },
      })
{
}

int CLexer::Scan(SToken &data)
{
    SkipSpaces();
    data.line = m_lineNo;
    data.column = 1 + unsigned(m_peep.data() - m_sources.c_str());

    if (m_peep.empty())
    {
        return 0;
    }
    double value = ParseDouble();
    if (!std::isnan(value))
    {
        data.value = value;
        return TK_NUMBER;
    }
    if (ParseString(data))
    {
        return TK_STRING;
    }
    switch (m_peep[0])
    {
    case '<':
        m_peep.remove_prefix(1);
        return TK_LESS;
    case '+':
        m_peep.remove_prefix(1);
        return TK_PLUS;
    case '-':
        m_peep.remove_prefix(1);
        return TK_MINUS;
    case '*':
        m_peep.remove_prefix(1);
        return TK_STAR;
    case '/':
        m_peep.remove_prefix(1);
        return TK_SLASH;
    case ',':
        m_peep.remove_prefix(1);
        return TK_COMMA;
    case '%':
        m_peep.remove_prefix(1);
        return TK_PERCENT;
    case '!':
        m_peep.remove_prefix(1);
        return TK_NOT;
    case '(':
        m_peep.remove_prefix(1);
        return TK_LPAREN;
    case ')':
        m_peep.remove_prefix(1);
        return TK_RPAREN;
    case '=':
        if (m_peep.length() >= 2 && (m_peep[1] == '='))
        {
            m_peep.remove_prefix(2);
            return TK_EQUALS;
        }
        m_peep.remove_prefix(1);
        return TK_ASSIGN;
    case '&':
        if (m_peep.length() >= 2 && (m_peep[1] == '&'))
        {
            m_peep.remove_prefix(2);
            return TK_AND;
        }
        break;
    case '|':
        if (m_peep.length() >= 2 && (m_peep[1] == '|'))
        {
            m_peep.remove_prefix(2);
            return TK_OR;
        }
        break;
    }
    std::string id = ParseIdentifier();
    if (!id.empty())
    {
        return AcceptIdOrKeyword(data, std::move(id));
    }

    // on error, return EOF
    OnError("unknown lexem", data);
    return 0;
}

// returns NaN if cannot parse double.
double CLexer::ParseDouble()
{
    double value = 0;
    bool parsedAny = false;
    while (!m_peep.empty() && std::isdigit(m_peep[0]))
    {
        parsedAny = true;
        const int digit = m_peep[0] - '0';
        value = value * 10.0 + double(digit);
        m_peep.remove_prefix(1);
    }
    if (!parsedAny)
    {
        return std::numeric_limits<double>::quiet_NaN();
    }
    if (m_peep.empty() || (m_peep[0] != '.'))
    {
        return value;
    }
    m_peep.remove_prefix(1);
    double factor = 1.0;
    while (!m_peep.empty() && std::isdigit(m_peep[0]))
    {
        const int digit = m_peep[0] - '0';
        factor *= 0.1;
        value += factor * double(digit);
        m_peep.remove_prefix(1);
    }

    return value;
}

std::string CLexer::ParseIdentifier()
{
    size_t size = 0;
    while (!m_peep.empty() && std::isalnum(m_peep[size]))
    {
        ++size;
    }
    std::string value = m_peep.substr(0, size).to_string();
    m_peep.remove_prefix(size);
    return value;
}

void CLexer::SkipSpaces()
{
    size_t count = 0;
    while (count < m_peep.size() && std::isspace(m_peep[count]))
    {
        ++count;
    }
    m_peep.remove_prefix(count);
}

bool CLexer::ParseString(SToken &data)
{
    if (m_peep[0] != '\"')
    {
        return false;
    }
    m_peep.remove_prefix(1);
    size_t quotePos = m_peep.find('\"');
    if (quotePos == boost::string_ref::npos)
    {
        OnError("missed end quote", data);
        data.stringId = m_stringPool.Insert(m_peep.to_string());
        m_peep.clear();

        return true;
    }

    boost::string_ref value = m_peep.substr(0, quotePos);
    data.stringId = m_stringPool.Insert(value.to_string());
    m_peep.remove_prefix(quotePos + 1);

    return true;
}

int CLexer::AcceptIdOrKeyword(SToken &data, std::string && id)
{
    if (id == "true")
    {
        data.boolValue = true;
        return TK_BOOL;
    }
    else if (id == "false")
    {
        data.boolValue = false;
        return TK_BOOL;
    }

    auto it = m_keywords.find(id);
    if (it != m_keywords.end())
    {
        return it->second;
    }

    data.stringId = m_stringPool.Insert(id);
    return TK_ID;
}

void CLexer::OnError(const char message[], SToken &data)
{
    if (m_onError)
    {
        std::stringstream formatter;
        formatter << message << "at (" << data.line << "," << data.column << ")";
        m_onError(formatter.str());
    }
}
