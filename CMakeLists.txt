cmake_minimum_required (VERSION 3.3)

project (NetCalculator)

set_property (GLOBAL PROPERTY USE_FOLDERS ON)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Boost 1.65.1 REQUIRED COMPONENTS system program_options)
find_program(BASH_PROGRAM bash)
if (NOT BASH_PROGRAM)
    message(STATUS "Bash was not found. Test NetCalculatorAppTest will be skipped.")
endif()

if(DEFINED ENV{NC_FREE_TEST_PORT})
    message(STATUS "Test port is set to: " $ENV{NC_FREE_TEST_PORT})
else()
    message(STATUS "Test port is not set. Test NetCalculatorAppTest will be skipped.")
endif()

add_subdirectory (lib)
add_subdirectory (app)
add_subdirectory (gen)

enable_testing ()
