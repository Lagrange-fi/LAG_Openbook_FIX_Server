set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library("${SERUM_NAME}" STATIC
        FIX8_SERUM_Data_classes.cpp
        FIX8_SERUM_Data_traits.cpp
        FIX8_SERUM_Data_types.cpp
        FIX8_SERUM_Order_classes.cpp
        FIX8_SERUM_Order_traits.cpp
        FIX8_SERUM_Order_types.cpp
        SERUM_Data_session.cpp
        # SERUM_Order_sandbox_session.cpp
        )

find_package(Fix8 REQUIRED)
find_package(P7 REQUIRED)

target_include_directories(${SERUM_NAME} PRIVATE
        "include"
        ".."
        ${FIX8_INCLUDE_DIRS}
        ${P7_INCLUDE_DIRS}
        )
