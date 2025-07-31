#!/bin/bash

# =============================================================================
# Distributed Notification System Setup Script
# =============================================================================
# 
# This script sets up a complete distributed notification system with the
# following architecture:
#
# 3 Notifiable Clients -> 3 Client Proxies -> 1 Server Proxy -> 1 Notifying Server
#
# Architecture Overview:
# ┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
# │ Notifiable      │───▶│ Client Proxy 1   │───▶│                 │
# │ Client 1        │    │ (Port: 16001)    │    │                 │
# │ (Port: 16000)   │    └──────────────────┘    │                 │
# └─────────────────┘                            │                 │
# ┌─────────────────┐    ┌──────────────────┐    │                 │    ┌─────────────────┐
# │ Notifiable      │───▶│ Client Proxy 2   │───▶│  Server Proxy   │───▶│  Notifying      │
# │ Client 2        │    │ (Port: 16003)    │    │ (Port: 17000)   │    │  Server         │
# │ (Port: 16002)   │    └──────────────────┘    │                 │    │ (Port: 17001)   │
# └─────────────────┘                            │                 │    └─────────────────┘
# ┌─────────────────┐    ┌──────────────────┐    │                 │
# │ Notifiable      │───▶│ Client Proxy 3   │───▶│                 │
# │ Client 3        │    │ (Port: 16005)    │    │                 │
# │ (Port: 16004)   │    └──────────────────┘    │                 │
# └─────────────────┘                            └─────────────────┘
#
# Each client sends requests with its ID every second and receives
# notifications from the server about requests from other clients.
# =============================================================================

set -e  # Exit on any error

# =============================================================================
# COLOR DEFINITIONS FOR OUTPUT
# =============================================================================
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# =============================================================================
# PORT CONSTANTS - CLEARLY DEFINED FOR EASY UNDERSTANDING
# =============================================================================

# Client Application Ports
# Each notifiable client runs on its own port
NOTIFIABLE_CLIENT1_PORT=16000
NOTIFIABLE_CLIENT2_PORT=16002
NOTIFIABLE_CLIENT3_PORT=16004

# TCP Server Proxy Ports
# Each TCP server proxy listens for connections from its assigned client
# and forwards to the TCP client proxy
TCP_SERVER_PROXY1_PORT=16001
TCP_SERVER_PROXY2_PORT=16003
TCP_SERVER_PROXY3_PORT=16004

# TCP Client Proxy Port
# The TCP client proxy receives connections from all TCP server proxies
# and forwards to the notifying server
TCP_CLIENT_PROXY_PORT=17000

# Notifying Server Port
# The notifying server receives all requests and sends notifications
# to all connected clients
NOTIFYING_SERVER_PORT=17001

# =============================================================================
# CLIENT ID CONSTANTS
# =============================================================================
CLIENT1_ID="client1"
CLIENT2_ID="client2"
CLIENT3_ID="client3"

# =============================================================================
# REQUEST INTERVAL CONSTANTS
# =============================================================================
REQUEST_INTERVAL_MS=1000  # 1 second between requests

# =============================================================================
# OUTPUT FUNCTIONS
# =============================================================================

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

print_header() {
    echo -e "${PURPLE}==============================================================================${NC}"
    echo -e "${PURPLE}$1${NC}"
    echo -e "${PURPLE}==============================================================================${NC}"
}

print_subheader() {
    echo -e "${CYAN}$1${NC}"
}

# =============================================================================
# UTILITY FUNCTIONS
# =============================================================================

# Function to check if a port is in use
check_port() {
    local port=$1
    local service_name=$2
    if lsof -i :$port >/dev/null 2>&1; then
        print_warning "Port $port ($service_name) is already in use"
        return 1
    fi
    return 0
}

# Function to kill processes on specific ports
kill_ports() {
    print_status "Killing any existing processes on our ports..."
    local ports=(
        $NOTIFYING_SERVER_PORT
        $TCP_CLIENT_PROXY_PORT
        $TCP_SERVER_PROXY1_PORT
        $TCP_SERVER_PROXY2_PORT
        $TCP_SERVER_PROXY3_PORT
        $NOTIFIABLE_CLIENT1_PORT
        $NOTIFIABLE_CLIENT2_PORT
        $NOTIFIABLE_CLIENT3_PORT
    )
    
    for port in "${ports[@]}"; do
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

# Function to wait for a UDP service to be ready
wait_for_udp_service() {
    local port=$1
    local service_name=$2
    local max_attempts=30
    local attempt=1
    
    print_status "Waiting for $service_name on UDP port $port..."
    while [ $attempt -le $max_attempts ]; do
        if lsof -i UDP:$port >/dev/null 2>&1; then
            print_success "$service_name is ready on UDP port $port"
            return 0
        fi
        sleep 1
        attempt=$((attempt + 1))
    done
    print_error "$service_name failed to start on UDP port $port"
    return 1
}

# Function to cleanup on exit
cleanup() {
    print_header "CLEANING UP"
    print_status "Stopping all running processes..."
    
    # Kill all background processes
    if [ ! -z "$NOTIFYING_SERVER_PID" ]; then
        kill $NOTIFYING_SERVER_PID 2>/dev/null || true
        print_status "Stopped Notifying Server"
    fi
    if [ ! -z "$TCP_CLIENT_PROXY_PID" ]; then
        kill $TCP_CLIENT_PROXY_PID 2>/dev/null || true
        print_status "Stopped TCP Client Proxy"
    fi
    if [ ! -z "$TCP_SERVER_PROXY1_PID" ]; then
        kill $TCP_SERVER_PROXY1_PID 2>/dev/null || true
        print_status "Stopped TCP Server Proxy 1"
    fi
    if [ ! -z "$TCP_SERVER_PROXY2_PID" ]; then
        kill $TCP_SERVER_PROXY2_PID 2>/dev/null || true
        print_status "Stopped TCP Server Proxy 2"
    fi
    if [ ! -z "$TCP_SERVER_PROXY3_PID" ]; then
        kill $TCP_SERVER_PROXY3_PID 2>/dev/null || true
        print_status "Stopped TCP Server Proxy 3"
    fi
    if [ ! -z "$CLIENT1_PID" ]; then
        kill $CLIENT1_PID 2>/dev/null || true
        print_status "Stopped Notifiable Client 1"
    fi
    if [ ! -z "$CLIENT2_PID" ]; then
        kill $CLIENT2_PID 2>/dev/null || true
        print_status "Stopped Notifiable Client 2"
    fi
    if [ ! -z "$CLIENT3_PID" ]; then
        kill $CLIENT3_PID 2>/dev/null || true
        print_status "Stopped Notifiable Client 3"
    fi
    
    kill_ports
    print_success "Cleanup completed"
}

# Set up cleanup trap
trap cleanup EXIT

# =============================================================================
# VERBOSE FLAG HANDLING
# =============================================================================
VERBOSE_FLAG=""
for arg in "$@"; do
    if [[ "$arg" == "--verbose" ]]; then
        VERBOSE_FLAG="--verbose"
        print_status "Verbose mode enabled - showing all server/proxy logs"
        break
    fi
done
if [[ -z "$VERBOSE_FLAG" ]]; then
    print_status "Non-verbose mode - showing only client validation logs and server errors"
fi

# =============================================================================
# EXECUTABLE VALIDATION
# =============================================================================

print_header "VALIDATING EXECUTABLES"

# Check if all required executables exist
required_executables=(
    "bin/notifying_server"
    "bin/notifiable_client"
    "bin/tcpclientproxy"
    "bin/tcpserverproxy"
)

for executable in "${required_executables[@]}"; do
    if [ ! -f "$executable" ]; then
        print_error "$executable not found. Please run 'make all' first."
        exit 1
    fi
    print_success "Found $executable"
done

print_success "All executables found"

# =============================================================================
# PORT VALIDATION
# =============================================================================

print_header "VALIDATING PORTS"

# Check if any of our ports are already in use
ports_to_check=(
    "$NOTIFYING_SERVER_PORT:Notifying Server"
    "$TCP_CLIENT_PROXY_PORT:TCP Client Proxy"
    "$TCP_SERVER_PROXY1_PORT:TCP Server Proxy 1"
    "$TCP_SERVER_PROXY2_PORT:TCP Server Proxy 2"
    "$TCP_SERVER_PROXY3_PORT:TCP Server Proxy 3"
    "$NOTIFIABLE_CLIENT1_PORT:Notifiable Client 1"
    "$NOTIFIABLE_CLIENT2_PORT:Notifiable Client 2"
    "$NOTIFIABLE_CLIENT3_PORT:Notifiable Client 3"
)

for port_info in "${ports_to_check[@]}"; do
    IFS=':' read -r port service_name <<< "$port_info"
    check_port $port "$service_name" || true
done

# Kill any existing processes on our ports
kill_ports

# =============================================================================
# STARTING THE DISTRIBUTED NOTIFICATION SYSTEM
# =============================================================================

print_header "STARTING DISTRIBUTED NOTIFICATION SYSTEM"

print_subheader "Step 1: Starting Notifying Server"
print_status "Starting Notifying Server on port $NOTIFYING_SERVER_PORT..."
print_status "This server will receive requests from all clients and send notifications to all connected clients"
./bin/notifying_server 127.0.0.1:$NOTIFYING_SERVER_PORT $VERBOSE_FLAG &
NOTIFYING_SERVER_PID=$!
wait_for_service $NOTIFYING_SERVER_PORT "Notifying Server"

print_subheader "Step 2: Starting TCP Client Proxy"
print_status "Starting TCP Client Proxy on port $TCP_CLIENT_PROXY_PORT..."
print_status "TCP Client Proxy forwards connections to Notifying Server on port $NOTIFYING_SERVER_PORT"
./bin/tcpclientproxy 127.0.0.1:$TCP_CLIENT_PROXY_PORT 127.0.0.1:$NOTIFYING_SERVER_PORT $VERBOSE_FLAG &
TCP_CLIENT_PROXY_PID=$!
wait_for_udp_service $TCP_CLIENT_PROXY_PORT "TCP Client Proxy"

print_subheader "Step 3: Starting TCP Server Proxies"
print_status "Starting TCP Server Proxy 1 on port $TCP_SERVER_PROXY1_PORT..."
print_status "TCP Server Proxy 1 forwards to TCP Client Proxy on port $TCP_CLIENT_PROXY_PORT"
./bin/tcpserverproxy 127.0.0.1:$TCP_SERVER_PROXY1_PORT 127.0.0.1:$TCP_CLIENT_PROXY_PORT $VERBOSE_FLAG &
TCP_SERVER_PROXY1_PID=$!
wait_for_service $TCP_SERVER_PROXY1_PORT "TCP Server Proxy 1"

print_status "Starting TCP Server Proxy 2 on port $TCP_SERVER_PROXY2_PORT..."
print_status "TCP Server Proxy 2 forwards to TCP Client Proxy on port $TCP_CLIENT_PROXY_PORT"
./bin/tcpserverproxy 127.0.0.1:$TCP_SERVER_PROXY2_PORT 127.0.0.1:$TCP_CLIENT_PROXY_PORT $VERBOSE_FLAG &
TCP_SERVER_PROXY2_PID=$!
wait_for_service $TCP_SERVER_PROXY2_PORT "TCP Server Proxy 2"

print_status "Starting TCP Server Proxy 3 on port $TCP_SERVER_PROXY3_PORT..."
print_status "TCP Server Proxy 3 forwards to TCP Client Proxy on port $TCP_CLIENT_PROXY_PORT"
./bin/tcpserverproxy 127.0.0.1:$TCP_SERVER_PROXY3_PORT 127.0.0.1:$TCP_CLIENT_PROXY_PORT $VERBOSE_FLAG &
TCP_SERVER_PROXY3_PID=$!
wait_for_service $TCP_SERVER_PROXY3_PORT "TCP Server Proxy 3"

print_subheader "Step 4: Starting Notifiable Clients"
print_status "Starting Notifiable Client 1 (ID: $CLIENT1_ID)..."
print_status "Client 1 connects to TCP Server Proxy 1 on port $TCP_SERVER_PROXY1_PORT"
print_status "Client 1 will send requests every ${REQUEST_INTERVAL_MS}ms"
./bin/notifiable_client 127.0.0.1:$TCP_SERVER_PROXY1_PORT --client-id $CLIENT1_ID --interval $REQUEST_INTERVAL_MS $VERBOSE_FLAG &
CLIENT1_PID=$!

print_status "Starting Notifiable Client 2 (ID: $CLIENT2_ID)..."
print_status "Client 2 connects to TCP Server Proxy 2 on port $TCP_SERVER_PROXY2_PORT"
print_status "Client 2 will send requests every ${REQUEST_INTERVAL_MS}ms"
./bin/notifiable_client 127.0.0.1:$TCP_SERVER_PROXY2_PORT --client-id $CLIENT2_ID --interval $REQUEST_INTERVAL_MS $VERBOSE_FLAG &
CLIENT2_PID=$!

print_status "Starting Notifiable Client 3 (ID: $CLIENT3_ID)..."
print_status "Client 3 connects to TCP Server Proxy 3 on port $TCP_SERVER_PROXY3_PORT"
print_status "Client 3 will send requests every ${REQUEST_INTERVAL_MS}ms"
./bin/notifiable_client 127.0.0.1:$TCP_SERVER_PROXY3_PORT --client-id $CLIENT3_ID --interval $REQUEST_INTERVAL_MS $VERBOSE_FLAG &
CLIENT3_PID=$!

# =============================================================================
# SYSTEM STATUS DISPLAY
# =============================================================================

print_header "SYSTEM STATUS"

print_success "All components started successfully!"

echo
print_subheader "Distributed Notification System Architecture:"
echo
echo "┌────────────────────────────┐    ┌─────────────────────────┐    ┌──────────────────────┐"
echo "│ Notifiable Client 1        │───▶│                         │───▶│                      │"
echo "│ ID: client1                │    │ TCP Server Proxy 1      │    │                      │"
echo "│ Port: 16000                │    │ Port: 16001             │    │                      │"
echo "└────────────────────────────┘    └─────────────────────────┘    │                      │"
echo "┌────────────────────────────┐    ┌─────────────────────────┐    │                      │    ┌────────────────────┐"
echo "│ Notifiable Client 2        │───▶│                         │───▶│  TCP Client Proxy    │───▶│  Notifying Server  │"
echo "│ ID: client2                │    │ TCP Server Proxy 2      │    │  Port: 17000         │    │  Port: 17001       │"
echo "│ Port: 16002                │    │ Port: 16003             │    │                      │    └────────────────────┘"
echo "└────────────────────────────┘    └─────────────────────────┘    │                      │"
echo "┌────────────────────────────┐    ┌─────────────────────────┐    │                      │"
echo "│ Notifiable Client 3        │───▶│                         │───▶│                      │"
echo "│ ID: client3                │    │ TCP Server Proxy 3      │    │                      │"
echo "│ Port: 16004                │    │ Port: 16005             │    │                      │"
echo "└────────────────────────────┘    └─────────────────────────┘    └──────────────────────┘"
echo

print_subheader "System Behavior:"
echo "• Each tcp client sends requests with its ID every ${REQUEST_INTERVAL_MS}ms"
echo "• The tcp server responds to each request and sends notifications to all other clients"
echo "• All communication flows through the proxy chain for tunneling over udp"
echo

print_subheader "Expected Behavior:"
echo "• Client 1 sends requests → Server responds → Clients 2 & 3 receive notifications"
echo "• Client 2 sends requests → Server responds → Clients 1 & 3 receive notifications"
echo "• Client 3 sends requests → Server responds → Clients 1 & 2 receive notifications"
echo "• This creates a distributed notification system where all clients are aware of each other's activity"
echo

# =============================================================================
# KEEP SYSTEM RUNNING
# =============================================================================

print_header "SYSTEM RUNNING"

print_status "Distributed notification system is now running!"
print_status "All clients are sending requests and receiving notifications from the server."
print_status "Press Ctrl+C to stop all services and cleanup..."

# Keep the script running
wait 