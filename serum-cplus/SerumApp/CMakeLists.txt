set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library("${SERUM_DEX_NAME}" STATIC
        SerumApp.cpp
        #SerumListener.cpp
)

find_package(Fix8 REQUIRED)
find_package(P7 REQUIRED)

target_include_directories(${SERUM_DEX_NAME} PRIVATE
        "include"
        ".."
        ${FIX8_INCLUDE_DIRS}
        ${P7_INCLUDE_DIRS}
        )