# Net calculator
NetCalculator is an application used to long infix arithmetic expressions evaluation.
NetCalculator is a server multithreaded application that keeps several simultaneous connections.

## Main highlights
NetCalculator:
 - accepts TCP-connections;
 - receives an infix arithmetic expression, evaluates a result and sends it back;
 - evaluates an expression on the run;
 - uses integer arithmetic (5/2 -> 2, 2/3 -> 0);
 - supports operations: +, -, *, /;
 - supports '(' and ')';
 - skips symbols '\t', '\r' and ' ' in a received expression;
 - considers '\n' as end of expression;
 - adds '\n' to each response;
 - uses 2-stack modification of Shunting-yard Dijkstra's algorithm;
 - can receive unified expression during several receive operations;
 - can receive and process several expressions during one receive operation;
 - doesn't close connection after sending correct result;
 - can process several simultaneous connections (depend on input parameter '-c');
 - can start several threads (depend on input parameter '-t');
 - implements event-driven approach;

## Info
It uses basic [C++14](https://isocpp.org/wiki/faq/cpp14-language) syntax.
It uses [boost](https://www.boost.org/) library. Version 1.65.1 is used.
[CMake](https://cmake.org/) is the chosen build system using [ctest](https://cmake.org/Wiki/CMake/Testing_With_CTest).
Also bash is used for a unit-test.
NetCalculagtor was build in Linux Ubuntu 18.0.4 and macOS 10.14.1

##  Project structure
| folder       | Content              |
| ------------ | -------------------- |
| [/lib](/lib) | Shunting-yard library |
| [/lib/include](/lib/include) | Shunting-yard library includes |
| [/lib/perf](/lib/perf) | A tool to check performance of Shunting-yard library |
| [/lib/test](/lib/test) | Unit-tests for Shunting-yard library |
| [/app](/app) | NetCalculator application |
| [/app/test](/app/test) | A unit-test for NetCalculator application |
| [/gen](/gen) | Random infix arithmetic expression generator |

## Get project
Go to working directory and do command.
```shell
git clone https://github.com/Egorov-Alexey/NetCalculator.git
```

## Generate project
Choose any free non-system port and set variable NC_FREE_TEST_PORT (a unit-test needs it).
```shell
export NC_FREE_TEST_PORT=8080
```

Specify build type debug/release
```shell
  # go to NetCalculator directory
  cd NetCalculator
  # generate a debug project
  cmake -H. -BBuild -DCMAKE_BUILD_TYPE=Debug
  # or generate a release project
  cmake -H. -BBuild -DCMAKE_BUILD_TYPE=Release
```

## Build
From the NetCalculator/Build folder
```shell
  cmake --build .
```

## Run tests
From the NetCalculator/Build folder
```shell
  ctest -V
```

## How to run?
Run with --help parameter and learn program options.
```shell
./NetCalculatorApp --help
General options:
  -h [ --help ]         Show help
  -a [ --address ] arg  Listen address (default value is 127.0.0.1)
  -p [ --port ] arg     Listen port
  -c [ --clients ] arg  Maximum number of simultaneous clients
  -t [ --threads ] arg  Number of threads (default value is
						hardware_concurrency() (1 if not computable))
```

Choose:
 - address (you can use 0.0.0.0, 127.0.0.1 or address of network interface);
 - any free non-system port (>=1024);
 - number of simultaneous connections you want to proceed;
 - how many threads will process these connections.
```shell
./NetCalculatorApp -a 0.0.0.0 -p 8080 -c 10 -t 2
```

Default value for 'address' parameter is '127.0.0.1'.
Default value for 'threads' parameter is std::hardware_concurrency() or 1 is value is not computable.
'threads' parameter can not exceed 'clients' parameter.

For simple testing you can use telnet.
```shell
telnet 127.0.0.1 8080
```
Input an infix expression and press 'Enter' and you will see a result.

Also you can generate a very long expression.
```shell
./NetCalculatorGen 1024
```
This command will generate a 1GB expression and will output it to the standard output stream.
You can store it to a file:
```shell
./NetCalculatorGen 1024 > expr_1gb
```

You can send this expression to NetCalculator using 'cat' and 'nc' commands.
```shell
cat expr_1gb | nc 127.0.0.1 8080
```
Definitely you can make requests from a remote computer (but don't use 127.0.0.1 for it).

## How to stop?
NetCalculator catches SIGINT and SIGTERM signals.
You can use Ctrl-C or kill command.

## Error processing
If an invalid expression was detected string "Invalid expression\n" would be writen to a socket.
After that NetDetect closes a connection.
If division by zero was detected string "Division by zero\n" would be writen to a socket.
After that NetDetect closes a connection.

## What is type of operands?
[int] (-2147483648 to 2147483647) type is used:
```cpp
ShuntingYard<int> shunting_yard;
```
Yoy can switch to another type.

## IPv4 and/or IPv6?
NetCalculator supports IPv4 only.
If you need to support IPv6 modify class NetCalcCore.

## How to check performance of ShuntingYard?
- generate a long expression using ExpressionGenerator;
- run ShuntingYardPerf and remember a result;
- make some changes in the algorithm;
- run ShuntingYardPerf again and compare results.

## Known issues
- NetCalculatorApp does not use any BitInt library and does not check a result overflow.
- ExpressionGenerator generates expressions that slightly higher than a requested size.
- ExpressionGenerator does not generate operation '/' because random expression generation is used and it often generates expressions like '(5/(2/3))'. NetCalculatorApp returns 'division by zero' for a such expression.
- ShuntingYard algorithm does not support explicit positive numbers. E.g. (7 + +5)
