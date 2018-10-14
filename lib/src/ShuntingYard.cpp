#include "ShuntingYard.h"

namespace
{
	//Auxiliary functions to fill ShuntingYard::base_operators.
	ShuntingYard::Type plus  (ShuntingYard::Type a, ShuntingYard::Type b) { return a + b; }
	ShuntingYard::Type minus (ShuntingYard::Type a, ShuntingYard::Type b) { return a - b; }
	ShuntingYard::Type mult  (ShuntingYard::Type a, ShuntingYard::Type b) { return a * b; }
	ShuntingYard::Type divide(ShuntingYard::Type a, ShuntingYard::Type b) { return a / b; }

	//Return iterator to first not a digit or it_end.
	std::string::const_iterator get_first_not_a_digit(std::string::const_iterator begin, std::string::const_iterator it_end)
	{
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

} //nameless namespace

const std::function<ShuntingYard::LocalParseResult(ShuntingYard* self, std::string::const_iterator&, std::string::const_iterator)> ShuntingYard::steps[5] =
{
    ShuntingYard::step_level_up,
    ShuntingYard::step_get_number,
    ShuntingYard::step_level_down,
    ShuntingYard::step_check_end_of_expression,
    ShuntingYard::step_process_operator
};

const ShuntingYard::BaseOperators ShuntingYard::base_operators[4] = { { 1, plus }, { 1, minus }, { 2, mult }, { 2, divide } };
const unsigned int ShuntingYard::order = 2;

ShuntingYard::Result ShuntingYard::parse(const char* s, size_t len)
{
	Result result{ParseResult::Success, Type{}};

	//Only '\n' in source data.
	if (len == 1 && s[0] == '\n' && is_empty())
	{
		return result;
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

			result.first = rc.first;
			return result;
		}
	}

	//Make result.
	result.second = operands.top();
	step = 0;
	operands.pop();

	return result;
};

void ShuntingYard::clear()
{
	step = 0;
	level = 0;
	Stack<Type> operands_;
	operands.swap(operands_);
	Stack<Operator> operators_;
	operators.swap(operators_);
}

bool ShuntingYard::is_empty() const
{
	return !step && !level && operands.empty() && operators.empty() && remainder.empty();
}

bool ShuntingYard::calculate()
{
	Type arg = operands.top();
	operands.pop();
	BaseOperatorsEnum base_operator = operators.top().base_operator;
	if (base_operator == BaseOperatorsEnum::Divide && !arg)
	{
		return false;
	}
	Type result = base_operators[static_cast<int>(base_operator)].calculator(operands.top(), arg);
	operands.pop();
	operands.push(result);
	operators.pop();

	return true;
}

ShuntingYard::LocalParseResult ShuntingYard::step_level_up(ShuntingYard* self, std::string::const_iterator& it, std::string::const_iterator it_end)
{
	ShuntingYard::LocalParseResult rc{ ParseResult::Success, false };

	while (it != it_end && (*it == '(' || *it == ' ' || *it == '\t'))
	{
		if (*it == '(')
		{
            self->level += ShuntingYard::order;
		}
		++it;
	}

	if (it == it_end)
    {
		rc.first = ParseResult::Incomplete;
		return rc;
	}

    ++(self->step);
	return rc;
}

ShuntingYard::LocalParseResult ShuntingYard::step_get_number(ShuntingYard* self, std::string::const_iterator& it, std::string::const_iterator it_end)
{
	ShuntingYard::LocalParseResult rc{ ParseResult::Success, false };

	bool negative = (*it == '-');

	if (negative)
	{
		++it;
	}

	if (it == it_end)
	{
        self->remainder = '-';
		rc.first = ParseResult::Incomplete;
		return rc;
	}

	std::string::const_iterator it2 = get_first_not_a_digit(it, it_end);
	if (it2 == it_end)
	{
        self->remainder.assign(negative ? it - 1 : it, it_end);
		rc.first = ParseResult::Incomplete;
		return rc;
	}
	else if (it == it2)
	{
		rc.first = ParseResult::InvalidExpression;
		return rc;
	}

	Type n;
	std::string s(negative ? it - 1 : it, it2);
	try
	{
		n = std::stoi(s);
	}
	catch (...)
	{
		rc.first = ParseResult::InvalidExpression;
		return rc;
	}

    self->operands.push(n);
	it = it2;

    ++(self->step);
	return rc;
}

ShuntingYard::LocalParseResult ShuntingYard::step_level_down(ShuntingYard* self, std::string::const_iterator& it, std::string::const_iterator it_end)
{
	ShuntingYard::LocalParseResult rc{ ParseResult::Success, false };

	while (it != it_end && (*it == ')' || *it == ' ' || *it == '\t'))
	{
		if (*it == ')')
		{
            if (!self->level)
            {
				rc.first = ParseResult::InvalidExpression;
				return rc;
			}

            self->level -= ShuntingYard::order;
		}
		++it;
	}

	if (it == it_end)
	{
		rc.first = ParseResult::Incomplete;
		return rc;
	}

    ++(self->step);
	return rc;
}

ShuntingYard::LocalParseResult ShuntingYard::step_check_end_of_expression(ShuntingYard* self, std::string::const_iterator& it, std::string::const_iterator it_end)
{
	ShuntingYard::LocalParseResult rc{ ParseResult::Success, false };

	if (*it == '\n')
	{
        if (self->level)
		{
			rc.first = ParseResult::InvalidExpression;
			return rc;
		}

        while (!self->operators.empty())
		{
            if (!self->calculate())
			{
				rc.first = ParseResult::DivisionByZero;
				return rc;
			}
		}

		if (++it != it_end)
		{
            self->remainder.assign(it, it_end);
		}

		rc.second = true;
	}

    ++(self->step);
	return rc;
}

ShuntingYard::LocalParseResult ShuntingYard::step_process_operator(ShuntingYard* self, std::string::const_iterator& it, std::string::const_iterator it_end)
{
	ShuntingYard::LocalParseResult rc{ ParseResult::Success, false };

	while (*it == ' ' || *it == '\t')
	{
		++it;
	}

	if (it == it_end)
	{
		rc.first = ParseResult::Incomplete;
		return rc;
	}

	BaseOperatorsEnum base_operator;
	switch (*it++)
	{
		case '+': base_operator = BaseOperatorsEnum::Plus;  break;
		case '-': base_operator = BaseOperatorsEnum::Minus; break;
		case '*': base_operator = BaseOperatorsEnum::Mult;  break;
		case '/': base_operator = BaseOperatorsEnum::Divide; break;
		default:
			rc.first = ParseResult::InvalidExpression;
			return rc;
	}

    unsigned int priority = base_operators[static_cast<int>(base_operator)].base_priority + self->level;

    while (!self->operators.empty() && self->operators.top().priority >= priority)
	{
        if (!self->calculate())
		{
			rc.first = ParseResult::DivisionByZero;
			return rc;
		}
	}

    self->operators.push(Operator{ priority, base_operator });

    self->step = 0;
	return rc;
}
