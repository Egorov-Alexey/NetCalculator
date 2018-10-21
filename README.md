# Net calculator
NetCalculator is used for calculation of long arithmetic expressions.

## Info
It uses basic [C++14](https://isocpp.org/wiki/faq/cpp14-language) syntax, but nothing really complicated.
It uses [boost](https://www.boost.org/) library. Version 1.65.1 was used.
[CMake](https://cmake.org/) is the chosen build system using [ctest](https://cmake.org/Wiki/CMake/Testing_With_CTest).
Also bash is used for a unit-test.

##  Project structure
| folder       | Content              |
| ------------ | -------------------- |
| [/lib](/lib) | Shunting Yard library |
| [/lib/perf](/lib/perf) | A tool for check performance of Shunting Yard library |
| [/lib/include](/lib/include) | Shunting Yard library includes |
| [/lib/test](/lib/test) | Unit-tests for Shunting Yard library |
| [/app](/app) | NetCalculator application |
| [/app/test](/app/test) | A unit-test for NetCalculator application |
| [/gen](/gen) | Random arithmetic expression generator |

## Generate project
Choose any free non-system port and set variable NC_FREE_TEST_PORT (unit-test needs it).
```shell
export NC_FREE_TEST_PORT=8080
```

Specify build type debug/release
```shell
  # generate a debug project
  cmake -H. -BBuild -DCMAKE_BUILD_TYPE=Debug
  # generate a release project
  cmake -H. -BBuild -DCMAKE_BUILD_TYPE=Release
```

## Build
From the Build folder

```shell
  cmake --build .
```

## Run tests
From the Build folder

```shell
  ctest -V
```

## How to run?

Just run with --help parameter and learn program options:
```shell
./NetCalculatorApp --help
Parameter 'port' is not defined.
Parameter 'clients' is not defined.
General options:
  -h [ --help ]         Show help
  -p [ --port ] arg     Listen port
  -c [ --clients ] arg  Maximum number of simultaneous clients
  -t [ --threads ] arg  Number of threads (default value is
                        std::thread::hardware_concurrency())
```

Choose any free non-system port, number of simultaneous connections you want to proceed and how many threads will process these connections:
```shell
./NetCalculatorApp -p 8080 -c 10 -t 2
```
Application will wait connections on loopback (127.0.0.1) address.

For simple testing you can use telnet:
```shell
telnet 127.0.0.1 8080
```
Just input expression and press 'Enter'.

Also you can generate a very big expression:
```shell
./NetCalculatorGen 1024
```
This command will generate a 1GB expression and will output it to the standard output stream.
You can store an expression to a file:
```shell
./NetCalculatorGen 1048576 > expr_1gb
```

For hard testing you can use nc:
```shell
cat expr_1gb | nc 127.0.0.1 8080
```
Definitely you can make requests from remote computer.

## Error processing

If an invalid expression was detected string "Invalid expression" would be writen to a socket.
If division by zero was detected string "Division by zero" would be writen to a socket.

## What is type of operands?

[int] (-2147483648 to 2147483647) type is used.
If you need to support an another type in class ShuntingYard just choose an another type at this line:
'using Type = int;'
But changing class ShuntingYard to template class is better way.

## How to make template class ShuntingYard?

- rename file ShuntingYard.cpp to ShuntingYard.tpp (or *.ipp);
- include ShuntingYard.tpp at the end of file ShuntingYard.tpp;
- delete 'using Type = int;'
- change class ShuntingYard to template class: 'template <class Type> class ShuntingYard'
- use it with necessary type 'ShuntingYard<size_t> shunting_yard;'

## IPv4 and/or IPv6?

NetCalculator supports IPv4 only.
If you need to support IPv6 modify class NetCalcCore.

## How to check performance of ShuntingYard?

- generate a big expression using ExpressionGenerator;
- run ShuntingYardPerf and remember a result;
- make some changes in teh algorithm;
- run ShuntingYardPerf again and compare results;

## Known issues

- NetCalculatorApp does not use any BitInt library and does not check result overflow.
- ExpressionGenerator generate expressions that slightly higher than a requested size.
- ExpressionGenerator generate does not generate operation '/' because random expression generation is used and it often generates expressions like '(5/(2/3))'. NetCalculatorApp returns 'division by zero' for a such expression.
- ShuntingYard algorithm does not support explicit positive numbers. E.g. (7 + +5)
