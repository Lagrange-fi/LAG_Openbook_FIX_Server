
find_package(Threads REQUIRED)
find_package(Poco REQUIRED Util XML JSON Net Foundation)
find_package(Fix8 REQUIRED)
find_package(P7 REQUIRED)
find_package(Boost 1.79.0 REQUIRED)
find_package(Websocketpp REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)
add_executable(${PROJECT_NAME}_daemon daemon.cpp)

set(PROJECT_INCLUDES 
    ".."
    ${FIX8_INCLUDE_DIRS}
    ${P7_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
    )

set(PROJECT_LIBRARIES 
    ${CMAKE_SOURCE_DIR}/build/serum/libfixserverserum.a
    ${CMAKE_SOURCE_DIR}/build/SerumDEX/libfixserverSerumDEX.a
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
    ${CURL_LIBRARIES}
    )

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_INCLUDES}
    )

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${PROJECT_LIBRARIES}
    )


target_include_directories(${PROJECT_NAME}_daemon PRIVATE
    ${PROJECT_INCLUDES}
    )

target_link_libraries(${PROJECT_NAME}_daemon PRIVATE
    ${PROJECT_LIBRARIES}
)