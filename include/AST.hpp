// TODO: Refactor (move nodes to separate files)
#pragma once
#include <complex>
#include <format>
#include <functional>
#include <memory>
#include <utility>
#include <variant>

#include "Lexer.hpp"
#include "Value.hpp"

struct ASTNode
{
    ASTNode() = default;
    virtual ~ASTNode() = default;
};

struct Scope;

struct ExprNode : ASTNode
{
    virtual ValuePtr Evaluate(std::shared_ptr<Scope>) = 0;
    virtual std::shared_ptr<ExprNode> Clone(const std::shared_ptr<Scope> scope) const
    {
        throw std::runtime_error("Expression is not cloneable");
    };
};

using ExprPtr = std::shared_ptr<ExprNode>;
using SymbolTable = std::unordered_map<std::string, ExprPtr>;

struct UndefinedExpr final : ExprNode
{
    ValuePtr Evaluate(const std::shared_ptr<Scope> scope) override
    {
        throw std::runtime_error("Evaluation of an undefined expression");
    }
};

struct Scope
{
    explicit Scope(const std::shared_ptr<Scope>& parent = {})
        : parent(parent)
    {}

    void Declare(const std::string& name, ExprPtr value)
    {
        symbols[name] = std::move(value);
    }

    void Reset()
    {
        symbols.clear();
    }

    ExprPtr& Get(const std::string& name)
    {
        if(const auto it = symbols.find(name); it != symbols.end())
            return it->second;

        if(parent)
        {
            // Some kind of caching
            const auto& result = parent->Get(name);
            return symbols[name] = result;
        }

        throw std::runtime_error(std::format("Symbol '{}' not found", name));
    }

    bool Contains(const std::string& name) const
    {
        return symbols.contains(name) || (parent && parent->Contains(name));
    }

    std::shared_ptr<Scope> parent;
    SymbolTable symbols;
};

using ScopePtr = std::shared_ptr<Scope>;
using FunctionType = std::function<ValuePtr(const std::vector<ValuePtr>&, ScopePtr)>;

inline auto globalScope = std::make_shared<Scope>();

struct ValueExpr final : ExprNode
{
    explicit ValueExpr(const Value& value)
        : value(std::make_shared<Value>(value))
    {}

    ValuePtr Evaluate(const ScopePtr scope) override
    {
        return value;
    }

    ExprPtr Clone(const ScopePtr scope) const override
    {
        return std::make_shared<ValueExpr>(*value);
    }

    ValuePtr value;
};

struct VariableExpr final : ExprNode
{
    explicit VariableExpr(std::string name)
        : name(std::move(name))
    {}

    ValuePtr Evaluate(const ScopePtr scope) override
    {
        if(!cached)
            cached = scope->Get(name)->Evaluate(scope);

        return cached;
    }

    ValuePtr cached;
    std::string name;
};

struct VariableDecl final : ExprNode
{
    VariableDecl(std::string name, ExprPtr value)
        : name(std::move(name)), value(std::move(value))
    {}

    ValuePtr Evaluate(const ScopePtr scope) override
    {
        const auto evaluated = value->Evaluate(scope);

        if(scope && !scope->Contains(name))
            scope->Declare(name, value);

        return evaluated;
    }

    ExprPtr Clone(const ScopePtr scope) const override
    {
        return std::make_shared<VariableDecl>(name, std::make_shared<ValueExpr>(value->Evaluate(scope)));
    }

    std::string name;
    ExprPtr value;
};

struct ReturnExpr final : ExprNode
{
    struct ReturnValue
    {
        ValuePtr value;
    };

    explicit ReturnExpr(ExprPtr value)
        : value(std::move(value))
    {}

    ValuePtr Evaluate(const ScopePtr scope) override
    {
        throw ReturnValue{ value->Evaluate(scope) };
    }

    ExprPtr value;
};

struct BreakExpr final : ExprNode
{
    struct Exception {};

    explicit BreakExpr() {}

    ValuePtr Evaluate(const ScopePtr scope) override
    {
        throw Exception{};
    }

    ExprPtr value;
};

struct ContinueExpr final : ExprNode
{
    struct Exception {};

    explicit ContinueExpr() {}

    ValuePtr Evaluate(const ScopePtr scope) override
    {
        throw Exception{};
    }

    ExprPtr value;
};

struct StatementList final : ExprNode
{
    explicit StatementList(std::vector<ExprPtr>&& statements, std::vector<ExprPtr>&& args = {})
        : statements(std::move(statements)), args(std::move(args))
    {}

    explicit StatementList(FunctionType&& nativeFunc)
        : nativeFunc(std::move(nativeFunc))
    {}

    ValuePtr Evaluate(const ScopePtr scope) override
    {
        if(nativeFunc)
        {
            std::vector<ValuePtr> evaluatedArgs;
            evaluatedArgs.reserve(passedArgs.size());

            for(const auto& arg : passedArgs)
                evaluatedArgs.push_back(arg->Evaluate(scope));

            return nativeFunc(evaluatedArgs, scope);
        }

        const auto localScope = noLocalScope ? scope : std::make_shared<Scope>(scope);
        for(int i = 0; i < args.size(); i++)
        {
            if(passedArgs.size() < i + 1)
                throw std::runtime_error("Not enough arguments");

            const auto& argName = dynamic_cast<VariableDecl*>(args[i].get())->name;
            localScope->Declare(argName, std::move(passedArgs[i]));
        }

        ValuePtr result{};

        for(const auto& i : statements)
            result = i->Evaluate(localScope);

        return result;
    }

    bool noLocalScope = false;
    FunctionType nativeFunc{};
    std::vector<ExprPtr> statements, args, passedArgs;
};

struct FunctionDecl final : ExprNode
{
    explicit FunctionDecl(std::string name, ExprPtr body)
        : name(std::move(name)), body(std::move(body))
    {}

    ValuePtr Evaluate(const ScopePtr scope) override
    {
        scope->Declare(name, std::move(body));

        return nullptr;
    }

    // Not really the right way to clone, but this is needed for structs...
    ExprPtr Clone(const ScopePtr scope) const override
    {
        return std::make_shared<StatementList>(*std::static_pointer_cast<StatementList>(body));
    }

    std::string name;
    ExprPtr body;
};

using StructBody = std::unordered_map<std::string, ExprPtr>;
using Order = std::vector<std::string>;

struct StructDecl final : ExprNode
{
    explicit StructDecl(std::string name)
        : name(std::move(name))
    {}

    ValuePtr Evaluate(const ScopePtr scope) override
    {
        scope->Declare(name, std::make_shared<StructDecl>(*this));

        return nullptr;
    }

    std::string name;
    StructBody content;
    Order order;
};

// TODO: Review
struct StructInstance final : ExprNode
{
    explicit StructInstance(std::string name, ScopePtr localScope)
        : name(std::move(name)), localScope(std::move(localScope))
    {}

    ValuePtr Evaluate(const ScopePtr scope) override
    {
        return nullptr;
    }

    std::string name; // Redundant
    ScopePtr localScope; // Might work instead of this entire struct
};

using StructInstancePtr = std::shared_ptr<StructInstance>;

struct ConstructorExpr final : ExprNode
{
    explicit ConstructorExpr(std::string&& name, std::vector<ExprPtr>&& args)
        : name(std::move(name)), args(std::move(args))
    {}

    ValuePtr Evaluate(const ScopePtr scope) override
    {
        if(const auto structDecl = dynamic_cast<StructDecl*>(scope->Get(name).get()))
        {
            auto newScope = std::make_shared<Scope>(scope);

            for(const auto& [name, value] : structDecl->content)
                newScope->Declare(name, value->Clone(newScope));

            auto instance = std::make_shared<StructInstance>(name, newScope);

            newScope->Declare("this",
                std::make_shared<VariableDecl>("this", std::make_shared<ValueExpr>(instance))
            );

            if(structDecl->content.contains(name))
            {
                const auto constructorExpr = structDecl->content[name];
                auto constructor = std::static_pointer_cast<FunctionDecl>(constructorExpr);

                if(constructor)
                {
                    auto body = std::static_pointer_cast<StatementList>(constructor->body);
                    body->passedArgs = args;

                    body->Evaluate(newScope);
                }
            }
            else if(!args.empty())
            {
                for(int i = 0; i < std::min(args.size(), structDecl->order.size()); i++)
                {
                    auto fieldValue = args[i]->Evaluate(scope);
                    newScope->Get(structDecl->order[i]) = std::make_shared<ValueExpr>(*fieldValue);
                }
            }

            return std::make_shared<Value>(std::any{ instance });
        }

        throw std::runtime_error(std::format("Symbol '{}' is not a struct", name));
    }

    std::string name;
    std::vector<ExprPtr> args;
};

struct IfStatement final : ExprNode
{
    IfStatement(ExprPtr condition, ExprPtr then, ExprPtr elseExpr)
        : condition(std::move(condition)), then(std::move(then)), elseExpr(std::move(elseExpr))
    {}

    ValuePtr Evaluate(const ScopePtr scope) override
    {
        if(ValueOp::toBool(*condition->Evaluate(scope)))
            return then->Evaluate(scope);
        if(elseExpr)
            return elseExpr->Evaluate(scope);
        return nullptr;
    }

    ExprPtr condition, then, elseExpr;
};

struct WhileStatement final : ExprNode
{
    WhileStatement(ExprPtr condition, ExprPtr body)
        : condition(std::move(condition)), body(std::move(body))
    {}

    ValuePtr Evaluate(const ScopePtr scope) override
    {
        ValuePtr result{};

        while(ValueOp::toBool(*condition->Evaluate(scope)))
        {
            try
            {
                result = body->Evaluate(scope);
            }
            catch(const BreakExpr::Exception&) { break; }
            catch(const ContinueExpr::Exception&) {}
            catch(const ReturnExpr::ReturnValue&) { throw; }
        }

        return result;
    }

    ExprPtr condition, body;
};

struct ForStatement final : ExprNode
{
    ForStatement(ExprPtr init, ExprPtr condition, ExprPtr step, ExprPtr body)
        : init(std::move(init)), condition(std::move(condition)),
          step(std::move(step)), body(std::move(body))
    {}

    ValuePtr Evaluate(const ScopePtr scope) override
    {
        if((!init || !body) && !condition)
            return nullptr;

        auto localScope = scope;

        if(init)
        {
            localScope = std::make_shared<Scope>(scope);
            init->Evaluate(localScope);
        }

        ValuePtr result{};

        while(!condition || ValueOp::toBool(*condition->Evaluate(localScope)))
        {
            try
            {
                result = body->Evaluate(localScope);
            }
            catch(const BreakExpr::Exception&) { break; }
            catch(const ContinueExpr::Exception&) {}
            catch(const ReturnExpr::ReturnValue&) { throw; }

            if(step)
                step->Evaluate(localScope);
        }

        return result;
    }

    ExprPtr init, condition, step, body;
};

struct FunctionCall final : ExprNode
{
    FunctionCall(std::string&& name, std::vector<ExprPtr>&& args)
        : name(std::move(name)), args(std::move(args))
    {}

    ValuePtr Evaluate(const ScopePtr scope) override
    {
        if(scope->Contains(name))
        {
            const auto expr = scope->Get(name).get();
            if(const auto cast = dynamic_cast<StatementList*>(expr))
            {
                cast->passedArgs = args;

                ValuePtr result{};

                try
                {
                    result = expr->Evaluate(scope);
                }
                catch(const ReturnExpr::ReturnValue& returnExpr)
                {
                    result = returnExpr.value;
                }

                return result;
            }

            throw std::runtime_error(std::format("'{}' is not a function", name));
        }

        throw std::runtime_error(std::format("Function '{}' not found", name));
    }

    std::string name;
    std::vector<ExprPtr> args;
};

struct IndexExpr final : ExprNode
{
    IndexExpr(ExprPtr expr, ExprPtr index)
        : expr(std::move(expr)), index(std::move(index))
    {}

    ValuePtr Evaluate(const ScopePtr scope) override
    {
        const auto ptrValue = expr->Evaluate(scope);

        if(std::holds_alternative<size_t>(*ptrValue))
        {
            const auto indexValue = index->Evaluate(scope);

            const int idx = std::get<int>(*indexValue);

            const auto base = std::get<size_t>(*ptrValue);
            const auto element = base + (idx * sizeof(Value));

            return { reinterpret_cast<Value*>(element), [](Value*) {} };
        }

        throw std::runtime_error("Index operator can only be used on pointers");
    }

    ExprPtr expr, index;
};

struct UnaryExpr final : ExprNode
{
    UnaryExpr(Lexer::Token token, ExprPtr expr)
        : token(std::move(token)), expr(std::move(expr))
    {}

    ValuePtr Evaluate(const ScopePtr scope) override
    {
        using namespace ValueOp;

        ValuePtr val;

        if(dynamic_cast<VariableExpr*>(expr.get())
            || dynamic_cast<VariableDecl*>(expr.get())
            || dynamic_cast<ValueExpr*>(expr.get()))
            val = cachedExpr ? cachedExpr : cachedExpr = expr->Evaluate(scope);
        else
            val = expr->Evaluate(scope);

        ValuePtr oldValue{};

        switch(token.first)
        {
        case Lexer::TokenType::Plus: return val;
        case Lexer::TokenType::Minus: return std::make_shared<Value>(-*val);
        case Lexer::TokenType::Not:
            if(std::holds_alternative<bool>(*val))
                return std::make_shared<Value>(!std::get<bool>(*val));
            return std::make_shared<Value>(false);
        case Lexer::TokenType::Increment:
            if(operationFirst)
            {
                *val = *val + 1;
                return val;
            }

            oldValue = std::make_shared<Value>(*val);
            *val = *val + 1;
            return oldValue;

        case Lexer::TokenType::Decrement:
            if(operationFirst)
            {
                *val = *val - 1;
                return val;
            }

            oldValue = std::make_shared<Value>(*val);
            *val = *val - 1;
            return oldValue;

        case Lexer::TokenType::Pointer:
        {
            if(std::holds_alternative<size_t>(*val))
                return { reinterpret_cast<Value*>(std::get<size_t>(*val)), [](auto) {} };
            return std::make_shared<Value>(reinterpret_cast<size_t>(val.get()));
        }

        default: break;
        }

        return val;
    }

    Lexer::Token token;

    ExprPtr expr;
    ValuePtr cachedExpr;

    bool operationFirst = true; // That way we can handle prefix/postfix inc/dec
};

struct BinaryExpr final : ExprNode
{
    BinaryExpr(Lexer::Token token, ExprPtr left, ExprPtr right)
        : token(std::move(token)), left(std::move(left)), right(std::move(right))
    {}

    ValuePtr Evaluate(const ScopePtr scope) override
    {
        if(token.first == Lexer::TokenType::Dot)
        {
            const auto structExpr = left->Evaluate(scope);

            if(std::holds_alternative<std::any>(*structExpr))
            {
                const auto any = std::get<std::any>(*structExpr);
                const auto structInstance = std::any_cast<StructInstancePtr>(any);

                // Might need more testing
                structInstance->localScope->parent = scope;

                return right->Evaluate(structInstance->localScope);
            }

            throw std::runtime_error("Dot operator can only be used on structs");
        }

        ValuePtr l, r;

        // TODO: Review
        if(dynamic_cast<VariableExpr*>(left.get())
            || dynamic_cast<VariableDecl*>(left.get())
            || dynamic_cast<ValueExpr*>(left.get()))
            l = cachedLeft ? cachedLeft : cachedLeft = left->Evaluate(scope);
        else
            l = left->Evaluate(scope);

        if(dynamic_cast<VariableExpr*>(right.get())
            || dynamic_cast<VariableDecl*>(right.get())
            || dynamic_cast<ValueExpr*>(right.get()))
            r = cachedRight ? cachedRight : cachedRight = right->Evaluate(scope);
        else
            r = right->Evaluate(scope);

        using namespace ValueOp;

        switch(token.first)
        {
        case Lexer::TokenType::Equal: *l = *r; return l;
        case Lexer::TokenType::AddAssign: *l = *l + *r; return l;
        case Lexer::TokenType::SubAssign: *l = *l - *r; return l;
        case Lexer::TokenType::MulAssign: *l = *l * *r; return l;
        case Lexer::TokenType::DivAssign: *l = *l / *r; return l;
        case Lexer::TokenType::ModAssign: *l = *l % *r; return l;
        case Lexer::TokenType::BitwiseAndAssign: *l = *l & *r; return l;
        case Lexer::TokenType::BitwiseOrAssign: *l = *l | *r; return l;
        case Lexer::TokenType::BitwiseXorAssign: *l = *l ^ *r; return l;
        case Lexer::TokenType::Plus: return std::make_shared<Value>(*l + *r);
        case Lexer::TokenType::Minus: return std::make_shared<Value>(*l - *r);
        case Lexer::TokenType::Multiply: return std::make_shared<Value>(*l * *r);
        case Lexer::TokenType::Divide: return std::make_shared<Value>(*l / *r);
        case Lexer::TokenType::Modulo: return std::make_shared<Value>(*l % *r);
        case Lexer::TokenType::IsEqual: return std::make_shared<Value>(*l == *r);
        case Lexer::TokenType::NotEqual: return std::make_shared<Value>(*l != *r);
        case Lexer::TokenType::BitwiseAnd: return std::make_shared<Value>(*l & *r);
        case Lexer::TokenType::BitwiseOr: return std::make_shared<Value>(*l | *r);
        case Lexer::TokenType::BitwiseXor: return std::make_shared<Value>(*l ^ *r);
        case Lexer::TokenType::And: return std::make_shared<Value>(*l && *r);
        case Lexer::TokenType::Or: return std::make_shared<Value>(*l || *r);
        case Lexer::TokenType::Less: return std::make_shared<Value>(*l < *r);
        case Lexer::TokenType::Greater: return std::make_shared<Value>(*l > *r);
        case Lexer::TokenType::LessEqual: return std::make_shared<Value>(*l <= *r);
        case Lexer::TokenType::GreaterEqual: return std::make_shared<Value>(*l >= *r);
        default: break;
        }

        return l;
    }

    Lexer::Token token;

    ExprPtr left, right;
    ValuePtr cachedLeft, cachedRight;
};
