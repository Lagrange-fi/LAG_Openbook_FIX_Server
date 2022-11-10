set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# find_package(GMP REQUIRED)

add_library("base58" STATIC
        base58.cpp
)