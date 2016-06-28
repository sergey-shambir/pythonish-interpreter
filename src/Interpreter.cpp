#include "Interpreter.h"
#include "StringPool.h"
#include "InterpreterContext.h"
#include "Parser.h"
#include "Lexer.h"
#include "Grammar.h"


using std::bind;
using namespace std::placeholders;


class CInterpreter::Impl
{
public:
    Impl(std::ostream &output, std::ostream &errors)
        : m_context(output, errors, m_stringPool)
        , m_parser(m_context)
    {
    }

    bool ConsumeLine(std::string const& line)
    {
        ++m_lineNo;
        auto errorHandler = bind(&CInterpreterContext::PrintError, std::ref(m_context), _1);
        CLexer lexer(m_lineNo, line, m_stringPool, errorHandler);
        SToken token;
        for (int tokenId = lexer.Scan(token); tokenId != 0; tokenId = lexer.Scan(token))
        {
            if (!m_parser.Advance(tokenId, token))
            {
                return false;
            }
        }
        token.line = m_lineNo;
        token.column = 1;
        return m_parser.Advance(TK_NEWLINE, token);
    }

    void StartDebugTrace()
    {
#ifndef NDEBUG
        m_parser.StartDebugTrace(stderr);
#endif
    }

private:
    CStringPool m_stringPool;
    CInterpreterContext m_context;
    CParser m_parser;
    unsigned m_lineNo = 0;
};

CInterpreter::CInterpreter(std::ostream &output, std::ostream &errors)
    : m_pImpl(new Impl(output, errors))
{
}

CInterpreter::~CInterpreter()
{
}

void CInterpreter::StartDebugTrace()
{
    m_pImpl->StartDebugTrace();
}

void CInterpreter::EnterLoop(std::istream &input)
{
    std::string line;
    while (std::getline(input, line))
    {
        if (!m_pImpl->ConsumeLine(line))
        {
            return;
        }
    }
}
