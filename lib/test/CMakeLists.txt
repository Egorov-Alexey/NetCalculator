# CMake build : library tests

#configure variables
set (TEST_APP_NAME "${LIB_NAME}Test")

#configure directories
set (TEST_MODULE_PATH "${LIBRARY_MODULE_PATH}/test")

#configure test directories
set (TEST_SRC_PATH  "${TEST_MODULE_PATH}/src" )

#set includes
include_directories (${LIBRARY_INCLUDE_PATH} ${Boost_INCLUDE_DIRS})

#set test sources
file (GLOB TEST_SOURCE_FILES "${TEST_SRC_PATH}/*.cpp")

#set target executable
add_executable (${TEST_APP_NAME} ${TEST_SOURCE_FILES})

#add the library
target_link_libraries (${TEST_APP_NAME})

# Turn on CMake testing capabilities
enable_testing()
add_test (NAME ${TEST_APP_NAME} COMMAND ${TEST_APP_NAME} )
