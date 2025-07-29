# Refactoring TODO List

## Overview
This document contains identified refactoring opportunities to improve code quality, maintainability, and adherence to the Single Responsibility Principle.

## High Priority Refactoring

*All high priority items have been completed.*

## Medium Priority Refactoring

### 8. Create Utility Classes for Common Patterns
**Issue**: Repetitive patterns throughout codebase
**Recommendation**: Create:
- `Constants` or `Configuration` class for magic numbers and timeouts
- `ErrorHandler` class for unified error handling
- Specialized logger wrappers for different contexts
- `TestBase` class with common setup methods



## Low Priority Refactoring

### 9. Extract Test Infrastructure Classes
**Files**: 
- `src/tests/proxy/NotificationTest/NotificationTest.h`
- `src/tests/proxy/LargeMessageTest/LargeMessageTest.h`

**Issue**: Test classes with too many responsibilities - setup, execution, verification
**Recommendation**: Extract into:
- `TestInfrastructureManager` - for server/proxy setup
- `ClientSessionManager` - for client connection management
- `MessageVerifier` - for message validation
- `NotificationValidator` - for notification verification

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