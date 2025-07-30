# Top-level Makefile for tcp-udp project (cross-platform: Linux/macOS)

# Output directories
BIN_DIR := bin
LIB_DIR := $(BIN_DIR)
OBJ_DIR := build

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
    SO_EXT := dylib
    ACE_INCLUDE := /opt/homebrew/include
    ACE_LIBDIR := /opt/homebrew/lib
    RPATH_FLAG := -Wl,-rpath,$(BIN_DIR)
else
    SO_EXT := so
    ACE_INCLUDE := /usr/local/include
    ACE_LIBDIR := /usr/local/lib
    RPATH_FLAG := -Wl,-rpath=$(BIN_DIR)
endif

CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -g -O0 -Wno-deprecated-declarations -fvisibility=hidden -fPIC -MMD -MP
COMPILE_COMMANDS_FLAGS := -std=c++17 -Wall -Wextra -g -O0 -Wno-deprecated-declarations -fvisibility=hidden -fPIC -MMD -MP
LDFLAGS += $(RPATH_FLAG) -L$(ACE_LIBDIR)

INCLUDES := \
  -Iinclude \
  -Isrc \
  -I$(ACE_INCLUDE)

# Libraries
ACE_LIB := ACE
GTEST_LIBS := -lgtest -lgtest_main -lpthread

# Google Test options
GTEST_OPTS :=

# ================================
# Phony Targets
.PHONY: all clean tests proxytest tcptest udptest notificationtest bin build\
 	clang-format clang-format-fix install-clang-format \
 	clang-tidy clang-tidy-fix install-clang-tidy \
 	install-bear compile-commands

# ================================
# Directory Creation
$(BIN_DIR) $(OBJ_DIR):
	@mkdir -p $@

# ================================
# Library Build Rules
# liblogger
LIBLOGGER_SRCS := $(shell find src/liblogger -name '*.cpp')
LIBLOGGER_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(LIBLOGGER_SRCS))
LIBLOGGER := $(BIN_DIR)/liblogger.$(SO_EXT)
$(LIBLOGGER): $(LIBLOGGER_OBJS) | $(BIN_DIR)
	$(CXX) -shared -o $@ $^ $(LDFLAGS) -L$(BIN_DIR)

# libacetools
LIBACETOOLS_SRCS := $(shell find src/libacetools -name '*.cpp')
LIBACETOOLS_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(LIBACETOOLS_SRCS))
LIBACETOOLS := $(BIN_DIR)/libacetools.$(SO_EXT)
$(LIBACETOOLS): $(LIBACETOOLS_OBJS) | $(BIN_DIR)
	$(CXX) -shared -o $@ $^ $(LDFLAGS) -l$(ACE_LIB)

# libtcp
LIBTCP_SRCS := $(shell find src/libtcp -name '*.cpp')
LIBTCP_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(LIBTCP_SRCS))
LIBTCP := $(BIN_DIR)/libtcp.$(SO_EXT)
$(LIBTCP): $(LIBTCP_OBJS) $(LIBACETOOLS) | $(BIN_DIR)
	$(CXX) -shared -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -lacetools -l$(ACE_LIB)

# libudp
LIBUDP_SRCS := $(shell find src/libudp -name '*.cpp')
LIBUDP_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(LIBUDP_SRCS))
LIBUDP := $(BIN_DIR)/libudp.$(SO_EXT)
$(LIBUDP): $(LIBUDP_OBJS) $(LIBACETOOLS) | $(BIN_DIR)
	$(CXX) -shared -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -lacetools -l$(ACE_LIB)

# libtcpclientproxy
LIBTCPCLIENTPROXY_SRCS := $(shell find src/libtcpclientproxy -name '*.cpp')
LIBTCPCLIENTPROXY_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(LIBTCPCLIENTPROXY_SRCS))
LIBTCPCLIENTPROXY := $(BIN_DIR)/libtcpclientproxy.$(SO_EXT)
$(LIBTCPCLIENTPROXY): $(LIBTCPCLIENTPROXY_OBJS) $(LIBACETOOLS) $(LIBTCP) $(LIBUDP) | $(BIN_DIR)
	$(CXX) -shared -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -ltcp -ludp -lacetools -l$(ACE_LIB)

# libtcpserverproxy
LIBTCPSERVERPROXY_SRCS := $(shell find src/libtcpserverproxy -name '*.cpp')
LIBTCPSERVERPROXY_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(LIBTCPSERVERPROXY_SRCS))
LIBTCPSERVERPROXY := $(BIN_DIR)/libtcpserverproxy.$(SO_EXT)
$(LIBTCPSERVERPROXY): $(LIBTCPSERVERPROXY_OBJS) $(LIBACETOOLS) $(LIBTCP) $(LIBUDP) | $(BIN_DIR)
	$(CXX) -shared -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -ltcp -ludp -lacetools -l$(ACE_LIB)

# libargsparser
LIBARGSPARSER_SRCS := $(shell find src/libargsparser -name '*.cpp')
LIBARGSPARSER_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(LIBARGSPARSER_SRCS))
LIBARGSPARSER := $(BIN_DIR)/libargsparser.$(SO_EXT)
$(LIBARGSPARSER): $(LIBARGSPARSER_OBJS) $(LIBACETOOLS) | $(BIN_DIR)
	$(CXX) -shared -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -lacetools

# ================================
# Executable Build Rules
# tcpclient executable
TCPCLIENT_APP_SRCS := $(shell find src/tcpclient -name '*.cpp')
TCPCLIENT_APP_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(TCPCLIENT_APP_SRCS))
TCPCLIENT := $(BIN_DIR)/tcpclient
$(TCPCLIENT): $(TCPCLIENT_APP_OBJS) $(LIBTCP) $(LIBARGSPARSER) $(LIBACETOOLS) $(LIBLOGGER) | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -ltcp -largsparser -lacetools -llogger

# tcp_echo_server executable
TCP_ECHO_SERVER_APP_SRCS := $(shell find src/tcp_echo_server -name '*.cpp')
TCP_ECHO_SERVER_APP_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(TCP_ECHO_SERVER_APP_SRCS))
TCP_ECHO_SERVER := $(BIN_DIR)/tcp_echo_server
$(TCP_ECHO_SERVER): $(TCP_ECHO_SERVER_APP_OBJS) $(LIBTCP) $(LIBARGSPARSER) $(LIBACETOOLS) $(LIBLOGGER) | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -ltcp -largsparser -lacetools -llogger

# udp_echo_server executable
UDP_ECHO_SERVER_APP_SRCS := $(shell find src/udp_echo_server -name '*.cpp')
UDP_ECHO_SERVER_APP_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(UDP_ECHO_SERVER_APP_SRCS))
UDP_ECHO_SERVER := $(BIN_DIR)/udp_echo_server
$(UDP_ECHO_SERVER): $(UDP_ECHO_SERVER_APP_OBJS) $(LIBUDP) $(LIBARGSPARSER) $(LIBACETOOLS) $(LIBLOGGER) | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -ludp -largsparser -lacetools -llogger

# tcpclientproxy executable
TCPCLIENTPROXY_APP_SRCS := $(shell find src/tcpclientproxy -name '*.cpp')
TCPCLIENTPROXY_APP_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(TCPCLIENTPROXY_APP_SRCS))
TCPCLIENTPROXY := $(BIN_DIR)/tcpclientproxy
$(TCPCLIENTPROXY): $(TCPCLIENTPROXY_APP_OBJS) $(LIBTCPCLIENTPROXY) $(LIBTCP) $(LIBUDP) $(LIBARGSPARSER) $(LIBACETOOLS) $(LIBLOGGER) | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -ltcpclientproxy -ltcp -ludp -largsparser -lacetools -llogger

# tcpserverproxy executable
TCPSERVERPROXY_APP_SRCS := $(shell find src/tcpserverproxy -name '*.cpp')
TCPSERVERPROXY_APP_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(TCPSERVERPROXY_APP_SRCS))
TCPSERVERPROXY := $(BIN_DIR)/tcpserverproxy
$(TCPSERVERPROXY): $(TCPSERVERPROXY_APP_OBJS) $(LIBTCPSERVERPROXY) $(LIBTCP) $(LIBUDP) $(LIBARGSPARSER) $(LIBACETOOLS) $(LIBLOGGER) | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -ltcpserverproxy -ltcp -ludp -largsparser -lacetools -llogger

# notifiable_client executable
NOTIFIABLE_CLIENT_APP_SRCS := $(shell find src/notifiable_client -name '*.cpp')
NOTIFIABLE_CLIENT_APP_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(NOTIFIABLE_CLIENT_APP_SRCS))
NOTIFIABLE_CLIENT := $(BIN_DIR)/notifiable_client
$(NOTIFIABLE_CLIENT): $(NOTIFIABLE_CLIENT_APP_OBJS) $(LIBTCP) $(LIBARGSPARSER) $(LIBACETOOLS) $(LIBLOGGER) | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -ltcp -largsparser -lacetools -llogger

# tests executable
TEST_SRCS := $(shell find src/tests -name '*.cpp')
TEST_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(TEST_SRCS))
TESTS := $(BIN_DIR)/tests
$(TESTS): $(TEST_OBJS) $(LIBTCP) $(LIBUDP) $(LIBACETOOLS) $(LIBTCPCLIENTPROXY) $(LIBTCPSERVERPROXY) $(LIBARGSPARSER) $(LIBLOGGER) | $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS) -L$(BIN_DIR) -ltcp -ludp -ltcpclientproxy -ltcpserverproxy -largsparser -lacetools -llogger $(GTEST_LIBS)

# ================================
# Clang-format Installation Target
install-clang-format:
	@echo "Installing clang-format..."
	@./scripts/clang/install_clang_format.sh || echo "clang-format installation failed, but build will continue"

# Clang-tidy Installation Target
install-clang-tidy:
	@echo "Installing clang-tidy..."
	@./scripts/clang/install_clang_tidy.sh || echo "clang-tidy installation failed, but build will continue"

CLANG_FORMAT_SCRIPT := ./scripts/clang/run_clang_format.sh

# Clang-format target (check formatting without modifying files)
clang-format:
	$(CLANG_FORMAT_SCRIPT) --check

# Clang-format with fixes target (apply formatting)
clang-format-fix:
	$(CLANG_FORMAT_SCRIPT) --fix

CLANG_TIDY_SCRIPT := ./scripts/clang/run_clang_tidy.sh

clang-tidy:
	$(CLANG_TIDY_SCRIPT) --no-fix

clang-tidy-fix:
	$(CLANG_TIDY_SCRIPT) --fix

# ================================
# Compile Database Management

# Install bear for generating compile_commands.json
install-bear:
	@./scripts/clang/install_bear.sh

# Generate compile_commands.json using bear
compile-commands:
	@./scripts/clang/generate_compile_commands.sh

# ================================
# Default Target: Build Everything
all: clang-format-fix $(LIBACETOOLS) $(LIBTCP) $(LIBUDP) $(LIBTCPCLIENTPROXY) $(LIBTCPSERVERPROXY) \
     $(TCPCLIENT) $(TCP_ECHO_SERVER) $(UDP_ECHO_SERVER) $(TCPCLIENTPROXY) $(TCPSERVERPROXY) $(NOTIFIABLE_CLIENT) $(TESTS)

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
	./$(TESTS) $(GTEST_OPTS)

# Run only the proxy chain test
proxytest: $(TESTS)
	GTEST_FILTER=ProxyChainTest.* ./$(TESTS) $(GTEST_OPTS)

# Run only the TCP client/server test
tcptest: $(TESTS)
	GTEST_FILTER=TcpClientServerTest.* ./$(TESTS) $(GTEST_OPTS)

# Run only the UDP client/server test
udptest: $(TESTS)
	GTEST_FILTER=UdpClientServerTest.* ./$(TESTS) $(GTEST_OPTS)

# Run only the multi-client notification test
notificationtest: $(TESTS)
	GTEST_FILTER=MultiClientNotificationTest.* ./$(TESTS) $(GTEST_OPTS)

# Run only the large message test
largemessagetest: $(TESTS)
	GTEST_FILTER=LargeMessageTest.* ./$(TESTS) $(GTEST_OPTS)

killall:
	sudo lsof -i :15001 -i :15002 -i :15003 -sTCP:LISTEN -t | xargs -r sudo kill -9

# Clean rule
clean: killall
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Clean and rebuild everything
rebuild: clean all