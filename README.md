# TCP-over-UDP Proxy Chain

## Overview

This project implements a **TCP-over-UDP proxy chain** that enables TCP tunneling through UDP networks. This is particularly useful in scenarios where:

- UDP is the only available protocol (e.g., certain corporate firewalls)
- You need to tunnel TCP applications over UDP-only networks
- You want to create distributed TCP services that communicate over UDP

The system consists of two main proxy components that work together to create a seamless TCP-over-UDP tunnel.

## Core Concept: TCP-over-UDP

The fundamental concept is to encapsulate TCP connections within UDP packets, allowing TCP applications to communicate through UDP-only networks. Here's how it works:

1. **TCP Server Proxy** - Accepts TCP connections from clients and forwards them over UDP
2. **TCP Client Proxy** - Receives UDP packets and forwards them to the actual TCP server
3. **UDP Tunnel** - The middle layer that carries TCP data as UDP payload

This creates a transparent tunnel where TCP applications can communicate as if they were directly connected, even when separated by UDP-only networks.

## Main Products

### 1. TCP Server Proxy (`tcpserverproxy`)

The **TCP Server Proxy** acts as the frontend that accepts TCP connections from clients and forwards them over UDP.

**Usage:**
```bash
./bin/tcpserverproxy [listen_endpoint] [forward_endpoint] [--verbose]
```

**Arguments:**
- `listen_endpoint` - The TCP endpoint to listen on (e.g., `127.0.0.1:16001`)
- `forward_endpoint` - The UDP endpoint to forward to (e.g., `127.0.0.1:17000`)
- `--verbose` - Enable verbose logging (optional)

**Example:**
```bash
./bin/tcpserverproxy 127.0.0.1:16001 127.0.0.1:17000 --verbose
```

### 2. TCP Client Proxy (`tcpclientproxy`)

The **TCP Client Proxy** acts as the backend that receives UDP packets and forwards them to the actual TCP server.

**Usage:**
```bash
./bin/tcpclientproxy [listen_endpoint] [forward_endpoint] [--verbose]
```

**Arguments:**
- `listen_endpoint` - The UDP endpoint to listen on (e.g., `127.0.0.1:17000`)
- `forward_endpoint` - The TCP endpoint to forward to (e.g., `127.0.0.1:17001`)
- `--verbose` - Enable verbose logging (optional)

**Example:**
```bash
./bin/tcpclientproxy 127.0.0.1:17000 127.0.0.1:17001 --verbose
```

## Setup Script: Distributed Notification System

The project includes a comprehensive setup script that demonstrates the proxy chain in action with a distributed notification system.

### What the Setup Script Does

The `scripts/setup/setup_notification_proxy_chain.sh` script creates a complete distributed notification system that showcases the TCP-over-UDP proxy capabilities:

1. **Starts a Notifying Server** - A TCP server that echoes requests and sends notifications to all connected clients
2. **Launches Multiple TCP Server Proxies** - Each accepting TCP connections from clients
3. **Runs a TCP Client Proxy** - Forwards all connections over UDP to the server
4. **Starts Multiple Notifiable Clients** - Each sending requests and receiving notifications through the proxy chain

### Architecture Diagram

```
┌────────────────────────────┐    ┌─────────────────────────┐    ┌──────────────────────┐
│ Notifiable Client 1        │───▶│                         │───▶│                      │
│ ID: client1                │    │ TCP Server Proxy 1      │    │                      │
│ Port: 16000                │    │ Port: 16001             │    │                      │
└────────────────────────────┘    └─────────────────────────┘    │                      │
┌────────────────────────────┐    ┌─────────────────────────┐    │                      │    ┌───────────────────┐
│ Notifiable Client 2        │───▶│                         │───▶│  TCP Client Proxy    │───▶│  Notifying Server │
│ ID: client2                │    │ TCP Server Proxy 2      │    │  Port: 17000         │    │  Port: 17001       │
│ Port: 16002                │    │ Port: 16003             │    │                      │    └───────────────────┘
└────────────────────────────┘    └─────────────────────────┘    │                      │
┌────────────────────────────┐    ┌─────────────────────────┐    │                      │
│ Notifiable Client 3        │───▶│                         │───▶│                      │
│ ID: client3                │    │ TCP Server Proxy 3      │    │                      │
│ Port: 16004                │    │ Port: 16005             │    │                      │
└────────────────────────────┘    └─────────────────────────┘    └──────────────────────┘
```

### Running the Setup Script

**Basic mode (client validation only):**
```bash
./scripts/setup/setup_notification_proxy_chain.sh
```

**Verbose mode (all logs):**
```bash
./scripts/setup/setup_notification_proxy_chain.sh --verbose
```

**High-load testing (100ms intervals):**
```bash
# Edit REQUEST_INTERVAL_MS=100 in the script for high-load testing
./scripts/setup/setup_notification_proxy_chain.sh
```

### System Behavior

- Each client sends requests with its ID every 1000ms (or 100ms for high-load testing)
- The server responds to each request and sends notifications to all other clients
- All communication flows through the proxy chain for tunneling over UDP
- Clients validate echo responses and notification sequences
- The system demonstrates robust message handling and validation

## Building the Project

### Using Make (Command Line)

**Prerequisites:**
- C++17 compatible compiler (GCC 7+ or Clang 5+)
- Make
- ACE (Adaptive Communication Environment) library

**Build Commands:**
```bash
# Build all executables
make all

# Build specific components
make notifying_server
make notifiable_client
make tcpserverproxy
make tcpclientproxy

# Run tests
make tests

# Clean build artifacts
make clean

# Format code (requires clang-format)
make clang-format-fix
```

**Output Location:**
All executables are built to the `bin/` directory:
- `bin/notifying_server` - The notification server
- `bin/notifiable_client` - The self-validating client
- `bin/tcpserverproxy` - TCP Server Proxy
- `bin/tcpclientproxy` - TCP Client Proxy
- `bin/tcp_echo_server` - Simple TCP echo server
- `bin/udp_echo_server` - Simple UDP echo server

### Using CMake (CLion/IDE)

**Prerequisites:**
- CLion or any CMake-compatible IDE
- CMake 3.15+
- ACE library

**Build Steps:**
1. Open the project in CLion
2. CLion will automatically detect the `CMakeLists.txt` and configure the project
3. Build using the IDE's build button or `Ctrl+F9`
4. Executables will be placed in `cmake-build-debug/bin/` or `cmake-build-release/bin/`

**CMake Configuration:**
```bash
# Manual CMake build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Project Structure

### Key Libraries

- **`libacetools`** - ACE utility wrappers and socket abstractions
- **`libtcp`** - TCP server/client abstractions and session management
- **`libudp`** - UDP server/client abstractions
- **`libtcpclientproxy`** - TCP client proxy implementation
- **`libtcpserverproxy`** - TCP server proxy implementation
- **`libargsparser`** - Command-line argument parsing utilities
- **`liblogger`** - Logging system with multiple backends

### Main Executables

- **`tcpserverproxy`** - TCP server proxy (frontend)
- **`tcpclientproxy`** - TCP client proxy (backend)
- **`notifying_server`** - Demo TCP server with notification capabilities
- **`notifiable_client`** - Demo TCP client with validation
- **`tcp_echo_server`** - Simple TCP echo server for testing
- **`tcpclient`** - Simple TCP client for testing

### Test Applications

- **`tests`** - Comprehensive test suite with GoogleTest
- **`tcp_echo_server`** - Echo server for testing
- **`udp_echo_server`** - UDP echo server for testing

### Build System

**Build with Make:**
```bash
make all
```

**Run tests:**
```bash
make tests
```

**Clean build:**
```bash
make clean
```

## Prerequisites

- **OS:** Linux or macOS
- **ACE library:** Must be installed ([The ACE ORB](https://www.dre.vanderbilt.edu/~schmidt/ACE.html))
- **Build tools:** GCC/Clang with C++17 support
- **Dependencies:** Update ACE include/library paths in `Makefile` and `CMakeLists.txt` as needed

## Use Cases

1. **Corporate Networks** - Tunnel TCP applications through UDP-only corporate firewalls
2. **Distributed Systems** - Create TCP services that communicate over UDP networks
3. **Network Testing** - Test application behavior under UDP-only conditions
4. **Protocol Translation** - Bridge TCP applications to UDP-only environments

## Features

- **Robust Message Handling** - Handles concatenated messages and network delays
- **Validation System** - Clients validate echo responses and notification sequences
- **High Performance** - Supports high-load scenarios with 100ms intervals
- **Flexible Logging** - Configurable verbosity levels for debugging
- **Comprehensive Testing** - Full test suite with integration tests
- **Clean Architecture** - Well-separated concerns with library-based design 