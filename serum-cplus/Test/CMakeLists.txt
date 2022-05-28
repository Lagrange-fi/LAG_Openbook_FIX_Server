
find_package(Threads REQUIRED)
find_package(Boost 1.79.0 REQUIRED)
find_package(Websocketpp REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(${SERUM_TEST} SerumAppTest.cpp)

#message(STATUS P7_INCLUDE_DIRS:${P7_INCLUDE_DIRS})
#message(STATUS P7_LIBRARY_STATIC:${P7_LIBRARY_STATIC})

target_include_directories(${SERUM_TEST} PRIVATE
        ".."
        ${CMAKE_SOURCE_DIR}/BrokerLib
        ${CMAKE_SOURCE_DIR}/Utility
        ${CMAKE_SOURCE_DIR}/SerumApp
        ${OPENSSL_INCLUDE_DIR}
        )

target_link_libraries(${SERUM_TEST} PRIVATE
    ${CMAKE_SOURCE_DIR}/build/SerumApp/libfixserverSerumApp.a
    ${CMAKE_THREAD_LIBS_INIT}
    ${OPENSSL_CRYPTO_LIBRARY}
    ${OPENSSL_LIBRARIES}
    )