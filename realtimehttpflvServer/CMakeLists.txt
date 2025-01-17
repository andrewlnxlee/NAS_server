cmake_minimum_required(VERSION 3.10.2)
project(L4-httpflvServer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_BUILD_TYPE DEBUG)
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

find_package(Boost REQUIRED COMPONENTS asio)

set(INCLUDE_DIR /usr/local/include)
set(LIB_DIR /usr/local/lib)
include_directories(${INCLUDE_DIR})
link_directories(${LIB_DIR})

add_executable(L4-httpflvServer
        main.cpp
		Utils/Config.cpp
		Server/BoostServer.cpp
		Server/HttpServerConnection.cpp
        )
TARGET_LINK_LIBRARIES(L4-httpflvServer jsoncpp  asio pthread)






