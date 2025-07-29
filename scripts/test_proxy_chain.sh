#!/bin/bash

# Test script for the Notification Proxy Chain
# This script tests the proxy chain by sending messages to each server proxy

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Port configuration (matching the setup script)
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

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to test a proxy endpoint
test_proxy() {
    local port=$1
    local test_name=$2
    local message="hello from $test_name"
    
    print_status "Testing $test_name on port $port..."
    
    # Send message and capture response
    response=$(echo "$message" | nc -w 5 localhost $port 2>/dev/null)
    
    if [ $? -eq 0 ] && [ ! -z "$response" ]; then
        print_success "$test_name: Sent '$message', received '$response'"
        return 0
    else
        print_error "$test_name: Failed to get response from port $port"
        return 1
    fi
}

# Function to test with the tcpclient executable
test_with_tcpclient() {
    print_status "Testing with tcpclient executable..."
    
    if [ -f "bin/tcpclient" ]; then
        # Note: The tcpclient is hardcoded to connect to port 12345
        # We would need to modify it to accept command line arguments
        print_status "tcpclient executable found, but it's hardcoded to port 12345"
        print_status "To test with tcpclient, you would need to modify it to accept target port"
    else
        print_error "tcpclient executable not found"
    fi
}

# Main test function
main() {
    print_status "Testing Notification Proxy Chain..."
    echo
    
    # Test each server proxy
    test_proxy $TCP_SERVER_PROXY1_PORT "Server Proxy 1"
    test_proxy $TCP_SERVER_PROXY2_PORT "Server Proxy 2" 
    test_proxy $TCP_SERVER_PROXY3_PORT "Server Proxy 3"
    
    echo
    test_with_tcpclient
    
    echo
    print_status "Test completed!"
    print_status "If all tests passed, your proxy chain is working correctly!"
}

# Check if netcat is available
if ! command -v nc &> /dev/null; then
    print_error "netcat (nc) is not installed. Please install it to run this test."
    exit 1
fi

# Run the test
main 