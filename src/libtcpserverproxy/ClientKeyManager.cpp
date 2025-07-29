#include "ClientKeyManager.h"

ClientKeyManager::ClientKeyManager(logger::ILogger& logger) : logger_(logger) {}

std::string ClientKeyManager::generateClientKey(const tcp::ITcpSession& session) {
    Endpoint tcp_client = session.get_peer();
    return tcp_client.to_string();
}

void ClientKeyManager::logClientConnection(const std::string& client_key) const {
    logger_.log(logger::LogLevel::INFO, "ClientKeyManager: New TCP client connected: %s", client_key.c_str());
}