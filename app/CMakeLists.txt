# CMake build : main application

#configure variables
set (APP_NAME "${PROJECT_NAME}App")

#configure directories
set (APP_MODULE_PATH "${PROJECT_SOURCE_DIR}/app")
set (APP_SRC_PATH  "${APP_MODULE_PATH}/src" )
set (APP_INCLUDE_PATH  "${APP_MODULE_PATH}/include" )

#set includes
include_directories (${APP_INCLUDE_PATH} ${LIBRARY_INCLUDE_PATH} ${Boost_INCLUDE_DIRS})

#set sources
file (GLOB APP_SOURCE_FILES "${APP_SRC_PATH}/*.cpp")

#set target executable
add_executable (${APP_NAME} ${APP_SOURCE_FILES})

#add the library
target_link_libraries (${APP_NAME} ${LIB_NAME} Threads::Threads ${Boost_SYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})

#test
enable_testing ()
add_subdirectory (test)
