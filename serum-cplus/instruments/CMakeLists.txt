set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library("instruments" STATIC
        converter.cpp
)

# find_package(Fix8 REQUIRED)
# find_package(P7 REQUIRED)
# find_package(GMP REQUIRED)

# target_include_directories("base64" PRIVATE
#         "include"
#         ".."
#         )