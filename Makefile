# Top-level Makefile for tcp-udp project (Linux version)

# ================================
# Project Metadata and Toolchain
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -g -O0 -Wno-deprecated-declarations -fvisibility=hidden -fPIC -MMD -MP
#LDFLAGS := -L/usr/local/lib

# Output directories
BIN_DIR := bin
LIB_DIR := $(BIN_DIR)
OBJ_DIR := build

LDFLAGS += -Wl,-rpath=$(BIN_DIR)

# Include directories
INCLUDES := \
  -Iinclude \
  -Isrc \
  -I/usr/local/include

# Libraries
ACE_LIB := ACE
GTEST_LIBS := -lgtest -lgtest_main -lpthread

# ================================
# Phony Targets
.PHONY: all clean tests proxytest tcptest udptest bin build

# ================================
# Directory Creation
$(BIN_DIR) $(OBJ_DIR):
	@mkdir -p $@

# ================================
# Library Build Rules
# libacetools
LIBACETOOLS_SRCS := $(shell find src/libacetools -name '*.cpp')
LIBACETOOLS_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(LIBACETOOLS_SRCS))
LIBACETOOLS := $(BIN_DIR)/libacetools.so
$(LIBACETOOLS): $(LIBACETOOLS_OBJS) | $(BIN_DIR)
	$(CXX) -shared -o $@ $^ $(LDFLAGS) -l$(ACE_LIB)

# libtcp
LIBTCP_SRCS := $(shell find src/libtcp -name '*.cpp')
LIBTCP_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(LIBTCP_SRCS))
LIBTCP := $(BIN_DIR)/libtcp.so
$(LIBTCP): $(LIBTCP_OBJS) $(LIBACETOOLS) | $(BIN_DIR)
	$(CXX) -shared -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -lacetools -l$(ACE_LIB)

# libudp
LIBUDP_SRCS := $(shell find src/libudp -name '*.cpp')
LIBUDP_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(LIBUDP_SRCS))
LIBUDP := $(BIN_DIR)/libudp.so
$(LIBUDP): $(LIBUDP_OBJS) $(LIBACETOOLS) | $(BIN_DIR)
	$(CXX) -shared -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -lacetools -l$(ACE_LIB)

# libtcpclientproxy
LIBTCPCLIENTPROXY_SRCS := $(shell find src/libtcpclientproxy -name '*.cpp')
LIBTCPCLIENTPROXY_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(LIBTCPCLIENTPROXY_SRCS))
LIBTCPCLIENTPROXY := $(BIN_DIR)/libtcpclientproxy.so
$(LIBTCPCLIENTPROXY): $(LIBTCPCLIENTPROXY_OBJS) $(LIBACETOOLS) | $(BIN_DIR)
	$(CXX) -shared -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -ltcp -ludp -lacetools -l$(ACE_LIB)

# libtcpserverproxy
LIBTCPSERVERPROXY_SRCS := $(shell find src/libtcpserverproxy -name '*.cpp')
LIBTCPSERVERPROXY_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(LIBTCPSERVERPROXY_SRCS))
LIBTCPSERVERPROXY := $(BIN_DIR)/libtcpserverproxy.so
$(LIBTCPSERVERPROXY): $(LIBTCPSERVERPROXY_OBJS) $(LIBACETOOLS) | $(BIN_DIR)
	$(CXX) -shared -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -ltcp -ludp -lacetools -l$(ACE_LIB)

# libargsparser
LIBARGSPARSER_SRCS := $(shell find src/libargsparser -name '*.cpp')
LIBARGSPARSER_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(LIBARGSPARSER_SRCS))
LIBARGSPARSER := $(BIN_DIR)/libargsparser.so
$(LIBARGSPARSER): $(LIBARGSPARSER_OBJS) $(LIBACETOOLS) | $(BIN_DIR)
	$(CXX) -shared -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -lacetools

# ================================
# Executable Build Rules
# tcpclient executable
TCPCLIENT_APP_SRCS := $(shell find src/tcpclient -name '*.cpp')
TCPCLIENT_APP_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(TCPCLIENT_APP_SRCS))
TCPCLIENT := $(BIN_DIR)/tcpclient
$(TCPCLIENT): $(TCPCLIENT_APP_OBJS) $(LIBTCP) $(LIBACETOOLS) | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -ltcp

# tcp_echo_server executable
TCP_ECHO_SERVER_APP_SRCS := $(shell find src/tcp_echo_server -name '*.cpp')
TCP_ECHO_SERVER_APP_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(TCP_ECHO_SERVER_APP_SRCS))
TCP_ECHO_SERVER := $(BIN_DIR)/tcp_echo_server
$(TCP_ECHO_SERVER): $(TCP_ECHO_SERVER_APP_OBJS) $(LIBTCP) $(LIBARGSPARSER) $(LIBACETOOLS) | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -ltcp -largsparser -lacetools

# udp_echo_server executable
UDP_ECHO_SERVER_APP_SRCS := $(shell find src/udp_echo_server -name '*.cpp')
UDP_ECHO_SERVER_APP_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(UDP_ECHO_SERVER_APP_SRCS))
UDP_ECHO_SERVER := $(BIN_DIR)/udp_echo_server
$(UDP_ECHO_SERVER): $(UDP_ECHO_SERVER_APP_OBJS) $(LIBUDP) $(LIBARGSPARSER) $(LIBACETOOLS) | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -ludp -largsparser -lacetools

# tcpclientproxy executable
TCPCLIENTPROXY_APP_SRCS := $(shell find src/tcpclientproxy -name '*.cpp')
TCPCLIENTPROXY_APP_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(TCPCLIENTPROXY_APP_SRCS))
TCPCLIENTPROXY := $(BIN_DIR)/tcpclientproxy
$(TCPCLIENTPROXY): $(TCPCLIENTPROXY_APP_OBJS) $(LIBTCPCLIENTPROXY) $(LIBARGSPARSER) $(LIBACETOOLS) | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -ltcpclientproxy -largsparser -lacetools

# tcpserverproxy executable
TCPSERVERPROXY_APP_SRCS := $(shell find src/tcpserverproxy -name '*.cpp')
TCPSERVERPROXY_APP_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(TCPSERVERPROXY_APP_SRCS))
TCPSERVERPROXY := $(BIN_DIR)/tcpserverproxy
$(TCPSERVERPROXY): $(TCPSERVERPROXY_APP_OBJS) $(LIBTCPSERVERPROXY) $(LIBARGSPARSER) $(LIBACETOOLS) | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -ltcpserverproxy -largsparser -lacetools

# tests executable
TEST_SRCS := $(shell find src/tests -name '*.cpp')
TEST_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(TEST_SRCS))
TESTS := $(BIN_DIR)/tests
$(TESTS): $(TEST_OBJS) $(LIBTCP) $(LIBUDP) $(LIBACETOOLS) $(LIBTCPCLIENTPROXY) $(LIBTCPSERVERPROXY) | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -ltcp -ludp -ltcpclientproxy -ltcpserverproxy $(GTEST_LIBS)

# ================================
# Default Target: Build Everything
all: $(LIBACETOOLS) $(LIBTCP) $(LIBUDP) $(LIBTCPCLIENTPROXY) $(LIBTCPSERVERPROXY) \
     $(TCPCLIENT) $(TCP_ECHO_SERVER) $(UDP_ECHO_SERVER) $(TCPCLIENTPROXY) $(TCPSERVERPROXY) $(TESTS)

# ================================
# Pattern Rule for Object Files
$(OBJ_DIR)/%.o: src/%.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# ================================
# Dependency Includes
-include $(LIBACETOOLS_OBJS:.o=.d) \
          $(LIBTCP_OBJS:.o=.d) $(LIBUDP_OBJS:.o=.d) \
          $(LIBTCPCLIENTPROXY_OBJS:.o=.d) $(LIBTCPSERVERPROXY_OBJS:.o=.d) \
          $(LIBARGSPARSER_OBJS:.o=.d) \
          $(TCPCLIENT_APP_OBJS:.o=.d) $(TCP_ECHO_SERVER_APP_OBJS:.o=.d) \
          $(UDP_ECHO_SERVER_APP_OBJS:.o=.d) $(TCPCLIENTPROXY_APP_OBJS:.o=.d) $(TCPSERVERPROXY_APP_OBJS:.o=.d) $(TEST_OBJS:.o=.d)

# ================================
# Test and Utility Rules
# Run tests

tests: $(TESTS)
	./$(TESTS)

# Run only the proxy chain test
proxytest: $(TESTS)
	GTEST_FILTER=ProxyChainTest.* ./$(TESTS)

# Run only the TCP client/server test
tcptest: $(TESTS)
	GTEST_FILTER=TcpClientServerTest.* ./$(TESTS)

# Run only the UDP client/server test
udptest: $(TESTS)
	GTEST_FILTER=UdpClientServerTest.* ./$(TESTS)

# Clean rule
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR)
