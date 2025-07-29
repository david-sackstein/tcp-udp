# Notification Proxy Chain Setup

This directory contains scripts to set up and test a proxy chain similar to the Notification test in the test suite.

## Architecture

The proxy chain follows this architecture:

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   TCP Client 1  │───▶│ Server Proxy 1   │───▶│                 │
│   Port: 16000   │    │ Port: 17001      │    │                 │
└─────────────────┘    └──────────────────┘    │                 │
┌─────────────────┐    ┌──────────────────┐    │                 │
│   TCP Client 2  │───▶│ Server Proxy 2   │───▶│  Client Proxy   │───▶│  TCP Echo Server │
│   Port: 16001   │    │ Port: 17004      │    │ Port: 17002     │    │ Port: 17003      │
└─────────────────┘    └──────────────────┘    │                 │
┌─────────────────┐    ┌──────────────────┐    │                 │
│   TCP Client 3  │───▶│ Server Proxy 3   │───▶│                 │
│   Port: 16002   │    │ Port: 17005      │    │                 │
└─────────────────┘    └──────────────────┘    └─────────────────┘
```

## Prerequisites

1. **Build the project**: Run `make all` to build all executables
2. **Install netcat**: Required for testing (`sudo apt install netcat` on Ubuntu/Debian)
3. **Sudo access**: Required for killing processes on ports

## Files

- `setup_notification_proxy_chain.sh` - Main setup script
- `test_proxy_chain.sh` - Test script to verify the proxy chain
- `PROXY_CHAIN_README.md` - This file

## Usage

### 1. Setup the Proxy Chain

```bash
./setup_notification_proxy_chain.sh
```

This script will:
- Kill any existing processes on the required ports
- Start the TCP echo server on port 17003
- Start the client proxy on port 17002
- Start three server proxies on ports 17001, 17004, and 17005
- Wait for all services to be ready
- Display the architecture diagram
- Keep running until you press Ctrl+C

### 2. Test the Proxy Chain

In a separate terminal, run:

```bash
./test_proxy_chain.sh
```

This will test each server proxy by sending a message and verifying the echo response.

### 3. Manual Testing

You can also test manually using netcat:

```bash
# Test Server Proxy 1
echo "hello from client 1" | nc localhost 17001

# Test Server Proxy 2  
echo "hello from client 2" | nc localhost 17004

# Test Server Proxy 3
echo "hello from client 3" | nc localhost 17005
```

## Port Configuration

The scripts use the same port configuration as the NotificationTest:

- **Client ports**: 16000, 16001, 16002 (for TCP clients)
- **Server proxies**: 17001, 17004, 17005 (entry points for clients)
- **Client proxy**: 17002 (forwards to TCP server)
- **TCP echo server**: 17003 (final destination)

## Features

### Setup Script Features

- **Colored output**: Easy to read status messages
- **Port checking**: Verifies ports are available before starting
- **Process cleanup**: Automatically kills existing processes on ports
- **Service readiness**: Waits for each service to be ready before continuing
- **Graceful shutdown**: Cleans up all processes when you press Ctrl+C
- **Error handling**: Exits on any error with helpful messages

### Test Script Features

- **Multiple test endpoints**: Tests all three server proxies
- **Response verification**: Checks that echo responses are received
- **Colored output**: Clear success/error indicators
- **Netcat dependency check**: Ensures netcat is available

## Troubleshooting

### Port Already in Use

If you see "Port X is already in use" errors:

```bash
# Kill all processes on the ports
sudo lsof -i :17001 -i :17002 -i :17003 -i :17004 -i :17005 -sTCP:LISTEN -t | xargs -r sudo kill -9

# Or use the built-in cleanup
./setup_notification_proxy_chain.sh
```

### Executables Not Found

If you see "executable not found" errors:

```bash
# Build all executables
make all
```

### Permission Denied

If you get permission errors:

```bash
# Make scripts executable
chmod +x setup_notification_proxy_chain.sh
chmod +x test_proxy_chain.sh
```

### Netcat Not Found

If netcat is not installed:

```bash
# Ubuntu/Debian
sudo apt install netcat

# CentOS/RHEL
sudo yum install nc

# macOS
brew install netcat
```

## Example Output

### Setup Script Output

```
[SUCCESS] All executables found
[INFO] Setting up Notification Proxy Chain...
[INFO] Architecture: 3 TCP clients -> 3 server proxies -> 1 client proxy -> 1 TCP echo server
[INFO] Starting TCP echo server on port 17003...
[SUCCESS] TCP Echo Server is ready on port 17003
[INFO] Starting TCP client proxy on port 17002...
[SUCCESS] TCP Client Proxy is ready on port 17002
[INFO] Starting TCP server proxy 1 on port 17001...
[SUCCESS] TCP Server Proxy 1 is ready on port 17001
[INFO] Starting TCP server proxy 2 on port 17004...
[SUCCESS] TCP Server Proxy 2 is ready on port 17004
[INFO] Starting TCP server proxy 3 on port 17005...
[SUCCESS] TCP Server Proxy 3 is ready on port 17005
[SUCCESS] All proxy components started successfully!
```

### Test Script Output

```
[INFO] Testing Notification Proxy Chain...

[INFO] Testing Server Proxy 1 on port 17001...
[SUCCESS] Server Proxy 1: Sent 'hello from Server Proxy 1', received 'echo hello from Server Proxy 1'

[INFO] Testing Server Proxy 2 on port 17004...
[SUCCESS] Server Proxy 2: Sent 'hello from Server Proxy 2', received 'echo hello from Server Proxy 2'

[INFO] Testing Server Proxy 3 on port 17005...
[SUCCESS] Server Proxy 3: Sent 'hello from Server Proxy 3', received 'echo hello from Server Proxy 3'

[INFO] Test completed!
[INFO] If all tests passed, your proxy chain is working correctly!
```

## Comparison with Notification Test

This setup mimics the `MultiClientNotificationTest` from the test suite:

- **Same port configuration**: Uses identical ports
- **Same architecture**: 3 clients → 3 server proxies → 1 client proxy → 1 server
- **Same components**: TCP echo server, client proxy, server proxies
- **Same behavior**: Echo responses for all messages

The main difference is that this is a manual setup for testing and demonstration, while the test suite uses automated setup and verification. 