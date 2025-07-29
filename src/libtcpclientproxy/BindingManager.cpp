#include "BindingManager.h"
#include "BindingFactory.h"

BindingManager::BindingManager(logger::ILogger& logger) : logger_(logger) {}

void BindingManager::setBindingFactory(std::unique_ptr<BindingFactory> factory) {
    factory_ = std::move(factory);
}

BindingManager::BindingPtr& BindingManager::getOrCreateBinding(const std::string& source_key,
    udp::IUdpSession& udp_session,
    const Endpoint& udp_sender) {
    auto it = bindings_.find(source_key);
    if (it != bindings_.end()) {
        return it->second;
    }

    if (!factory_) {
        throw std::runtime_error("Binding factory not available");
    }

    auto binding = factory_->createBinding(source_key, udp_session, udp_sender);
    auto [new_it, inserted] = bindings_.emplace(source_key, std::move(binding));
    if (!inserted) {
        throw std::runtime_error("Failed to store binding");
    }

    return new_it->second;
}

void BindingManager::removeBinding(const std::string& source_key) {
    bindings_.erase(source_key);
}

bool BindingManager::hasBinding(const std::string& source_key) const {
    return bindings_.find(source_key) != bindings_.end();
}