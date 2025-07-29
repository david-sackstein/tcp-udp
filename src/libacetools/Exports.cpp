#include "SignalRegistration.h"
#include "SocketIO.h"

#include <libacetools/Exports.h>

EXPORTED ISocketIO& get_socket_io() {
    static SocketIO instance;
    return instance;
}

EXPORTED std::unique_ptr<ISignalRegistration> register_for_sigint(ACE_Reactor* reactor) {
    return std::make_unique<SignalRegistration>(reactor);
}
