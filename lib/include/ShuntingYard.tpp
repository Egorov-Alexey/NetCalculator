#include <boost/lexical_cast.hpp>

template<class Type>
const std::function<typename ShuntingYard<Type>::LocalParseResult(ShuntingYard<Type>* self, std::string::const_iterator&, std::string::const_iterator)> ShuntingYard<Type>::steps[5] =
{
    ShuntingYard::step_level_up_and_skip,
    ShuntingYard::step_get_number,
    ShuntingYard::step_level_down_and_skip,
    ShuntingYard::step_check_end_of_expression,
    ShuntingYard::step_process_operator
};

template<class Type>
const typename ShuntingYard<Type>::BaseOperators ShuntingYard<Type>::base_operators[4] = { { 1, plus }, { 1, minus }, { 2, mult }, { 2, divide } };

template<class Type>
const unsigned int ShuntingYard<Type>::order = 2;

template<class Type>
typename ShuntingYard<Type>::Result ShuntingYard<Type>::parse(const char* s, size_t len)
{
    if (len == 1 && s[0] == '\n' && is_empty())
    {	//Only '\n' in source data.
        return std::make_pair(ParseResult::Success, Type{});
    }

    //Make string to parse.
    std::string str;
    str.reserve(len + remainder.size());
    str = remainder;
    remainder.clear();
    str.append(s, len);

    LocalParseResult rc{ ParseResult::Success, false };
    std::string::const_iterator it = str.begin();
    std::string::const_iterator it_end = str.end();

    //Process data in state machine.
    while (!rc.second)
    {
        rc = steps[step](this, it, it_end);
        if (rc.first != ParseResult::Success)
        {
            if (rc.first != ParseResult::Incomplete)
            {
                clear();
            }
            return std::make_pair(rc.first, Type{});
        }
    }

    //Make result.
    Type value = operands.top();
    operands.pop();
    step = 0;

    return std::make_pair(ParseResult::Success, value);
};

template<class Type>
void ShuntingYard<Type>::clear()
{
    step = 0;
    level = 0;
    Stack<Type> operands_;
    operands.swap(operands_);
    Stack<Operator> operators_;
    operators.swap(operators_);
    remainder.clear();
}

template<class Type>
bool ShuntingYard<Type>::is_empty() const
{
    return !step && !level && operands.empty() && operators.empty() && remainder.empty();
}

template<class Type>
bool ShuntingYard<Type>::calculate()
{
    Type arg = operands.top();
    operands.pop();
    BaseOperatorsEnum base_operator = operators.top().base_operator;
    if (base_operator == BaseOperatorsEnum::Divide && !arg)
    {
        return false;
    }
    operands.top() = base_operators[static_cast<int>(base_operator)].calculator(operands.top(), arg);
    operators.pop();

    return true;
}

template<class Type>
typename ShuntingYard<Type>::LocalParseResult ShuntingYard<Type>::step_level_up_and_skip(ShuntingYard* self, std::string::const_iterator& it, std::string::const_iterator it_end)
{
    while (it != it_end && (*it == '(' || is_skip_symbol(*it)))
    {
        if (*it == '(')
        {
            self->level += ShuntingYard::order;
        }
        ++it;
    }

    if (it == it_end)
    {
        return std::make_pair(ParseResult::Incomplete, false);
    }

    ++(self->step);
    return std::make_pair(ParseResult::Success, false);
}

template<class Type>
typename ShuntingYard<Type>::LocalParseResult ShuntingYard<Type>::step_get_number(ShuntingYard* self, std::string::const_iterator& it, std::string::const_iterator it_end)
{   //Assume that 'it' can not be equal to 'it_end' at the beginnig. Skip symbols are absent at the beginnig.
    bool negative = (*it == '-');
    if (negative)
    {
        ++it;
    }

    if (it == it_end)
    {
        self->remainder = '-';
        return std::make_pair(ParseResult::Incomplete, false);
    }

    std::string::const_iterator it2 = get_first_not_a_digit(it, it_end);
    if (it2 == it_end)
    {
        self->remainder.assign(negative ? it - 1 : it, it_end);
        return std::make_pair(ParseResult::Incomplete, false);
    }
    else if (it == it2)
    {
        return std::make_pair(ParseResult::InvalidExpression, false);
    }

    std::pair<Type, bool> n{convert(std::string(negative ? it - 1 : it, it2))};
    if (!n.second)
    {
        return std::make_pair(ParseResult::InvalidExpression, false);
    }

    self->operands.push(n.first);
    it = it2;

    ++(self->step);
    return std::make_pair(ParseResult::Success, false);
}

template<class Type>
typename ShuntingYard<Type>::LocalParseResult ShuntingYard<Type>::step_level_down_and_skip(ShuntingYard* self, std::string::const_iterator& it, std::string::const_iterator it_end)
{
    while (it != it_end && (*it == ')' || is_skip_symbol(*it)))
    {
        if (*it == ')')
        {
            if (!self->level)
            {
                return std::make_pair(ParseResult::InvalidExpression, false);
            }

            self->level -= ShuntingYard::order;
        }
        ++it;
    }

    if (it == it_end)
    {
        return std::make_pair(ParseResult::Incomplete, false);
    }

    ++(self->step);
    return std::make_pair(ParseResult::Success, false);
}

template<class Type>
typename ShuntingYard<Type>::LocalParseResult ShuntingYard<Type>::step_check_end_of_expression(ShuntingYard* self, std::string::const_iterator& it, std::string::const_iterator it_end)
{   //Assume that 'it' can not be equal to 'it_end' at the beginnig. Skip symbols are absent at the beginnig.
    bool found = false;

    if (*it == '\n')
    {
        if (self->level)
        {
            return std::make_pair(ParseResult::InvalidExpression, false);
        }

        while (!self->operators.empty())
        {
            if (!self->calculate())
            {
                return std::make_pair(ParseResult::DivisionByZero, false);
            }
        }

        if (++it != it_end)
        {
            self->remainder.assign(it, it_end);
        }

        found = true;
    }

    ++(self->step);
    return std::make_pair(ParseResult::Success, found);
}

template<class Type>
typename ShuntingYard<Type>::LocalParseResult ShuntingYard<Type>::step_process_operator(ShuntingYard* self, std::string::const_iterator& it, std::string::const_iterator it_end)
{
    while (it != it_end && is_skip_symbol(*it)) { ++it; }

    if (it == it_end)
    {
        return std::make_pair(ParseResult::Incomplete, false);
    }

    BaseOperatorsEnum base_operator{get_base_operator(*it++)};
    if (base_operator == BaseOperatorsEnum::Invalid)
    {
        return std::make_pair(ParseResult::InvalidExpression, false);
    }

    unsigned int priority = base_operators[static_cast<int>(base_operator)].base_priority + self->level;

    while (!self->operators.empty() && self->operators.top().priority >= priority)
    {
        if (!self->calculate())
        {
            return std::make_pair(ParseResult::DivisionByZero, false);
        }
    }

    self->operators.push(Operator{ priority, base_operator });
    self->step = 0;

    return std::make_pair(ParseResult::Success, false);
}

template<class Type>
std::string::const_iterator ShuntingYard<Type>::get_first_not_a_digit(std::string::const_iterator begin, std::string::const_iterator it_end)
{
    //Return iterator to first not a digit or it_end.
	//Note: std::string::find_first_not_of has complexity O(str1.size() * str2.size()) at worst.
    std::string::const_iterator it = begin;
    for (; it != it_end; ++it)
    {
        if (!isdigit(*it))
        {
            break;
        }
    }

    return it;
}

template<class Type>
bool ShuntingYard<Type>::is_skip_symbol(char op)
{
    return op == ' ' || op == '\t' || op == '\r';
}

template<class Type>
typename ShuntingYard<Type>::BaseOperatorsEnum ShuntingYard<Type>::get_base_operator(char op)
{
    switch (op)
    {
        case '+': return BaseOperatorsEnum::Plus;
        case '-': return BaseOperatorsEnum::Minus;
        case '*': return BaseOperatorsEnum::Mult;
        case '/': return BaseOperatorsEnum::Divide;
    }

    return BaseOperatorsEnum::Invalid;
}

template<class Type>
std::pair<Type, bool> ShuntingYard<Type>::convert(const std::string& value)
{
    std::pair<Type, bool> result{Type{}, true};

    try
    {
        result.first = boost::lexical_cast<Type>(value);
    }
    catch (const boost::bad_lexical_cast&)
    {
        result.second = false;
        return result;
    }

    return  result;
}
