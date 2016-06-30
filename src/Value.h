#pragma once
#include <boost/variant.hpp>
#include <iostream>

class CValue
{
public:
    using Value = boost::variant<
        std::exception_ptr,
        bool,
        double,
        std::string
    >;

	// Конструктор без параметров нужен для упрощения хранения CValue в контейнерах STL
    CValue() = default;

    static CValue FromError(const std::exception_ptr &value);
    static CValue FromErrorMessage(const std::string &message);
    static CValue FromDouble(double value);
    static CValue FromBoolean(bool value);
    static CValue FromString(const std::string &value);

    // Преобразует в строку. Перебрасывает исключение, если объект хранит ошибку.
    std::string ToString()const;
	// Возвращает true, если значение позволяет войти в ветку then
	// в инструкциях условного выполнения.
	bool AllowsThenBranch()const;
    // Перевыбрасывает исключение, если объект хранит ошибку.
    void RethrowIfException()const;

    // Получение значения известного типа.
    // Выбрасывает boost::bad_get в случае несоответствия типа ожидаемому.
    bool AsBool()const;
    const std::string & AsString()const;
    double AsDouble()const;

    // Унарные и бинарные операции с проверкой типов.
    CValue operator +()const;
    CValue operator -()const;
    CValue operator !()const;
    CValue operator <(const CValue &other)const;
    CValue operator ==(const CValue &other)const;
    CValue operator &&(const CValue &other)const;
    CValue operator ||(const CValue &other)const;
    CValue operator +(const CValue &other)const;
    CValue operator -(const CValue &other)const;
    CValue operator *(const CValue &other)const;
    CValue operator /(const CValue &other)const;
    CValue operator %(const CValue &other)const;

private:
    template<class TType>
    static bool BothValuesAre(const CValue &left, const CValue &right);

    static CValue GenerateError(const CValue &value, const char *description);
    static CValue GenerateError(const CValue &left, const CValue &right, const char *description);

    CValue(Value const& value);
    std::string TryConvertToString()const;

    Value m_value;
};
