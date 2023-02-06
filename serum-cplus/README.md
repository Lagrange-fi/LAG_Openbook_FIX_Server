# Environment
OS: Ubuntu Linux 18.04 LTS  
IP: 185.95.16.202  
SSH user: user  
SSH pass: MBDzNq3q2h  
SSH por

```bash
ssh user@185.95.16.202 -p 2222
```

# Prerequisites
```bash
sudo apt install build-essential cmake autoconf automake libtool-bin pkg-config zlib1g-dev libssl-dev libcurl4-openssl-dev libgmp-dev unzip curl gcc-6 g++-6
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-6 50 --slave /usr/bin/g++ g++ /usr/bin/g++-6
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 60 --slave /usr/bin/g++ g++ /usr/bin/g++-7
sudo update-alternatives --config gcc
```

## Boost
```bash
curl -LO https://sourceforge.net/projects/boost/files/boost/1.79.0/boost_1_79_0.tar.gz
tar -xzf boost_1_79_0.tar.gz && cd boost_1_79_0/
./bootstrap.sh
sudo ./b2 install
sudo ldconfig
```
## POCO
```bash
curl -LO https://github.com/pocoproject/poco/archive/poco-1.9.4-release.tar.gz
tar -xzf poco-1.9.4-release.tar.gz && cd poco-poco-1.9.4-release/
mkdir cmake-build && cd cmake-build/ && cmake ..
sudo cmake --build . --target install
sudo ldconfig
```
## Fix8
```bash
curl -L https://github.com/fix8/fix8/archive/1.4.1.tar.gz -o fix8-1.4.1.tar.gz
tar -xzf fix8-1.4.1.tar.gz && cd fix8-1.4.1/
./bootstrap && ./configure && make
sudo make install
sudo ldconfig

#compile error: need  fix https://stackoverflow.com/questions/46916875/error-when-building-fix-8
#You have to explicitly #include <functional> in logger.hpp.

# If an error occurs with the linking of the hftest library, then after running the commands "./bootstrap && ./configure" comment out the lines in fix8-1.4.1/test/Makefile :
# bin_PROGRAMS = hftest$(EXEEXT) hfprint$(EXEEXT) $(am__EXEEXT_1)
# hftest_LDFLAGS = $(ALL_LIBS) -lhftest $(am__append_13) $(am__append_18)
# hfprint_LDFLAGS = $(ALL_LIBS) -lhftest $(am__append_14) $(am__append_19)
# then continue installing the library, starting with "make" command
```
## Websocketpp
```bash
curl -L https://github.com/zaphoyd/websocketpp/archive/refs/tags/0.8.2.tar.gz -o websocketpp-0.8.2.tar.gz
tar -xzf websocketpp-0.8.2.tar.gz && cd websocketpp-0.8.2
cmake .
sudo make install
```

## GMP
```bash
wget https://gmplib.org/download/gmp/gmp-6.2.1.tar.xz
tar -xpJf gmp-6.2.1.tar.xz && cd gmp-6.2.1
./configure && make && make check
sudo make install
```

<!-- ## Nacl
```bash
wget https://hyperelliptic.org/nacl/nacl-20110221.tar.bz2
bunzip2 < nacl-20110221.tar.bz2 | tar -xf -
cd nacl-20110221
./do
sudo cp -i ./build/KOMPUTER/bin/* /usr/bin/
mv ./build/KOMPUTER/include/amd64 ./build/KOMPUTER/include/nacl  && sudo cp -ir ./build/KOMPUTER/include/* /usr/include/
mv ./build/KOMPUTER/lib/amd64 ./build/KOMPUTER/lib/nacl && sudo cp -ir ./build/KOMPUTER/lib/* /usr/lib/
``` -->

## CryptoPP
```bash
wget  https://www.cryptopp.com/cryptopp860.zip
mkdir cryptopp && cd cryptopp && mv ../cryptopp860.zip .
unzip cryptopp860.zip
make && make test
sudo make install
```

# Cloning and compiling the source
```bash
git clone --recurse-submodules https://github.com/TradingForge/SerumFixServer.git
cd serum-cplus
mkdir build && cd build
cmake ..
cmake --build . --config Debug --target all -- -j 6
ctest
```

```
f8c -p FIX8_SERUM -n SERUM  -c server SerumFIX44.xml SerumFIX44.xml
```

# CMake ExternalProject module examples
```cmake
include(ExternalProject)

set(3RD_PARTY_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/3rd_party")

ExternalProject_Add(
    POCO
    URL https://github.com/pocoproject/poco/archive/poco-1.9.4-release.tar.gz
    PREFIX ${CMAKE_BINARY_DIR}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND mkdir cmake-build && cd cmake-build && cmake -DCMAKE_INSTALL_PREFIX:PATH=${3RD_PARTY_INSTALL_PREFIX} ..
    BUILD_COMMAND cd cmake-build && cmake --build . --target install)

ExternalProject_Add(
    Fix8
    URL https://github.com/fix8/fix8/archive/1.4.1.tar.gz
    PREFIX ${CMAKE_BINARY_DIR}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND /bin/sh -c "./bootstrap"
    COMMAND /bin/sh -c "CPPFLAGS='-std=c++17 -I${3RD_PARTY_INSTALL_PREFIX}/include' LDFLAGS=-L${3RD_PARTY_INSTALL_PREFIX}/lib ./configure --prefix=${3RD_PARTY_INSTALL_PREFIX}"
    BUILD_COMMAND /bin/sh -c "CPPFLAGS='-std=c++17 -I${3RD_PARTY_INSTALL_PREFIX}/include' LDFLAGS=-L${3RD_PARTY_INSTALL_PREFIX}/lib LD_LIBRARY_PATH=${3RD_PARTY_INSTALL_PREFIX}/lib make"
    INSTALL_COMMAND /bin/sh -c "CPPFLAGS='-std=c++17 -I${3RD_PARTY_INSTALL_PREFIX}/include' LDFLAGS=-L${3RD_PARTY_INSTALL_PREFIX}/lib LD_LIBRARY_PATH=${3RD_PARTY_INSTALL_PREFIX}/lib make install")

add_dependencies(Fix8 POCO)

string(CONCAT FIXSERVER_NAME "${PROJECT_NAME}" "lib")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_library("${FIXSERVER_NAME}" STATIC
    calc.cpp)

add_dependencies("${FIXSERVER_NAME}" Fix8)

target_include_directories("${FIXSERVER_NAME}" PRIVATE "${3RD_PARTY_INSTALL_PREFIX}/include")
target_link_libraries("${FIXSERVER_NAME}" STATIC "${3RD_PARTY_INSTALL_PREFIX}/lib/libfix8.a")

target_include_directories("${FIXSERVER_NAME}" PRIVATE "cmake-build-release/serum_cplus/include")
```