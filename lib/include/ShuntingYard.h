#pragma once

#include <stack>
#include <vector>
#include <string>
#include <functional>

/**
 * This template class implements 2-stack modification of Shunting-yard Dijkstra's algorithm.
 *
 * This class:
 *  - evaluates an infix mathematical expressions;
 *  - evaluates an expression on the run;
 *  - supports operations: +, -, *, /;
 *  - supports '(' and ')';
 *  - can avaluate an expression for several parse() calls;
 *  - considers '\n' as end of en expession;
 *  - skips symbols '\t', '\r' , ' '.
 *
 * How to use it?
 * using ShuntingYardInt = ShuntingYard<int>;
 * ShuntingYardInt shanting_yard;
 * ShuntingYardInt::ParseResult r = shanting_yard.parse("(7 + 2", 6);
 * assert(r.first == ShuntingYardInt::Incomplete);
 * r = shanting_yard.parse(") / 3 + 5 * 6\n", 14);
 * assert(r.first == ShuntingYardInt::ParseResult::Success);
 * assert(r.first == 33);
 *
 * Library can receive several expressions:
 * r = shanting_yard.parse("1 + 2\n 3 - 4\n 5 * 6\n7 / 8\n", 26);
 * assert(r.first == ShuntingYardInt::ParseResult::Success);
 * assert(r.second == 3);
 * assert(shanting_yard.is_empty() == false);
 *
 * r = shanting_yard.parse(nullptr, 0);
 * assert(r.first == ShuntingYardInt::ParseResult::Success);
 * assert(r.second == -1);
 * assert(shanting_yard.is_empty() == false);
 *
 * r = shanting_yard.parse(nullptr, 0);
 * assert(r.first == ShuntingYardInt::ParseResult::Success);
 * assert(r.second == 30);
 * assert(shanting_yard.is_empty() == false);
 *
 * r = shanting_yard.parse(nullptr, 0);
 * assert(r.first == ShuntingYardInt::ParseResult::Success);
 * assert(r.second == 0);
 * assert(shanting_yard.is_empty() == true);
 *
 * This class can return 'DivisionByZero' and 'InvalidExpression':
 * r = shanting_yard.parse("5/(2/3)\n", 7);
 * assert(r.first == ShuntingYardInt::ParseResult::DivisionByZero);
 * r = shanting_yard.parse("(1 + 2\n", 7);
 * assert(r.first == ShuntingYardInt::ParseResult::InvalidExpression);
 */

template <class Type>
class ShuntingYard
{
public:
    enum class ParseResult
    {
        Success,
        Incomplete,
        DivisionByZero,
        InvalidExpression
    };

    using Result = std::pair<ParseResult, Type>;

    ShuntingYard() : step(0), level(0) {}

    /**
    * This method parses partial expression.
    * Symbol '\n' must be at end of expression.
    * The method skips symbols ' ', '\t' and '\r' in source data.
    * @param s[in] partial expression.
    * @param len[len] length of partial expression.
    * @retval <ShuntingYard::Success, value> if end of expression was met and there were no any mistakes in expression.
    * @retval <ShuntingYard::Incomplete, Type()> if end of expression was not met and there were no any mistakes in expression.
    * @retval <ShuntingYard::DivisionByZero, Type()> if division by zero in expression happens.
    * @retval <ShuntingYard::InvalidExpression, Type()> if there is mistake in expression.
    */
    Result parse(const char* s, size_t len);

    /** Clear parser to further processing. */
    void clear();

    /** Does ShuntingYard contain unprocessed data?*/
    bool is_empty() const;

private:
    enum class BaseOperatorsEnum
    {
        Plus,
        Minus,
        Mult,
        Divide,
        Invalid
    };

    struct BaseOperators
    {
        unsigned int base_priority;
        std::function<Type(Type, Type)> calculator;
    };

    struct Operator
    {
        unsigned int priority;
        BaseOperatorsEnum base_operator;
    };

private:
    using LocalParseResult = std::pair<ParseResult, bool>;

    template <class T>
    using Stack = std::stack<T, std::deque<T>>;

    /** Calculate partial result using members operands and operators. */
    bool calculate();

    /** Methods of state machine */
    static LocalParseResult step_level_up_and_skip(ShuntingYard* self, std::string::const_iterator& it, std::string::const_iterator it_end);
    static LocalParseResult step_get_number(ShuntingYard* self, std::string::const_iterator& it, std::string::const_iterator it_end);
    static LocalParseResult step_level_down_and_skip(ShuntingYard* self, std::string::const_iterator& it, std::string::const_iterator it_end);
    static LocalParseResult step_check_end_of_expression(ShuntingYard* self, std::string::const_iterator& it, std::string::const_iterator it_end);
    static LocalParseResult step_process_operator(ShuntingYard* self, std::string::const_iterator& it, std::string::const_iterator it_end);

    /** Auxiliary functions to fill ShuntingYard::base_operators. */
    static Type plus  (Type a, Type b) { return a + b; }
    static Type minus (Type a, Type b) { return a - b; }
    static Type mult  (Type a, Type b) { return a * b; }
    static Type divide(Type a, Type b) { return a / b; }

    static std::string::const_iterator get_first_not_a_digit(std::string::const_iterator begin, std::string::const_iterator it_end);
    static bool is_skip_symbol(char op);
    static BaseOperatorsEnum get_base_operator(char op);
    static std::pair<Type, bool> convert(const std::string& value);

private:
    static const std::function<LocalParseResult(ShuntingYard*, std::string::const_iterator&, std::string::const_iterator)> steps[5];
    static const BaseOperators base_operators[4];
    static const unsigned int order;

    /** Next step of state machine. */
    unsigned int step;

    /** Current level of brackets */
    unsigned int level;

    /** Stack of operands in Polish notation. */
    Stack<Type> operands;

    /** Stack of operators in Polish notation. */
    Stack<Operator> operators;

    /** First part of incomplete number. */
    std::string remainder;

    //Friend for unit-tests.
    friend class ShuntingYardTest;
};

#include "ShuntingYard.tpp"
