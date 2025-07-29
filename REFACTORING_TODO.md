# Refactoring TODO List

## Overview
This document contains identified refactoring opportunities to improve code quality, maintainability, and adherence to the Single Responsibility Principle.

## High Priority Refactoring

### 1. Extract `ArgsParser` Class
**File**: `src/libargsparser/ArgsParser.h` and `ArgsParser.cpp`
**Issue**: Class has too many responsibilities - argument parsing, user interaction, validation, and configuration
**Recommendation**: Extract into smaller, focused classes:
- `CommandLineParser` - for command line argument handling
- `InteractiveConfigurator` - for interactive user input  
- `EndpointValidator` - for endpoint validation logic
- `UserInputHandler` - for user input processing

### 2. Refactor `TcpEchoHandler::handle_client()` Lambda
**File**: `src/libtcp/examples/TcpEchoHandler.cpp` (lines 12-59)
**Issue**: Lambda function is doing too much - reading, processing, writing, and error handling
**Recommendation**: Extract into separate methods:
- `processEchoMessage()`
- `handleReadResult()`
- `handleWriteResult()`

### 3. Extract `TcpServerProxyClientHandler` Class
**File**: `src/libtcpserverproxy/TcpServerProxyClientHandler.h`
**Issue**: Handles both TCP session management and UDP forwarding logic
**Recommendation**: Extract into:
- `TcpSessionManager` - for TCP session lifecycle
- `UdpForwarder` - for UDP forwarding logic
- `ClientKeyManager` - for client key management

### 4. Extract `TcpClientProxyUdpHandler` Class
**File**: `src/libtcpclientproxy/TcpClientProxyUdpHandler.h`
**Issue**: Manages both UDP session handling and TCP binding creation
**Recommendation**: Extract into:
- `UdpSessionHandler` - for UDP session processing
- `TcpBindingManager` - for TCP binding lifecycle
- `BindingFactory` - for creating new bindings

## Medium Priority Refactoring

### 5. Extract Test Infrastructure Classes
**Files**: 
- `src/tests/proxy/NotificationTest/NotificationTest.h`
- `src/tests/proxy/LargeMessageTest/LargeMessageTest.h`

**Issue**: Test classes with too many responsibilities - setup, execution, verification
**Recommendation**: Extract into:
- `TestInfrastructureManager` - for server/proxy setup
- `ClientSessionManager` - for client connection management
- `MessageVerifier` - for message validation
- `NotificationValidator` - for notification verification

### 6. Refactor Long Test Methods
**File**: `src/tests/proxy/LargeMessageTest/LargeMessageTest.cpp`
**Method**: `runLargeMessageTest()` (lines 175-255)
**Issue**: Very long function handling multiple responsibilities
**Recommendation**: Extract into:
- `setupTestInfrastructure()`
- `sendLargeMessage()`
- `receiveAndVerifyMessage()`
- `handleMessageFragmentation()`

**File**: `src/tests/proxy/NotificationTest/NotificationTest.cpp`
**Method**: `readAllMessages()` (lines 280-310)
**Issue**: Complex message reading logic with multiple loops
**Recommendation**: Extract into:
- `readMessagesFromSession()`
- `processReceivedMessages()`
- `combineMessages()`

### 7. Refactor `ArgsParser` Helper Methods
**File**: `src/libargsparser/ArgsParser.cpp`
**Methods**:
- `handle_interactive_config()` (lines 44-65)
- `get_validated_user_input()` (lines 95-113)

**Issue**: Contains complex input validation logic with multiple responsibilities
**Recommendation**: Extract into:
- `validateNumericInput()`
- `displayInputError()`
- `getValidatedInteger()`
- `displayConfigurationMenu()`
- `validateUserConfiguration()`
- `buildFinalConfiguration()`

## Low Priority Refactoring

### 8. Create Utility Classes for Common Patterns
**Issue**: Repetitive patterns throughout codebase
**Recommendation**: Create:
- `Constants` or `Configuration` class for magic numbers and timeouts
- `ErrorHandler` class for unified error handling
- Specialized logger wrappers for different contexts
- `TestBase` class with common setup methods

### 9. Additional Method Extractions
**File**: `src/tests/proxy/NotificationTest/NotificationTest.cpp`
**Method**: `runCrossClientNotificationTest()` (lines 200-220)
**Issue**: Complex test setup and execution logic
**Recommendation**: Extract into:
- `setupNotificationTest()`
- `executeNotificationRound()`
- `verifyCrossClientNotifications()`

## Implementation Guidelines

### C++ Coding Standards to Follow
1. Every class that is not inherited today - make final
2. All headers that are redundant - remove
3. All methods that can be static - make static
4. All methods that cannot be made static but can be made const - make const
5. Follow modern C++ best practices
6. Use RAII principles
7. Prefer smart pointers over raw pointers
8. Use const correctness where possible
9. Minimize header dependencies

### Header Management Rules
- When a type is referenced as pointer or reference in a header and is part of the private definition of the class, use forward declaration
- If a type is part of the public API (method parameters, return types, public members), include the header that defines the type
- Only include headers for types that are actually used in the header file (not just declared)

### ACE Dependencies
- All components must hide ACE dependencies
- Public APIs should not expose ACE types or require linking to or including ACE headers
- The ITcpSession interface should remain ACE-agnostic and not expose ACE types like ACE_SOCK_Stream in its methods

## Notes
- The assistant should pause after each implementation step for review before continuing
- Whitespace should remain unchanged when editing code
- All refactoring should maintain existing functionality while improving code structure 