# CMake build : random arithmetic expression generator

#configure variables
set (APP_NAME "${PROJECT_NAME}Gen")

#configure directories
set (APP_MODULE_PATH "${PROJECT_SOURCE_DIR}/gen")
set (APP_SRC_PATH  "${APP_MODULE_PATH}/src" )

#set sources
file (GLOB APP_SOURCE_FILES "${APP_SRC_PATH}/*.cpp")

#set target executable
add_executable (${APP_NAME} ${APP_SOURCE_FILES})

#add the library
target_link_libraries (${APP_NAME})
