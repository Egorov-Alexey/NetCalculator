/**
 * This file contains program that generates long arithmetical infix expressions.
 */

#include <iostream>
#include <limits>
#include <random>

inline unsigned int number_of_digits(int value)
{
    unsigned int n = value < 0 ? 1 : 0;

    do
    {
        ++n;
        value /= 10;
    } while (value);

    return n;
}

char get_operator(int index)
{
    switch (index)
    {
        case 0: return '+';
        case 1: return '-';
        case 2: return '*';
        case 3: return '/';
    }
    return ' ';
}

void generate_random_expr(std::ostream& stream, unsigned int length)
{
    unsigned int open_brackets(0);
    std::default_random_engine random_engine(std::random_device{}());
    std::uniform_int_distribution<> distr_0_1(0, 1);
    std::uniform_int_distribution<> number_distr(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    std::uniform_int_distribution<> operation_distr(0, 2);
    unsigned int i = 0;

    while (i < length)
    {
        //1. Which entity will be generated open bracket or number?
        while (!distr_0_1(random_engine))
        {
            ++open_brackets;
            ++i;
            stream << "(";
        }

        //2. Generate number
        int rnd = number_distr(random_engine);
        i += number_of_digits(rnd);
        stream << rnd;

        //3. Which entity will be generated close bracket or operator?
        while (open_brackets && !distr_0_1(random_engine))
        {
            --open_brackets;
            ++i;
            stream << ")";
        }

        //4. Generate operator
        stream << get_operator(operation_distr(random_engine));
        ++i;
    }

    //Finalize
    stream << number_distr(random_engine);
    for (i = 0; i < open_brackets; ++i) { stream << ')'; }
    stream << '\n';
}

int main(int argc, const char *argv[])
{
    if (argc == 1)
    {
        std::cout << "Random arithmetic expression generator. " << std::endl <<
            "Please provide size of expression in MB." << std::endl;
        return 1;
    }

    int n = 0;
    try
    {
        n = std::stoi(argv[1]);
    }
    catch (...)
    {
        std::cerr << "Invalid size." << std::endl;
        return 1;
    }

    if (n <=0)
    {
        std::cerr << "Invalid size." << std::endl;
        return 1;
    }

    generate_random_expr(std::cout, n * 1024 * 1024);

    return 0;
}
