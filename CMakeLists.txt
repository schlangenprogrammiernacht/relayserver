cmake_minimum_required (VERSION 3.2)
project (RelayServer VERSION 0.1 LANGUAGES CXX)

add_subdirectory(lib/TcpServer/TcpServer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall -pedantic")

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

include_directories(
	${CMAKE_PROJECT_NAME}
	lib/msgpack-c/include/
	lib/TcpServer/TcpServer/include/
)

add_executable(
	${CMAKE_PROJECT_NAME}
	relayserver/main.cpp
)

target_link_libraries(
	${CMAKE_PROJECT_NAME}
	tcpserver
)
