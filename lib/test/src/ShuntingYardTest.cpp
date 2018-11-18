#include "ShuntingYard.h"

#include <string>
#include <iostream>
#include <functional>

using ShuntingYardInt = ShuntingYard<int>;

struct ShuntingYardTestCase
{
	std::string expr;
	int result;
    ShuntingYardInt::ParseResult parse_result;
};

const ShuntingYardTestCase shunting_yard_test_array[] =
{
    { "",					0,		ShuntingYardInt::ParseResult::Incomplete},
    { "-",					0,		ShuntingYardInt::ParseResult::Incomplete},
    { "1",					0,		ShuntingYardInt::ParseResult::Incomplete},
    { "1 + 2",				0,		ShuntingYardInt::ParseResult::Incomplete},
    { "\n",					0,		ShuntingYardInt::ParseResult::Success },
    { "123\n",              123,	ShuntingYardInt::ParseResult::Success },
    { "-123\n",             -123,	ShuntingYardInt::ParseResult::Success },
    { "(123)\n",            123,	ShuntingYardInt::ParseResult::Success },
    { "(-123)\n",           -123,	ShuntingYardInt::ParseResult::Success },
    { "123 + 456\n",		579,	ShuntingYardInt::ParseResult::Success },
    { "-123 + -456\n",		-579,	ShuntingYardInt::ParseResult::Success },
    { "(-123) + (-456)\n",	-579,	ShuntingYardInt::ParseResult::Success },
    { "123 + 456*789\n",	359907,	ShuntingYardInt::ParseResult::Success },
    { "(123 + 456)*789\n",	456831,	ShuntingYardInt::ParseResult::Success },
    { "123 - 456*789\n",	-359661,ShuntingYardInt::ParseResult::Success },
    { "(123 - 456)*789\n",	-262737,ShuntingYardInt::ParseResult::Success },
    { "456 + 789/123\n",	462,	ShuntingYardInt::ParseResult::Success },
    { "(12 * 34) / 56\n",	7,		ShuntingYardInt::ParseResult::Success },
    { "12 * 34 / 56\n",		7,		ShuntingYardInt::ParseResult::Success },
    { "12 * (56 / 5)\n",	132,	ShuntingYardInt::ParseResult::Success },
    { "((12+34)*56)/78-90\n",-57,	ShuntingYardInt::ParseResult::Success },
    { "1399/43/5\n",		6,		ShuntingYardInt::ParseResult::Success },
    { "(1399/43)/5\n",		6,		ShuntingYardInt::ParseResult::Success },
    { "1399/(43/5)\n",		174,	ShuntingYardInt::ParseResult::Success },
    { "+\n",				0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { "/\n",				0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { "*\n",				0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { "(123 + 456\n",		0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { "/123\n",				0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { "123/\n",				0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { "*123\n",				0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { "123*\n",				0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { "123-\n",				0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { "+123\n",				0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { "123+\n",				0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { "(123\n",				0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { "123(\n",				0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { ")123\n",				0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { "123)\n",				0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { "123 + 456)\n",		0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { "(123 + 456))\n",		0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { ")123 + 456\n",		0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { "123( + 456)\n",		0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { "123 + () + 456)\n",	0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { "123 +/ 456\n",		0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { "1 + 2147483648\n",	0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { "1 + -2147483649\n",	0,		ShuntingYardInt::ParseResult::InvalidExpression },
    { "1/0\n",				0,		ShuntingYardInt::ParseResult::DivisionByZero },
    { "5/(2/3)\n",			0,		ShuntingYardInt::ParseResult::DivisionByZero }
};

bool shunting_yard_test1()
{
	bool result = true;
    ShuntingYard<int> shunting_yard;

	for (auto& test : shunting_yard_test_array)
	{
        ShuntingYardInt::Result parse_result = shunting_yard.parse(test.expr.data(), test.expr.length());
		if (parse_result.first != test.parse_result)
		{
			std::cerr << "Test1 for expression '" << test.expr << "' failed." << std::endl;
			result = false;
		}
        else if (parse_result.first == ShuntingYardInt::ParseResult::Success && parse_result.second != test.result)
		{
			std::cerr << "Test1 for expression '" << test.expr << "' failed." << std::endl;
			result = false;
		}
        if (test.parse_result == ShuntingYardInt::ParseResult::Incomplete)
		{
			shunting_yard.clear();
		}
	}

	if (result)
	{
		std::cout << "Test1 passed" << std::endl;
	}

	return result;
}

bool shunting_yard_test2_aux(const char* s, size_t length, int expected_result)
{
    ShuntingYard<int> shunting_yard;
	int result = 0;

	for (size_t step = 1; step <= length; ++step)
	{
		size_t start = 0;
		while (start < length)
		{
            ShuntingYardInt::Result r = shunting_yard.parse(&s[start], start + step - 1 < length ? step : length - start);
			if (start + step < length)
			{
                if (r.first != ShuntingYardInt::ParseResult::Incomplete)
				{
					return false;
				}
			}
			else
			{
                if (r.first != ShuntingYardInt::ParseResult::Success && r.second != expected_result)
				{
					return false;
				}
				break;
			}

			start += step;
		}
	}

	return true;
}

bool shunting_yard_test2()
{
	std::string s1 = "53/(17-(19+23))*11-(31+(37+83))+113\n";
	bool result1 = shunting_yard_test2_aux(s1.data(), s1.length(), -60);
	if (!result1)
	{
		std::cerr << "Test2 for expression '" << s1 << "' failed." << std::endl;
	}

	std::string s2 = "53 	 / (  17 -  (  19 + 23 ) ) * 11  -	 (31 +  (  37   +  83 ) )  + 113  \n";
	bool result2 = shunting_yard_test2_aux(s2.data(), s2.length(), -60);
	if (!result2)
	{
		std::cerr << "Test2 for expression '" << s1 << "' failed." << std::endl;
	}

	if (result1 && result2)
	{
		std::cout << "Test2 passed" << std::endl;
	}

	return result1 && result2;
}

bool shunting_yard_test3()
{
    std::string s = "(2 + 3) * 7 / 11\n(109 - 53) * 17 / 19\n103/((67 - 43) / 7)\n";

    ShuntingYard<int> shunting_yard;
    ShuntingYardInt::Result result{shunting_yard.parse(s.data(), s.length())};
    if (result.first != ShuntingYardInt::ParseResult::Success || result.second != 3 || shunting_yard.is_empty())
    {
        std::cerr << "Test3 for first call failed" << std::endl;
        return false;
    }

    result = shunting_yard.parse(nullptr, 0);
    if (result.first != ShuntingYardInt::ParseResult::Success || result.second != 50 || shunting_yard.is_empty())
    {
        std::cerr << "Test3 for second call failed" << std::endl;
        return false;
    }

    result = shunting_yard.parse(nullptr, 0);
    if (result.first != ShuntingYardInt::ParseResult::Success || result.second != 34 || !shunting_yard.is_empty())
    {
        std::cerr << "Test3 for third call failed" << std::endl;
        return false;
    }

    std::cout << "Test3 passed" << std::endl;
    return true;
}

const std::function<bool()> tests[] =
{
    shunting_yard_test1,
    shunting_yard_test2,
    shunting_yard_test3
};

int main()
{
    bool result{true};

    for (auto& test: tests)
    {
        if (!test())
        {
            result = false;
        }
    }

    return result ? 0 : 1;
}
