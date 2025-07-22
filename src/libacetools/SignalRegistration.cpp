#include "SignalRegistration.h"

#include <ace/Reactor.h>
#include <ace/Event_Handler.h>

SignalRegistration::SignalRegistration(ACE_Reactor *reactor)
    : reactor_(reactor) {
    if (reactor_) {
        reactor_->register_handler(SIGINT, this, ACE_Event_Handler::SIGNAL_MASK);
    }
}

SignalRegistration::~SignalRegistration() {
    if (reactor_) {
        reactor_->remove_handler(SIGINT, ACE_Event_Handler::SIGNAL_MASK);
    }
}

int SignalRegistration::handle_signal(int signum, siginfo_t *, ucontext_t *) {
    if (signum == SIGINT && reactor_) {
        reactor_->end_reactor_event_loop();
    }
    return 0;
}
