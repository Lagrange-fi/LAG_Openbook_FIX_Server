
find_package(Threads REQUIRED)
find_package(Poco REQUIRED Util XML JSON Net Foundation)
find_package(Fix8 REQUIRED)
find_package(P7 REQUIRED)
find_package(Boost 1.79.0 REQUIRED)
find_package(Websocketpp REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)

#message(STATUS P7_INCLUDE_DIRS:${P7_INCLUDE_DIRS})
#message(STATUS P7_LIBRARY_STATIC:${P7_LIBRARY_STATIC})

target_include_directories(${PROJECT_NAME} PRIVATE
        ".."
        ${FIX8_INCLUDE_DIRS}
        ${P7_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIR}
        )

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/build/serum/libfixserverserum.a
    ${CMAKE_SOURCE_DIR}/build/SerumApp/libfixserverSerumApp.a
#    ${PROJECT_BINARY_DIR}/serum
#    ${MARKETLIB_NAME}
#    ${SHAREDLIB_NAME}
    ${CMAKE_THREAD_LIBS_INIT}
    PocoUtil
    PocoXML
    PocoJSON
    PocoNet
    PocoFoundation
    ${FIX8_LIBRARY}
    ${P7_LIBRARY_STATIC}
    ${OPENSSL_CRYPTO_LIBRARY}
    ${OPENSSL_LIBRARIES}
    )






