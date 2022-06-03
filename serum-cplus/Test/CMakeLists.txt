
find_package(Threads REQUIRED)
find_package(Boost 1.79.0 REQUIRED)
find_package(Websocketpp REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED) 

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
    )

add_executable("SerumTradeTest" SerumTradeTest.cpp)
target_include_directories("SerumTradeTest" PRIVATE
        ${DEX_INCLUDES}
        ${CMAKE_SOURCE_DIR}/SerumDEX
        )
target_link_libraries("SerumTradeTest" PRIVATE
    ${DEX_LIBRARIES}
    ${CMAKE_SOURCE_DIR}/build/SerumDEX/libfixserverSerumDEX.a
    )