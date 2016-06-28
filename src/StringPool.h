#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/noncopyable.hpp>

// Реализует приём "пул строк" (англ. string interning, string pool)
// Пул строк позволяет однозначно сопоставить строку и её целочисленный ID.
// - ID можно использовать в стеке LALR парсера, созданного Lemon. Lemon генерирует код с 'union',
//   в котором можно хранить unsigned и нельзя хранить std::string.
// - Также ID немного ускорит компиляцию, для него быстрее работает копирование, сравнение
//   и подсчёт хеша.
class CStringPool : private boost::noncopyable
{
public:
    CStringPool();

    unsigned Insert(std::string const& str);
    std::string GetString(unsigned id)const;

private:
    std::unordered_map<std::string, unsigned> m_mapping;
    std::vector<std::string> m_pool;
};
