#pragma once

#include "BindingFactory.h"
#include "UdpTcpBinding.h"

#include <liblogger/ILogger.h>
#include <libudp/server/IUdpSession.h>

#include <memory>
#include <unordered_map>

class BindingManager final {
public:
    explicit BindingManager(logger::ILogger& logger);

    using BindingPtr = std::unique_ptr<UdpTcpBinding>;
    using BindingMap = std::unordered_map<std::string, BindingPtr>;

    void setBindingFactory(std::unique_ptr<BindingFactory> factory);
    BindingPtr& getOrCreateBinding(const std::string& source_key,
        udp::IUdpSession& udp_session,
        const Endpoint& udp_sender);
    void removeBinding(const std::string& source_key);
    bool hasBinding(const std::string& source_key) const;

private:
    logger::ILogger& logger_;
    BindingMap bindings_;
    std::unique_ptr<BindingFactory> factory_;
};