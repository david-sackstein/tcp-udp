#include "SocketIO.h"

#include <libacetools/Converters.h>

#include <ace/Time_Value.h>

static std::chrono::milliseconds block = std::chrono::milliseconds::max();

IOResult SocketIO::read(ACE_SOCK_Stream& socket, Buffer buffer, std::chrono::milliseconds timeout) {
    return from_size(socket_read_to_size(socket, buffer, timeout));
}

IOResult SocketIO::write(ACE_SOCK_Stream& socket, ConstBuffer buffer, std::chrono::milliseconds timeout) {
    return from_size(socket_write_to_size(socket, buffer, timeout));
}

IOResult SocketIO::read_from(ACE_SOCK_Dgram& socket,
    Buffer buffer,
    ACE_INET_Addr& sender,
    std::chrono::milliseconds timeout) {
    return from_size(socket_read_from_to_size(socket, buffer, sender, timeout));
}

IOResult SocketIO::write(ACE_SOCK_Dgram& socket,
    ConstBuffer buffer,
    const ACE_INET_Addr& dest,
    std::chrono::milliseconds timeout) {
    return from_size(socket_write_to_size(socket, buffer, dest, timeout));
}

void SocketIO::set_linger_timeout(ACE_SOCK_Stream& socket) {
    linger linger_val{1, 0};
    socket.set_option(SOL_SOCKET, SO_LINGER, &linger_val, sizeof(linger_val));
}

// private

ssize_t SocketIO::socket_read_to_size(ACE_SOCK_Stream& socket, Buffer buffer, std::chrono::milliseconds timeout) {
    if (timeout == block) {
        return socket.recv(buffer.data, static_cast<int>(buffer.size));
    }
    ACE_Time_Value ace_timeout = to_ace_timeout(timeout);
    return socket.recv(buffer.data, static_cast<int>(buffer.size), &ace_timeout);
}

ssize_t SocketIO::socket_write_to_size(ACE_SOCK_Stream& socket, ConstBuffer buffer, std::chrono::milliseconds timeout) {
    if (timeout == block) {
        return socket.send(buffer.data, static_cast<int>(buffer.size));
    }
    ACE_Time_Value ace_timeout = to_ace_timeout(timeout);
    return socket.send(buffer.data, static_cast<int>(buffer.size), &ace_timeout);
}

ssize_t SocketIO::socket_read_from_to_size(ACE_SOCK_Dgram& socket,
    Buffer buffer,
    ACE_INET_Addr& sender,
    std::chrono::milliseconds timeout) {
    if (timeout == block) {
        return socket.recv(buffer.data, static_cast<int>(buffer.size), sender);
    }
    ACE_Time_Value ace_timeout = to_ace_timeout(timeout);
    return socket.recv(buffer.data, static_cast<int>(buffer.size), sender, 0, &ace_timeout);
}

ssize_t SocketIO::socket_write_to_size(ACE_SOCK_Dgram& socket,
    ConstBuffer buffer,
    const ACE_INET_Addr& dest,
    std::chrono::milliseconds timeout) {
    if (timeout == block) {
        return socket.send(buffer.data, static_cast<int>(buffer.size), dest);
    }
    ACE_Time_Value ace_timeout = to_ace_timeout(timeout);
    return socket.send(buffer.data, static_cast<int>(buffer.size), dest, 0, &ace_timeout);
}

IOResult SocketIO::from_size(ssize_t n) {
    if (n > 0) {
        return IOResult::result_success(static_cast<size_t>(n));
    }
    if (n == 0) {
        return IOResult::result_connection_closed();
    }
    // n == -1
    const int error = ACE_OS::last_error();
    if (error == EAGAIN || error == EWOULDBLOCK || error == ETIME) {
        return IOResult::result_timeout();
    }
    if (error == ECONNRESET || error == ENOTCONN || error == EBADF) {
        return IOResult::result_connection_closed();
    }
    return IOResult::result_error(create_error_message(error));
}

std::string SocketIO::create_error_message(int ace_last_error) {
    std::string error_msg = "Socket read ace_last_error: ";
    switch (ace_last_error) {
        case ECONNRESET:
            error_msg += "Connection reset by peer";
            break;
        case ENOTCONN:
            error_msg += "Socket not connected";
            break;
        case EBADF:
            error_msg += "Bad file descriptor";
            break;
        default:
            error_msg += "Unknown ace_last_error (" + std::to_string(ace_last_error) + ")";
            break;
    }
    return error_msg;
}
