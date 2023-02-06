

set(CMAKE_VERBOSE_MAKEFILE ON)

cmake_minimum_required(VERSION 3.10)

project(fixserver
    LANGUAGES C CXX 
    VERSION 0.1.0)

# c++ 17
set (CMAKE_CXX_STANDARD 17)

include(ExternalProject)

ExternalProject_Add(
    P7
    PREFIX "${CMAKE_SOURCE_DIR}/libP7Client_v5.3"
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/libP7Client_v5.3"
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND /bin/sh build.sh
    INSTALL_COMMAND "")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
# set(GMP_DIR "/usr/local/")

string(CONCAT SERUM_NAME ${PROJECT_NAME} "serum")
string(CONCAT SERUM_DEX_NAME ${PROJECT_NAME} "SerumDEX")
string(CONCAT CLI_NAME ${PROJECT_NAME} "cli")
set(SHAREDLIB_NAME "shared")
set(MARKETLIB_NAME "market")

add_subdirectory(serum)
add_subdirectory(base64)
add_subdirectory(base58)
add_subdirectory(instruments)
add_subdirectory(sol_sdk)
add_subdirectory(SerumDEX)
add_subdirectory(marketlib)
add_subdirectory(sharedlib)
add_subdirectory(cli)
add_subdirectory(Test)