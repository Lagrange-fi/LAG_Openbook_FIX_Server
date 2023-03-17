set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library("${SERUM_DEX_NAME}" STATIC
        SerumMD.cpp
        SerumTrade.cpp
        PoolRequester/InstrumentJSON.cpp
        PoolRequester/InstrumentsJSON.cpp
        PoolRequester/PoolsRequester.cpp
        SerumMarket/UserCredentials.cpp
        SerumMarket/FileSettings.cpp
        SerumMarket/Market.cpp
)

# find_package(Fix8 REQUIRED)
# find_package(P7 REQUIRED)
# find_package(GMP REQUIRED)

target_include_directories(${SERUM_DEX_NAME} PRIVATE
        "include"
        # "rapidjson"
        # "sol_sdk"
        ".."
        # ${FIX8_INCLUDE_DIRS}
        # ${P7_INCLUDE_DIRS}
        )