# Installing dependencies

## Toolchains and related utilities
```bash
sudo apt install build-essential cmake autoconf automake libtool-bin pkg-config zlib1g-dev gcc-6 g++-6
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-6 50 --slave /usr/bin/g++ g++ /usr/bin/g++-6
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 60 --slave /usr/bin/g++ g++ /usr/bin/g++-7
sudo update-alternatives --config gcc
```

## Boost
Boost doesn't really need much introduction. Let's quote from their website.
> Boost provides free peer-reviewed portable C++ source libraries.  
>  
> We emphasize libraries that work well with the C++ Standard Library. Boost libraries are intended to be widely useful, and usable across a broad spectrum of applications. The Boost license encourages the use of Boost libraries for all users with minimal restrictions.  
>  
> We aim to establish "existing practice" and provide reference implementations so that Boost libraries are suitable for eventual standardization. Beginning with the ten Boost Libraries included in the Library Technical Report (TR1) and continuing with every release of the ISO standard for C++ since 2011, the C++ Standards Committee has continued to reply on Boost as a valuable source for additions to the Standard C++ Library.

### To install Boost follow the steps outlined below
```BOOST
curl -LO https://dl.bintray.com/boostorg/release/1.79.0/source/boost_1_79_0.tar.gz
tar -xzf boost_1_79_0.tar.gz && cd boost_1_79_0/
./bootstrap.sh
sudo ./b2 install
sudo ldconfig
```

## POCO
> The POCO C++ Libraries are powerful cross-platform C++ libraries for building network- and internet-based applications that run on desktop, server, mobile, IoT, and embedded systems. 

### To install The POCO C++ Libraries follow the steps outlined below
```bash
curl -LO https://github.com/pocoproject/poco/archive/poco-1.9.4-release.tar.gz
tar -xzf poco-1.9.4-release.tar.gz && cd poco-poco-1.9.4-release/
mkdir cmake-build && cd cmake-build/
cmake .. && cmake --build . --target install
sudo cmake --build . --target install
sudo ldconfig
```

## Fix8
> Fix8 Community Edition is the fastest C++ Open Source FIX framework. For the same message, Fix8 can significantly reduce encode and decode latency in your application.

### To install Fix8 follow the steps outlined below
```bash
curl -L https://github.com/fix8/fix8/archive/1.4.1.tar.gz -o fix8-1.4.1.tar.gz
tar -xzf fix8-1.4.1.tar.gz && cd fix8-1.4.1/
./bootstrap && ./configure && make
sudo make install
sudo ldconfig

#compile error: need  fix https://stackoverflow.com/questions/46916875/error-when-building-fix-8
#You have to explicitly #include <functional> in logger.hpp.


# Cloning and compiling the project's source
```bash
git clone --recurse-submodules https://github.com/TradingForge/SerumFixServer.git
cd serum-cplus
mkdir build && cd build
cmake ..
cmake --build . --config Debug --target all -- -j 6
```
