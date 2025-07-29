#pragma once

#include <libacetools/ISignalRegistration.h>

#include <ace/Event_Handler.h>

class SignalRegistration final : public ISignalRegistration, public ACE_Event_Handler {
public:
    explicit SignalRegistration(ACE_Reactor* reactor);
    ~SignalRegistration() override;

    int handle_signal(int signum, siginfo_t*, ucontext_t*) override;
private:
    // non-owning pointer
    ACE_Reactor* reactor_;
}; 