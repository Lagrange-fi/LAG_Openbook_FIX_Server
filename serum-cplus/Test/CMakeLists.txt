
find_package(Threads REQUIRED)
find_package(Boost 1.79.0 REQUIRED)
find_package(Websocketpp REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED) 
# find_package(GMP REQUIRED) 

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

set(DEX_INCLUDES 
        ".."
        ${CMAKE_SOURCE_DIR}/sharedlib
        ${CMAKE_SOURCE_DIR}/marketlib
        ${OPENSSL_INCLUDE_DIR}
        ${CURL_INCLUDE_DIRS}
        )

set(DEX_LIBRARIES 
        ${CMAKE_THREAD_LIBS_INIT}
        ${OPENSSL_CRYPTO_LIBRARY}
        ${OPENSSL_LIBRARIES}
        ${CURL_LIBRARIES}
        )


add_executable("SerumMDTest" SerumMDTest.cpp)
target_include_directories("SerumMDTest" PRIVATE
        ${DEX_INCLUDES}
        ${CMAKE_SOURCE_DIR}/SerumDEX
        )
target_link_libraries("SerumMDTest" PRIVATE
    ${DEX_LIBRARIES}
    ${CMAKE_SOURCE_DIR}/build/SerumDEX/libfixserverSerumDEX.a
    ${CMAKE_SOURCE_DIR}/build/instruments/libinstruments.a
    )

add_executable("SerumTradeTest" SerumTradeTest.cpp)
target_include_directories("SerumTradeTest" PRIVATE
        ${DEX_INCLUDES}
        ${CMAKE_SOURCE_DIR}/SerumDEX
        )
target_link_libraries("SerumTradeTest" PRIVATE
    ${DEX_LIBRARIES}
    ${CMAKE_SOURCE_DIR}/build/SerumDEX/libfixserverSerumDEX.a
    ${CMAKE_SOURCE_DIR}/build/instruments/libinstruments.a
    )

add_executable("SerumMarketTest" SerumMarketTest.cpp)
target_include_directories("SerumMarketTest" PRIVATE
    ${DEX_INCLUDES}
    ${CMAKE_SOURCE_DIR}/SerumDEX
    )
target_link_libraries("SerumMarketTest" PRIVATE
    ${DEX_LIBRARIES}
    ${CMAKE_SOURCE_DIR}/build/SerumDEX/libfixserverSerumDEX.a
    ${CMAKE_SOURCE_DIR}/build/base64/libbase64.a
    ${CMAKE_SOURCE_DIR}/build/base58/libbase58.a
    ${CMAKE_SOURCE_DIR}/build/sol_sdk/libsol_sdk.a
    ${CMAKE_SOURCE_DIR}/build/instruments/libinstruments.a
    /usr/local/lib/libgmp.a
    /usr/local/lib/libcryptopp.a
    /usr/local/lib/
   )

#    add_executable("Test"Test.cpp)
#    target_include_directories("Test" PRIVATE
#            ${DEX_INCLUDES}
#            ${CMAKE_SOURCE_DIR}/SerumDEX
#            )
#    target_link_libraries("Test" PRIVATE
#         ${DEX_LIBRARIES}
#         ${CMAKE_SOURCE_DIR}/build/SerumDEX/libfixserverSerumDEX.a
#         ${CMAKE_SOURCE_DIR}/build/base64/libbase64.a
#         ${CMAKE_SOURCE_DIR}/build/base58/libbase58.a
#         ${CMAKE_SOURCE_DIR}/build/sol_sdk/libsol_sdk.a
#         ${CMAKE_SOURCE_DIR}/build/instruments/libinstruments.a
#         /usr/local/lib/libgmp.a
#         /usr/local/lib/libcryptopp.a
#        )
   