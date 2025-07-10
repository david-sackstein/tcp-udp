cmake_minimum_required(VERSION 3.15)
project(tcp_udp_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")

add_compile_options(
        -Wall -Wextra -Werror -g -O0 -Wno-deprecated-declarations -fvisibility=hidden -Wno-error=attributes

)

# Include and link directories (adjust if needed)
include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src /opt/homebrew/include)
link_directories(/usr/lib /usr/local/lib /opt/homebrew/lib)

set(ACE_LIB ACE)

# ================================
# acetools library: owns ACE init/fini singleton
file(GLOB_RECURSE ACETOOLS_SOURCES src/libacetools/*.cpp)
file(GLOB_RECURSE ACETOOLS_HEADERS include/libacetools/*.h)
add_library(libacetools SHARED ${ACETOOLS_SOURCES})
target_include_directories(libacetools PUBLIC include/libacetools)
target_link_libraries(libacetools PRIVATE ${ACE_LIB})

# ================================
# udp library
file(GLOB_RECURSE UDP_SOURCES src/libudp/*.cpp)
file(GLOB_RECURSE UDP_HEADERS include/libudp/*.h)
add_library(libudp SHARED ${UDP_SOURCES})
target_include_directories(libudp PUBLIC include/libudp)
target_link_libraries(libudp PRIVATE libacetools ${ACE_LIB})

# ================================
# tcp library
file(GLOB_RECURSE TCP_SOURCES src/libtcp/*.cpp)
file(GLOB_RECURSE TCP_HEADERS include/libtcp/*.h)
add_library(libtcp SHARED ${TCP_SOURCES})
target_include_directories(libtcp PUBLIC include/libtcp)
target_link_libraries(libtcp PRIVATE libacetools ${ACE_LIB})

# ================================
# tcpclientproxy library
file(GLOB_RECURSE TCPCLIENTPROXY_SOURCES src/libtcpclientproxy/*.cpp)
file(GLOB_RECURSE TCPCLIENTPROXY_HEADERS include/libtcpclientproxy/*.h)
add_library(libtcpclientproxy SHARED ${TCPCLIENTPROXY_SOURCES})
target_include_directories(libtcpclientproxy PUBLIC include/libtcpclientproxy)
target_link_libraries(libtcpclientproxy PRIVATE libtcp libudp libacetools ${ACE_LIB})

# ================================
# tcpserverproxy library
file(GLOB_RECURSE TCPSERVERPROXY_SOURCES src/libtcpserverproxy/*.cpp)
file(GLOB_RECURSE TCPSERVERPROXY_HEADERS include/libtcpserverproxy/*.h)
add_library(libtcpserverproxy SHARED ${TCPSERVERPROXY_SOURCES})
target_include_directories(libtcpserverproxy PUBLIC include/libtcpserverproxy)
target_link_libraries(libtcpserverproxy PRIVATE libtcp libudp libacetools ${ACE_LIB})

# ================================
# argsparser library
file(GLOB_RECURSE ARGSPARSER_SOURCES src/libargsparser/*.cpp)
file(GLOB_RECURSE ARGSPARSER_HEADERS include/libargsparser/*.h)
add_library(libargsparser SHARED ${ARGSPARSER_SOURCES})
target_include_directories(libargsparser PUBLIC include/libargsparser)
target_link_libraries(libargsparser PRIVATE libacetools)

# ================================
# tcpclient executable
file(GLOB_RECURSE TCPCLIENT_SOURCES src/tcpclient/*.cpp)
add_executable(tcpclient ${TCPCLIENT_SOURCES})
target_link_libraries(tcpclient PRIVATE libtcp)

# ================================
# tcp_echo_server executable
file(GLOB_RECURSE TCP_ECHO_SERVER_SOURCES src/tcp_echo_server/*.cpp)
add_executable(tcp_echo_server ${TCP_ECHO_SERVER_SOURCES})
target_link_libraries(tcp_echo_server PRIVATE libtcp libargsparser)

# ================================
# udp_echo_server executable
file(GLOB_RECURSE UDP_ECHO_SERVER_SOURCES src/udp_echo_server/*.cpp)
add_executable(udp_echo_server ${UDP_ECHO_SERVER_SOURCES})
target_link_libraries(udp_echo_server PRIVATE libudp libargsparser)

# tcpclientproxy executable
file(GLOB_RECURSE TCPCLIENTPROXY_APP_SOURCES src/tcpclientproxy/*.cpp)
add_executable(tcpclientproxy ${TCPCLIENTPROXY_APP_SOURCES})
target_link_libraries(tcpclientproxy PRIVATE libtcpclientproxy libargsparser)

# tcpserverproxy executable
file(GLOB_RECURSE TCPSERVERPROXY_APP_SOURCES src/tcpserverproxy/*.cpp)
add_executable(tcpserverproxy ${TCPSERVERPROXY_APP_SOURCES})
target_link_libraries(tcpserverproxy PRIVATE libtcpserverproxy libargsparser)

# Tests executable
file(GLOB_RECURSE TEST_SOURCES src/tests/*.cpp)
add_executable(tests ${TEST_SOURCES})
target_include_directories(tests PRIVATE include)
target_link_libraries(tests
        PRIVATE
        libtcp
        libudp
        libtcpclientproxy
        libtcpserverproxy
        gtest
        gtest_main
        pthread
)

# ================================
# Output directories
set_target_properties(
        libacetools libtcp libudp libtcpclientproxy libtcpserverproxy libargsparser
        PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
)

set_target_properties(
        tcpclient tcp_echo_server udp_echo_server tcpclientproxy tcpserverproxy tests
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
)
