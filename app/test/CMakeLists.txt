# CMake build : main application test

#configure variables
set (TEST1_APP_NAME "${APP_NAME}_SimpleTest")
set (TEST2_APP_NAME "${APP_NAME}_NetCalcCoreTest")
set (TEST3_APP_NAME "${APP_NAME}_ConfigTest")

#configure directories
set (TEST_MODULE_PATH "${APP_MODULE_PATH}/test")

#configure test directories
set (TEST_SRC_PATH  "${TEST_MODULE_PATH}/src" )

#set includes
include_directories (${APP_INCLUDE_PATH})

#set test sources
set(TEST1_SOURCE_FILES "${TEST_SRC_PATH}/NetCalculatorAppTest.cpp")
set(TEST2_SOURCE_FILES "${APP_SRC_PATH}/NetCalcCore.cpp" "${TEST_SRC_PATH}/NetCalcCoreTest.cpp")
set(TEST3_SOURCE_FILES "${APP_SRC_PATH}/Config.cpp" "${TEST_SRC_PATH}/ConfigTest.cpp")

#set target executable
add_executable (${TEST1_APP_NAME} ${TEST1_SOURCE_FILES})
add_executable (${TEST2_APP_NAME} ${TEST2_SOURCE_FILES})
add_executable (${TEST3_APP_NAME} ${TEST3_SOURCE_FILES})

#add the library
target_link_libraries (${TEST1_APP_NAME} ${Boost_SYSTEM_LIBRARY})
target_link_libraries (${TEST2_APP_NAME} ${LIB_NAME} Threads::Threads ${Boost_SYSTEM_LIBRARY})
target_link_libraries (${TEST3_APP_NAME} ${Boost_SYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})

# Turn on CMake testing capabilities
enable_testing ()
if((DEFINED ENV{NC_FREE_TEST_PORT}) AND (BASH_PROGRAM))
    add_test(NAME ${TEST1_APP_NAME} COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/script/test.sh $ENV{NC_FREE_TEST_PORT} WORKING_DIRECTORY ${CMAKE_BINARY_DIR} )
endif()

add_test (NAME ${TEST2_APP_NAME} COMMAND ${TEST2_APP_NAME} )
add_test (NAME ${TEST3_APP_NAME} COMMAND ${TEST3_APP_NAME} )
