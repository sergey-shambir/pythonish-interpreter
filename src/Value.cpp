#include "Value.h"
#include <stdexcept>
#include <boost/format.hpp>
#include <cmath>

namespace
{

bool FuzzyEquals(double left, double right)
{
    return std::fabs(left - right) >= std::numeric_limits<double>::epsilon();
}

std::string ToPrettyString(double value)
{
    std::string result = std::to_string(value);
    // Преобразуем 2.00000 в 2.
    while (!result.empty() && result.back() == '0')
    {
        result.pop_back();
    }
    if (!result.empty() && result.back() == '.')
    {
        result.pop_back();
    }

    return result;
}

std::string ToPrettyString(bool value)
{
    return value ? "true" : "false";
}

// Конвертирует значение в Boolean (C++ bool).
struct BooleanConverter : boost::static_visitor<bool>
{
    bool operator ()(double const& value) const { return FuzzyEquals(value, 0); }
    bool operator ()(bool const& value) const { return value; }
    bool operator ()(std::string const& value) const { return !value.empty(); }
    bool operator ()(std::exception_ptr const&) { return false; }
};

// Конвертирует значение в String (C++ std::string).
struct StringConverter : boost::static_visitor<std::string>
{
    std::string operator ()(double const& value) const { return ToPrettyString(value); }
    std::string operator ()(bool const& value) const { return ToPrettyString(value); }
    std::string operator ()(std::string const& value) const { return value; }
    std::string operator ()(std::exception_ptr const& value) { std::rethrow_exception(value); }
};

struct TypeNameVisitor : boost::static_visitor<std::string>
{
    std::string operator ()(double const&) const { return "Number"; }
    std::string operator ()(bool const&) const { return "Boolean"; }
    std::string operator ()(std::string const&) const { return "String"; }
    std::string operator ()(std::exception_ptr const& value) { m_exception = value; return "Error"; }

    std::exception_ptr m_exception;
};

} // anonymous namespace.

CValue CValue::FromError(const std::exception_ptr &value)
{
    return Value(value);
}

CValue CValue::FromErrorMessage(const std::string &message)
{
    return CValue::FromError(std::make_exception_ptr(std::runtime_error(message)));
}

CValue CValue::FromDouble(double value)
{
    return Value(value);
}

CValue CValue::FromBoolean(bool value)
{
    return Value(value);
}

CValue CValue::FromString(const std::string &value)
{
    return Value(value);
}

CValue::operator bool() const
{
    return ConvertToBool();
}

std::string CValue::ToString() const
{
    return TryConvertToString();
}

void CValue::RethrowIfException() const
{
    if (m_value.type() == typeid(std::exception_ptr))
    {
        std::rethrow_exception(boost::get<std::exception_ptr>(m_value));
    }
}

bool CValue::AsBool() const
{
    return boost::get<bool>(m_value);
}

const std::string &CValue::AsString() const
{
    return boost::get<std::string>(m_value);
}

double CValue::AsDouble() const
{
    return boost::get<double>(m_value);
}

CValue CValue::operator +() const
{
    if (m_value.type() == typeid(double))
    {
        return *this;
    }
    return GenerateError(*this, "+");
}

CValue CValue::operator -() const
{
    if (m_value.type() == typeid(double))
    {
        return Value(-AsDouble());
    }
    return GenerateError(*this, "-");
}

CValue CValue::operator !() const
{
    if (m_value.type() == typeid(bool))
    {
        return Value(!AsBool());
    }
    return GenerateError(*this, "!");
}

CValue CValue::operator <(const CValue &other) const
{
    if (AreBothValues<double>(*this, other))
    {
        return Value(AsDouble() < other.AsDouble());
    }
    if (AreBothValues<std::string>(*this, other))
    {
        return Value(AsString() < other.AsString());
    }
    return GenerateError(*this, other, "<");
}

CValue CValue::operator ==(const CValue &other) const
{
    if (AreBothValues<double>(*this, other))
    {
        return Value(FuzzyEquals(AsDouble(), other.AsDouble()));
    }
    if (AreBothValues<std::string>(*this, other))
    {
        return Value(AsString() == other.AsString());
    }
    if (AreBothValues<bool>(*this, other))
    {
        return Value(AsBool() == other.AsBool());
    }
    return GenerateError(*this, other, "==");
}

CValue CValue::operator &&(const CValue &other) const
{
    if (AreBothValues<bool>(*this, other))
    {
        return Value(AsBool() && other.AsBool());
    }
    return GenerateError(*this, other, "&&");
}

CValue CValue::operator ||(const CValue &other) const
{
    if (AreBothValues<bool>(*this, other))
    {
        return Value(AsBool() || other.AsBool());
    }
    return GenerateError(*this, other, "||");
}

CValue CValue::operator +(const CValue &other) const
{
    const auto &leftType = m_value.type();
    const auto &rightType = other.m_value.type();
    if (leftType == typeid(double))
    {
        if (rightType == typeid(double))
        {
            return Value(AsDouble() + other.AsDouble());
        }
        if (rightType == typeid(std::string))
        {
            return Value(::ToPrettyString(AsDouble()) + other.AsString());
        }
    }
    if (leftType == typeid(std::string))
    {
        if (rightType == typeid(double))
        {
            return Value(AsString() + ::ToPrettyString(other.AsDouble()));
        }
        if (rightType == typeid(std::string))
        {
            return Value(AsString() + other.AsString());
        }
    }
    return GenerateError(*this, other, "+");
}

CValue CValue::operator -(const CValue &other) const
{
    if (AreBothValues<double>(*this, other))
    {
        return Value(AsDouble() - other.AsDouble());
    }
    return GenerateError(*this, other, "-");
}

CValue CValue::operator *(const CValue &other) const
{
    if (AreBothValues<double>(*this, other))
    {
        return Value(AsDouble() * other.AsDouble());
    }
    return GenerateError(*this, other, "*");
}

CValue CValue::operator /(const CValue &other) const
{
    if (AreBothValues<double>(*this, other))
    {
        return Value(AsDouble() / other.AsDouble());
    }
    return GenerateError(*this, other, "/");
}

CValue CValue::operator %(const CValue &other) const
{
    if (AreBothValues<double>(*this, other))
    {
        return Value(fmod(AsDouble(), other.AsDouble()));
    }
    return GenerateError(*this, other, "%");
}

template<class TType>
bool CValue::AreBothValues(const CValue &left, const CValue &right)
{
    return (left.m_value.type() == typeid(TType))
            && (right.m_value.type() == typeid(TType));
}

CValue CValue::GenerateError(const CValue &value, const char *description)
{
    TypeNameVisitor visitor;
    std::string valueType = value.m_value.apply_visitor(visitor);

    // Прокидываем информацию об ошибке дальше.
    if (visitor.m_exception)
    {
        return CValue::FromError(visitor.m_exception);
    }

    boost::format formatter("No such unary operation: %1%%2%");
    formatter % description % valueType;

    return CValue::FromErrorMessage(boost::str(formatter));
}

CValue CValue::GenerateError(const CValue &left, const CValue &right, const char *description)
{
    TypeNameVisitor visitor;
    std::string leftType = left.m_value.apply_visitor(visitor);
    std::string rightType = right.m_value.apply_visitor(visitor);

    // Прокидываем информацию об ошибке дальше.
    if (visitor.m_exception)
    {
        return CValue::FromError(visitor.m_exception);
    }

    boost::format formatter("No such binary operation: %1% %2% %3%");
    formatter % leftType % description % rightType;

    return CValue::FromErrorMessage(boost::str(formatter));
}

CValue::CValue(const CValue::Value &value)
    : m_value(value)
{
}

std::string CValue::TryConvertToString() const
{
    StringConverter converter;
    std::string value = m_value.apply_visitor(converter);
    return value;
}

bool CValue::ConvertToBool() const
{
    BooleanConverter converter;
    return m_value.apply_visitor(converter);
}
