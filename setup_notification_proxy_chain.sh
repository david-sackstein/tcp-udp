#!/bin/bash

# Setup Notification Proxy Chain
# This script sets up a proxy chain similar to the Notification test:
# 3 TCP clients -> 3 server proxies -> 1 client proxy -> 1 TCP echo server

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Port configuration (matching NotificationTest.h)
TCP_CLIENT1_PORT=16000
TCP_CLIENT2_PORT=16001
TCP_CLIENT3_PORT=16002
TCP_CLIENT_PROXY_PORT=17002
TCP_SERVER_PORT=17003
TCP_SERVER_PROXY1_PORT=17001
TCP_SERVER_PROXY2_PORT=17004
TCP_SERVER_PROXY3_PORT=17005

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if a port is in use
check_port() {
    local port=$1
    if lsof -i :$port >/dev/null 2>&1; then
        print_warning "Port $port is already in use"
        return 1
    fi
    return 0
}

# Function to kill processes on specific ports
kill_ports() {
    print_status "Killing any existing processes on our ports..."
    for port in $TCP_SERVER_PORT $TCP_CLIENT_PROXY_PORT $TCP_SERVER_PROXY1_PORT $TCP_SERVER_PROXY2_PORT $TCP_SERVER_PROXY3_PORT; do
        if lsof -i :$port >/dev/null 2>&1; then
            sudo lsof -i :$port -sTCP:LISTEN -t | xargs -r sudo kill -9
            print_status "Killed processes on port $port"
        fi
    done
}

# Function to wait for a service to be ready
wait_for_service() {
    local port=$1
    local service_name=$2
    local max_attempts=30
    local attempt=1
    
    print_status "Waiting for $service_name on port $port..."
    while [ $attempt -le $max_attempts ]; do
        if nc -z localhost $port 2>/dev/null; then
            print_success "$service_name is ready on port $port"
            return 0
        fi
        sleep 1
        attempt=$((attempt + 1))
    done
    print_error "$service_name failed to start on port $port"
    return 1
}

# Function to cleanup on exit
cleanup() {
    print_status "Cleaning up..."
    if [ ! -z "$TCP_SERVER_PID" ]; then
        kill $TCP_SERVER_PID 2>/dev/null || true
    fi
    if [ ! -z "$CLIENT_PROXY_PID" ]; then
        kill $CLIENT_PROXY_PID 2>/dev/null || true
    fi
    if [ ! -z "$SERVER_PROXY1_PID" ]; then
        kill $SERVER_PROXY1_PID 2>/dev/null || true
    fi
    if [ ! -z "$SERVER_PROXY2_PID" ]; then
        kill $SERVER_PROXY2_PID 2>/dev/null || true
    fi
    if [ ! -z "$SERVER_PROXY3_PID" ]; then
        kill $SERVER_PROXY3_PID 2>/dev/null || true
    fi
    kill_ports
}

# Set up cleanup trap
trap cleanup EXIT

# Check if executables exist
if [ ! -f "bin/tcp_echo_server" ]; then
    print_error "tcp_echo_server executable not found. Please run 'make all' first."
    exit 1
fi

if [ ! -f "bin/tcpclientproxy" ]; then
    print_error "tcpclientproxy executable not found. Please run 'make all' first."
    exit 1
fi

if [ ! -f "bin/tcpserverproxy" ]; then
    print_error "tcpserverproxy executable not found. Please run 'make all' first."
    exit 1
fi

if [ ! -f "bin/tcpclient" ]; then
    print_error "tcpclient executable not found. Please run 'make all' first."
    exit 1
fi

print_success "All executables found"

# Kill any existing processes
kill_ports

print_status "Setting up Notification Proxy Chain..."
print_status "Architecture: 3 TCP clients -> 3 server proxies -> 1 client proxy -> 1 TCP echo server"

# Start TCP echo server
print_status "Starting TCP echo server on port $TCP_SERVER_PORT..."
./bin/tcp_echo_server 127.0.0.1:$TCP_SERVER_PORT &
TCP_SERVER_PID=$!
wait_for_service $TCP_SERVER_PORT "TCP Echo Server"

# Start client proxy
print_status "Starting TCP client proxy on port $TCP_CLIENT_PROXY_PORT..."
print_status "Client proxy forwards to TCP server on port $TCP_SERVER_PORT..."
./bin/tcpclientproxy 127.0.0.1:$TCP_CLIENT_PROXY_PORT 127.0.0.1:$TCP_SERVER_PORT &
CLIENT_PROXY_PID=$!
wait_for_service $TCP_CLIENT_PROXY_PORT "TCP Client Proxy"

# Start server proxies
print_status "Starting TCP server proxy 1 on port $TCP_SERVER_PROXY1_PORT..."
./bin/tcpserverproxy 127.0.0.1:$TCP_SERVER_PROXY1_PORT 127.0.0.1:$TCP_CLIENT_PROXY_PORT &
SERVER_PROXY1_PID=$!
wait_for_service $TCP_SERVER_PROXY1_PORT "TCP Server Proxy 1"

print_status "Starting TCP server proxy 2 on port $TCP_SERVER_PROXY2_PORT..."
./bin/tcpserverproxy 127.0.0.1:$TCP_SERVER_PROXY2_PORT 127.0.0.1:$TCP_CLIENT_PROXY_PORT &
SERVER_PROXY2_PID=$!
wait_for_service $TCP_SERVER_PROXY2_PORT "TCP Server Proxy 2"

print_status "Starting TCP server proxy 3 on port $TCP_SERVER_PROXY3_PORT..."
./bin/tcpserverproxy 127.0.0.1:$TCP_SERVER_PROXY3_PORT 127.0.0.1:$TCP_CLIENT_PROXY_PORT &
SERVER_PROXY3_PID=$!
wait_for_service $TCP_SERVER_PROXY3_PORT "TCP Server Proxy 3"

print_success "All proxy components started successfully!"

# Display the proxy chain architecture
echo
print_status "Proxy Chain Architecture:"
echo "┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐"
echo "│   TCP Client 1  │───▶│ Server Proxy 1   │───▶│                 │"
echo "│   Port: $TCP_CLIENT1_PORT   │    │ Port: $TCP_SERVER_PROXY1_PORT │    │                 │"
echo "└─────────────────┘    └──────────────────┘    │                 │"
echo "┌─────────────────┐    ┌──────────────────┐    │                 │"
echo "│   TCP Client 2  │───▶│ Server Proxy 2   │───▶│  Client Proxy   │───▶│  TCP Echo Server │"
echo "│   Port: $TCP_CLIENT2_PORT   │    │ Port: $TCP_SERVER_PROXY2_PORT │    │ Port: $TCP_CLIENT_PROXY_PORT │    │ Port: $TCP_SERVER_PORT │"
echo "└─────────────────┘    └──────────────────┘    │                 │"
echo "┌─────────────────┐    ┌──────────────────┐    │                 │"
echo "│   TCP Client 3  │───▶│ Server Proxy 3   │───▶│                 │"
echo "│   Port: $TCP_CLIENT3_PORT   │    │ Port: $TCP_SERVER_PROXY3_PORT │    │                 │"
echo "└─────────────────┘    └──────────────────┘    └─────────────────┘"
echo

print_status "All services are running. You can now:"
print_status "1. Test with the tcpclient executable:"
print_status "   ./bin/tcpclient"
print_status "2. Test with netcat:"
print_status "   echo 'hello' | nc localhost $TCP_SERVER_PROXY1_PORT"
print_status "   echo 'hello' | nc localhost $TCP_SERVER_PROXY2_PORT"
print_status "   echo 'hello' | nc localhost $TCP_SERVER_PROXY3_PORT"
print_status "3. Press Ctrl+C to stop all services"

# Keep the script running
print_status "Proxy chain is running. Press Ctrl+C to stop..."
wait 