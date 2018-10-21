# CMake build : main application test

#configure variables
set (TEST_APP_NAME "${APP_NAME}Test")

#configure directories
set (TEST_MODULE_PATH "${APP_MODULE_PATH}/test")

#configure test directories
set (TEST_SRC_PATH  "${TEST_MODULE_PATH}/src" )

#set test sources
file (GLOB TEST_SOURCE_FILES "${TEST_SRC_PATH}/*.cpp")

#set target executable
add_executable (${TEST_APP_NAME} ${TEST_SOURCE_FILES})

#add the library
target_link_libraries (${TEST_APP_NAME} ${Boost_SYSTEM_LIBRARY})

# Turn on CMake testing capabilities
enable_testing ()
if((DEFINED ENV{NC_FREE_TEST_PORT}) AND (BASH_PROGRAM))
    message(STATUS "MY DIR: " ${CMAKE_BINARY_DIR})
    add_test(NAME ${TEST_APP_NAME} COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/script/test.sh $ENV{NC_FREE_TEST_PORT} WORKING_DIRECTORY ${CMAKE_BINARY_DIR} )
endif()
