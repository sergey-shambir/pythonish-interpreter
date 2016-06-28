#include "Parser.h"
#include "Lexer.h"
#include "StringPool.h"
#include "Interpreter.h"
#include "Token.h"
#include "Grammar.h"
#include <iostream>
#include <time.h>

int main()
{
    std::srand(static_cast<unsigned>(time(nullptr)));
    CInterpreter interpreter(std::cout, std::cerr);

    interpreter.EnterLoop(std::cin);

    return 0;
}
