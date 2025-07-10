#pragma once

#include <common/Exported.h>

#include <libacetools/ISignalRegistration.h>
#include <libacetools/ISocketIO.h>

#include <ace/Reactor.h>

#include <memory>

EXPORTED ISocketIO& get_socket_io();

std::unique_ptr<ISignalRegistration> register_for_sigint(ACE_Reactor* reactor);
