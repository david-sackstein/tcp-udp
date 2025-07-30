# Notification Proxy Chain Implementation Plan

## Overview

This document outlines the implementation plan for creating a distributed notification system with 3 clients, 3 client proxies, 1 server proxy, and 1 server. The system will allow clients to send requests and receive notifications about requests from other clients.

## Current State Analysis

### Existing Components
- ✅ **NotificationTest**: Comprehensive test demonstrating the desired behavior
- ✅ **Proxy Infrastructure**: `libtcpclientproxy` and `libtcpserverproxy` libraries
- ✅ **TCP Libraries**: `libtcp` with client/server functionality
- ❌ **Client Application**: Missing - needs to be created
- ❌ **Server Application**: Missing - needs to be created
- ❌ **Setup Script**: Incomplete - references non-existent executables

### Architecture Requirements
```
Client 1 ──► Client Proxy 1 ──┐
Client 2 ──► Client Proxy 2 ──┼──► Server Proxy ──► Notifying Server
Client 3 ──► Client Proxy 3 ──┘
```

## Implementation Steps

### Step 1: Move NotificationHandler to Library (Safe Refactoring)
**Goal**: Make NotificationHandler reusable as a library component
**Files to modify**:
- Move `src/tests/proxy/NotificationTest/NotificationHandler.h` → `include/libtcp/examples/NotificationHandler.h`
- Move `src/tests/proxy/NotificationTest/NotificationHandler.cpp` → `src/libtcp/examples/NotificationHandler.cpp`
- Update `include/libtcp/Exports.h` - add `create_tcp_notification_handler()` factory method
- Update `src/libtcp/Exports.cpp` - implement factory method
- Update `src/tests/proxy/NotificationTest/NotificationTest.cpp` - use new factory method
- Update `CMakeLists.txt` and `Makefile` - add new source files

**Validation**: All existing tests should continue to pass

### Step 2: Create Notifiable Client Application
**Goal**: Create client application that sends periodic requests with client ID
**Files to create**:
- `src/notifiable_client/main.cpp` - Main application entry point
- `include/libtcp/examples/NotifiableClientHandler.h` - Client-side handler
- `src/libtcp/examples/NotifiableClientHandler.cpp` - Implementation

**Client Behavior**:
- Connect to server via proxy chain
- Send requests every second with format: `"request from client {id}: {timestamp}"`
- Process responses (echo + notifications from other clients)
- Handle connection errors gracefully
- Support command-line arguments for endpoint configuration

**Technical Considerations**:
- Use non-blocking I/O pattern for sending/receiving without reactor
- Implement periodic sending using `std::thread` or similar
- Handle connection drops and reconnection logic

### Step 3: Create Notifying Server Application
**Goal**: Create server application using notification handler
**Files to create**:
- `src/notifying_server/main.cpp` - Main application entry point

**Server Behavior**:
- Use the `NotificationHandler` from Step 1
- Accept multiple client connections
- Echo responses to requesting client
- Send notifications to all other connected clients
- Support command-line arguments for endpoint configuration

### Step 4: Update Build Configuration
**Goal**: Add new executables to build system
**Files to modify**:
- `CMakeLists.txt` - Add notifiable_client and notifying_server targets
- `Makefile` - Add build rules for new executables

### Step 5: Update Setup Script
**Goal**: Fix and complete the proxy chain setup script
**Files to modify**:
- `scripts/setup/setup_notification_proxy_chain.sh` - Fix executable references and add proper error handling

**Script Improvements**:
- Use correct executable names (`notifiable_client`, `notifying_server`)
- Start 3 client proxies (one for each client)
- Start 1 server proxy (shared by all client proxies)
- Add proper cleanup and error handling
- Add validation for all required executables
- Improve logging and status reporting

## Technical Design Details

### Client Handler Design
```cpp
class NotifiableClientHandler {
private:
    logger::ILogger& logger_;
    std::string client_id_;
    std::chrono::milliseconds request_interval_;
    std::atomic<bool> running_;
    std::thread sender_thread_;
    std::thread receiver_thread_;
    
public:
    NotifiableClientHandler(logger::ILogger& logger, std::string client_id);
    ~NotifiableClientHandler();
    
    void start(std::shared_ptr<tcp::ITcpSession> session);
    void stop();
    
private:
    void sender_loop(std::shared_ptr<tcp::ITcpSession> session);
    void receiver_loop(std::shared_ptr<tcp::ITcpSession> session);
};
```

### Server Application Design
```cpp
// Main application using existing NotificationHandler
auto handler = tcp::create_tcp_notification_handler(*logger);
auto server = tcp::start_tcp_server(*logger, listen_endpoint, *handler);
```

### Command Line Interface
Both applications should support:
- `--endpoint <host:port>` - Specify connection/listen endpoint
- `--client-id <id>` - Client identifier (for client app only)
- `--interval <ms>` - Request interval in milliseconds (for client app only)

## Testing Strategy

### Unit Tests
- Test client handler in isolation
- Test server with mock clients
- Test notification propagation logic

### Integration Tests
- Test full proxy chain with 3 clients
- Test notification delivery to all clients
- Test connection resilience and reconnection

### Manual Testing
- Use setup script to run full system
- Verify notifications are received by all clients
- Test with different client IDs and intervals

## Risk Assessment

### Low Risk
- **Step 1**: Moving existing code to library (refactoring)
- **Step 3**: Creating server app (reuses existing handler)

### Medium Risk
- **Step 2**: Client application (new threading model)
- **Step 4**: Build configuration changes

### High Risk
- **Step 5**: Setup script (complex process management)

## Success Criteria

1. ✅ All existing tests continue to pass
2. ✅ New applications build successfully
3. ✅ Setup script runs without errors
4. ✅ 3 clients can connect and send requests
5. ✅ Notifications are delivered to all clients
6. ✅ System handles connection drops gracefully

## Timeline Estimate

- **Step 1**: 1-2 hours (safe refactoring)
- **Step 2**: 3-4 hours (new client application)
- **Step 3**: 1 hour (server application)
- **Step 4**: 1 hour (build configuration)
- **Step 5**: 2-3 hours (setup script)

**Total**: 8-11 hours

## Next Steps

1. Begin with **Step 1** - Move NotificationHandler to library
2. Validate that all existing tests pass
3. Proceed with **Step 2** - Create client application
4. Continue with remaining steps in order
5. Test full system integration

## Notes

- All code changes will follow the established C++ coding standards
- Header organization will follow the project's include order rules
- All public APIs will remain ACE-agnostic
- Whitespace and formatting will be preserved during edits 