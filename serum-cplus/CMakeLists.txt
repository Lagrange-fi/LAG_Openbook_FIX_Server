cmake_minimum_required(VERSION 3.10)
project(fixserver
    LANGUAGES C CXX 
    VERSION 0.1.0)

include(ExternalProject)

message(CMAKE_SOURCE_DIR:${CMAKE_SOURCE_DIR}\n)

ExternalProject_Add(
    P7
    PREFIX "${CMAKE_SOURCE_DIR}/libP7Client_v5.3"
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/libP7Client_v5.3"
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND /bin/sh build.sh
    INSTALL_COMMAND "")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

message(CMAKE_MODULE_PATH:${CMAKE_MODULE_PATH}\n)

string(CONCAT SERUM_NAME ${PROJECT_NAME} "serum")
string(CONCAT CLI_NAME ${PROJECT_NAME} "cli")
set(SHAREDLIB_NAME "shared")
set(MARKETLIB_NAME "market")

# c++ 17
set (CMAKE_CXX_STANDARD 17)

# boost v. latest
#enable_testing()

add_subdirectory(serum)
add_subdirectory(marketlib)
add_subdirectory(sharedlib)
