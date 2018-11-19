# CMake build : perf application

#configure variables
set (PERF_APP_NAME "${LIB_NAME}Perf")

#configure directories
set (PERF_MODULE_PATH "${LIBRARY_MODULE_PATH}/perf")

#configure perf directories
set (PERF_SRC_PATH  "${PERF_MODULE_PATH}/src" )

#set includes
include_directories (${LIBRARY_INCLUDE_PATH} ${Boost_INCLUDE_DIRS})

#set perf sources
file (GLOB PERF_SOURCE_FILES "${PERF_SRC_PATH}/*.cpp")

#set target executable
add_executable (${PERF_APP_NAME} ${PERF_SOURCE_FILES})

#add the library
target_link_libraries (${PERF_APP_NAME})
