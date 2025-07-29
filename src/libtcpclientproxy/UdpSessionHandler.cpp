#include "UdpSessionHandler.h"

#include <common/Buffer.h>
#include <common/StringUtils.h>
#include <libacetools/IOResult.h>

#include <stdexcept>

UdpSessionHandler::UdpSessionHandler(logger::ILogger& logger) : logger_(logger) {}

ConstBuffer UdpSessionHandler::readUdpMessage(udp::IUdpSession& client_session, Endpoint& udp_sender) {
    IOResult udp_read_result = client_session.read_from(buffer_.view(), udp_sender, timeout_ms);

    if (udp_read_result.code == IOResultCode::Error) {
        throw std::runtime_error(format_string("UdpSessionHandler: %s failed to read UDP: %s",
            udp_sender.to_string().c_str(), udp_read_result.error_message.c_str()));
    }

    if (udp_read_result.code == IOResultCode::ConnectionClosed) {
        logger_.log(
            logger::LogLevel::INFO, "UdpSessionHandler: %s read UDP ConnectionClosed", udp_sender.to_string().c_str());
        return {};
    }

    if (udp_read_result.code == IOResultCode::Timeout) {
        return {};
    }

    return buffer_.view(udp_read_result.count);
}

void UdpSessionHandler::sendResponseToUdp(udp::IUdpSession& client_session,
    ConstBuffer response,
    const Endpoint& udp_sender) const {
    IOResult udp_write_result = client_session.write_to(response, udp_sender, timeout_ms);

    if (udp_write_result.code == IOResultCode::Error) {
        throw std::runtime_error(format_string("UdpSessionHandler: %s failed to send UDP response: %s",
            udp_sender.to_string().c_str(), udp_write_result.error_message.c_str()));
    }

    if (udp_write_result.code == IOResultCode::ConnectionClosed) {
        logger_.log(
            logger::LogLevel::INFO, "UdpSessionHandler: %s write UDP ConnectionClosed", udp_sender.to_string().c_str());
        return;
    }

    if (udp_write_result.code == IOResultCode::Timeout) {
        return;
    }
}