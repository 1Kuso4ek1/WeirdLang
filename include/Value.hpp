#pragma once
#include <memory>
#include <variant>

using Value = std::variant<int, float, double, bool, std::string>;
using ValuePtr = std::shared_ptr<Value>;

inline Value operator-(const Value& val)
{
    return std::visit([](auto&& v) -> Value { return -v;  }, val);
}

inline Value operator+(const Value& left, const Value& right)
{
    return std::visit([](auto&& l, auto&& r) -> Value { return l + r; }, left, right);
}

inline Value operator-(const Value& left, const Value& right)
{
    return std::visit([](auto&& l, auto&& r) -> Value { return l - r; }, left, right);
}

inline Value operator*(const Value& left, const Value& right)
{
    return std::visit([](auto&& l, auto&& r) -> Value { return l * r; }, left, right);
}

inline Value operator/(const Value& left, const Value& right)
{
    return std::visit([](auto&& l, auto&& r) -> Value { return l / r; }, left, right);
}

inline Value operator%(const Value& left, const Value& right)
{
    return std::visit([](auto&& l, auto&& r) -> Value
    {
        if constexpr (std::is_same_v<std::decay_t<decltype(l)>, int>
                    && std::is_same_v<std::decay_t<decltype(r)>, int>)
            return l % r;
        return 0;
    }, left, right);
}

inline Value operator!(const Value& val)
{
    return std::visit([](auto&& v) -> Value
    {
        if constexpr (std::is_same_v<std::decay_t<decltype(v)>, bool>)
            return !v;
        return false;
    }, val);
}

inline Value operator&(const Value& left, const Value& right)
{
    return std::visit([](auto&& l, auto&& r) -> Value
    {
        if constexpr (std::is_integral_v<std::decay_t<decltype(l)>>
                    && std::is_integral_v<std::decay_t<decltype(r)>>)
            return l & r;
        return 0;
    }, left, right);
}

inline Value operator|(const Value& left, const Value& right)
{
    return std::visit([](auto&& l, auto&& r) -> Value
    {
        if constexpr (std::is_integral_v<std::decay_t<decltype(l)>>
                    && std::is_integral_v<std::decay_t<decltype(r)>>)
            return l | r;
        return 0;
    }, left, right);
}

inline Value operator^(const Value& left, const Value& right)
{
    return std::visit([](auto&& l, auto&& r) -> Value
    {
        if constexpr (std::is_integral_v<std::decay_t<decltype(l)>>
                    && std::is_integral_v<std::decay_t<decltype(r)>>)
            return l ^ r;
        return 0;
    }, left, right);
}

inline Value operator&&(const Value& left, const Value& right)
{
    return std::visit([](auto&& l, auto&& r) -> Value
    {
        if constexpr (std::is_integral_v<std::decay_t<decltype(l)>>
                    && std::is_integral_v<std::decay_t<decltype(r)>>)
            return l && r;
        return false;
    }, left, right);
}

inline Value operator||(const Value& left, const Value& right)
{
    return std::visit([](auto&& l, auto&& r) -> Value
    {
        if constexpr (std::is_integral_v<std::decay_t<decltype(l)>>
                    && std::is_integral_v<std::decay_t<decltype(r)>>)
            return l || r;
        return false;
    }, left, right);
}

inline Value operator==(const Value& left, const Value& right)
{
    return std::visit([](auto&& l, auto&& r) -> Value { return l == r; }, left, right);
}

inline Value operator!=(const Value& left, const Value& right)
{
    return std::visit([](auto&& l, auto&& r) -> Value { return l != r; }, left, right);
}

inline Value operator<(const Value& left, const Value& right)
{
    return std::visit([](auto&& l, auto&& r) -> Value
    {
        if constexpr (std::is_arithmetic_v<std::decay_t<decltype(l)>>
                    && std::is_arithmetic_v<std::decay_t<decltype(r)>>)
            return l < r;
        return false;
    }, left, right);
}

inline Value operator>(const Value& left, const Value& right)
{
    return std::visit([](auto&& l, auto&& r) -> Value
    {
        if constexpr (std::is_arithmetic_v<std::decay_t<decltype(l)>>
                    && std::is_arithmetic_v<std::decay_t<decltype(r)>>)
            return l > r;
        return false;
    }, left, right);
}

inline Value operator<=(const Value& left, const Value& right)
{
    return std::visit([](auto&& l, auto&& r) -> Value
    {
        if constexpr (std::is_arithmetic_v<std::decay_t<decltype(l)>>
                    && std::is_arithmetic_v<std::decay_t<decltype(r)>>)
            return l <= r;
        return false;
    }, left, right);
}

inline Value operator>=(const Value& left, const Value& right)
{
    return std::visit([](auto&& l, auto&& r) -> Value
    {
        if constexpr (std::is_arithmetic_v<std::decay_t<decltype(l)>>
                    && std::is_arithmetic_v<std::decay_t<decltype(r)>>)
            return l >= r;
        return false;
    }, left, right);
}

inline bool toBool(const Value& val)
{
    return std::visit([](auto&& v) -> bool
    {
        if constexpr (std::is_integral_v<std::decay_t<decltype(v)>>)
            return static_cast<bool>(v);
        return false;
    }, val);
}
