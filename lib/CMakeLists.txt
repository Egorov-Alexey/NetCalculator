# CMake build : library

#configure variables
set (LIB_NAME "ShuntingYardLib")

#configure directories
set (LIBRARY_MODULE_PATH "${PROJECT_SOURCE_DIR}/lib")
set (LIBRARY_INCLUDE_PATH  "${LIBRARY_MODULE_PATH}/include")

#export vars
set (LIBRARY_INCLUDE_PATH  ${LIBRARY_INCLUDE_PATH} PARENT_SCOPE)

#perf
add_subdirectory (perf)

#test
enable_testing ()
add_subdirectory (test)
