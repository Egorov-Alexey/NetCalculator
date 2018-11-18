# CMake build : main application

#configure variables
set (APP_NAME         "${PROJECT_NAME}App")
set (CONFIG_LIB_NAME  "${PROJECT_NAME}ConfigLib")
set (NETCORE_LIB_NAME "${PROJECT_NAME}NetCoreLib")

#configure directories
set (APP_MODULE_PATH   "${PROJECT_SOURCE_DIR}/app")
set (APP_SRC_PATH      "${APP_MODULE_PATH}/src" )
set (APP_INCLUDE_PATH  "${APP_MODULE_PATH}/include" )

#set includes
include_directories (${APP_INCLUDE_PATH} ${LIBRARY_INCLUDE_PATH} ${Boost_INCLUDE_DIRS})

#set sources
set(APP_SOURCE_FILES         "${APP_SRC_PATH}/NetCalculator.cpp")
set(CONFIG_LIB_SOURCE_FILES  "${APP_SRC_PATH}/Config.cpp")
set(NETCORE_LIB_SOURCE_FILES "${APP_SRC_PATH}/NetCalcCore.cpp")

#set library
add_library (${CONFIG_LIB_NAME}  STATIC ${CONFIG_LIB_SOURCE_FILES})
add_library (${NETCORE_LIB_NAME} STATIC ${NETCORE_LIB_SOURCE_FILES})

#set target executable
add_executable (${APP_NAME} ${APP_SOURCE_FILES})

#add the library
target_link_libraries (${APP_NAME} ${CONFIG_LIB_NAME} ${NETCORE_LIB_NAME} Threads::Threads ${Boost_SYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})

#test
enable_testing ()
add_subdirectory (test)
