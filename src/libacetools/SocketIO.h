#pragma once

#include <common/IOResult.h>
#include <libacetools/ISocketIO.h>

#include <ace/SOCK_Stream.h>
#include <ace/SOCK_Dgram.h>
#include <ace/INET_Addr.h>

using msec = std::chrono::milliseconds;

class SocketIO : public ISocketIO {
public:
    // TCP methods
    IOResult read(ACE_SOCK_Stream &socket, Buffer buffer, msec timeout) override;
    IOResult write(ACE_SOCK_Stream &socket, ConstBuffer buffer, msec timeout) override;

    // UDP methods
    IOResult read_from(ACE_SOCK_Dgram &socket, Buffer buffer, ACE_INET_Addr &sender, msec timeout) override;
    IOResult write(ACE_SOCK_Dgram &socket, ConstBuffer buffer, const ACE_INET_Addr &dest, msec timeout) override;

    // Socket configuration
    void set_linger_timeout(ACE_SOCK_Stream& socket) override;

private:

    static ssize_t socket_read_to_size(ACE_SOCK_Stream &, Buffer, msec);
    static ssize_t socket_write_to_size(ACE_SOCK_Stream &, ConstBuffer, msec);
    static ssize_t socket_read_from_to_size(ACE_SOCK_Dgram &, Buffer, ACE_INET_Addr &, msec);
    static ssize_t socket_write_to_size(ACE_SOCK_Dgram &, ConstBuffer, const ACE_INET_Addr &, msec);

    static IOResult from_size(ssize_t n);
    static std::string create_error_message(int ace_last_error);
};
