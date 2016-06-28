#pragma once

#include <cmath>
#include <cassert>
#include <utility>
#include <memory>
#include "Token.h"
#include "Parser.h"
#include "AST.h"

namespace parser_private
{

// Создаёт новый узел AST из списка аргументов начиная со 2-го.
// Помещает указатель на узел в ячейку стека, переданную 1-м аргументом.
template <class TNode, class TRuleNode, class ...TArgs>
void EmplaceAST(TRuleNode *& stackRecord, TArgs&&... args)
{
    stackRecord = new (std::nothrow) TNode(std::forward<TArgs>(args)...);
}

// Создаёт новый узел AST из списка аргументов.
// Возвращает unique_ptr на узел.
template <class T, class ...TArgs>
std::unique_ptr<T> Make(TArgs&&... args)
{
    return std::unique_ptr<T>(new (std::nothrow) T(std::forward<TArgs>(args)...));
}

// Обнуляет указатель на узел AST в ячейке стека, возвращает его как unique_ptr.
template <class T>
std::unique_ptr<T> Take(T* & stackRecord)
{
    std::unique_ptr<T> ret(stackRecord);
    stackRecord = nullptr;
    return ret;
}

// Перемещает указатель из одной ячейки стека в другую, обнуляя исходной ячейки.
template <class T>
void MovePointer(T *& stackRecord, T *& targetRecord)
{
    targetRecord = stackRecord;
    stackRecord = nullptr;
}

// Освобождает указатель, хранящийся в ячейке стека, и обнуляет его.
template <class T>
void Destroy(T *& stackRecord)
{
    delete stackRecord;
    stackRecord = nullptr;
}

// Создаёт список указателей на узлы AST, помещает в этот список 2-й аргумент.
// Затем помещает указатель на список в ячейку стека, переданную 1-м аргументом.
template <class TTarget, class TItem>
void CreateList(TTarget *& target, TItem *& item)
{
    auto list = Make<TTarget>();
    if (item)
    {
        list->emplace_back(Take(item));
    }
    target = list.release();
};

// Извлекает список указателей на узлы AST, переданный 2-м аргументом,
// добавляет в этот список 3-й аргумент.
// Затем помещает указатель на список в ячейку стека, переданную 1-м аргументом.
template <class TTarget, class TItem>
void ConcatList(TTarget *& target, TTarget *& source, TItem *& item)
{
    auto pList = Take(source);
    if (item)
    {
        pList->emplace_back(Take(item));
    }
    target = pList.release();
};

using ExpressionListPtr = ExpressionList*;
using StatementListPtr = StatementsList*;
using StatementPtr = IStatementAST*;
using ExpressionPtr = IExpressionAST*;
using FunctionPtr = IFunctionAST*;
using NamesList = std::vector<unsigned>;
using NamesListPtr = NamesList*;

}

