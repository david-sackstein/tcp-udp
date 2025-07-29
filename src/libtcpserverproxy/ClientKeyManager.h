#pragma once

#include <common/Endpoint.h>
#include <liblogger/ILogger.h>
#include <libtcp/ITcpSession.h>

#include <string>

class ClientKeyManager final {
public:
    explicit ClientKeyManager(logger::ILogger& logger);

    static std::string generateClientKey(const tcp::ITcpSession& session);
    void logClientConnection(const std::string& client_key) const;

private:
    logger::ILogger& logger_;
};