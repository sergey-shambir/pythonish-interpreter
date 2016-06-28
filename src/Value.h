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

    CValue() = default;

    static CValue FromError(const std::exception_ptr &value);
    static CValue FromErrorMessage(const std::string &message);
    static CValue FromDouble(double value);
    static CValue FromBoolean(bool value);
    static CValue FromString(const std::string &value);

    // Преобразует в строку. Перебрасывает исключение, если объект хранит ошибку.
    std::string ToString()const;
    // Перебрасывает исключение, если объект хранит ошибку.
    void RethrowIfException()const;

    // Прямое приведение типов,
    // Выбрасывает boost::bad_get в случае несоответствия типа ожидаемому.
    bool AsBool()const;
    const std::string & AsString()const;
    double AsDouble()const;

    // Унарные и бинарные операции с проверкой типов.
    explicit operator bool()const;
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
    static bool AreBothValues(const CValue &left, const CValue &right);

    static CValue GenerateError(const CValue &value, const char *description);
    static CValue GenerateError(const CValue &left, const CValue &right, const char *description);

    CValue(Value const& value);
    std::string TryConvertToString()const;
    bool ConvertToBool()const;

    Value m_value;
};
