// System definition files.
//
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <thread>

// Common definition files.
//
#include "Communicator/IP.hpp"
#include "Communicator/TCP.hpp"
#include "Toolkit/Report.h"

// Local definition files.
//
#include "Servus/Configuration.hpp"
#include "Servus/Fabulatorium/Listener.hpp"
#include "Servus/Fabulatorium/Session.hpp"

Fabulatorium::Listener::Listener(
    const std::string&      listenerName,
    const std::string&      listenerAddress,
    const unsigned short    listenerPortNumber) :
Inherited(IP::IPv4, listenerAddress, listenerPortNumber),
listenerName(listenerName),
listenerAddress(listenerAddress),
listenerPortNumber(listenerPortNumber)
{
    this->waitForFirstTransmission =
            Servus::DefaultListenerWaitForFirstTransmission;
    this->waitForTransmissionCompletion =
            Servus::DefaultListenerWaitForTransmissionCompletion;

    this->statistics.receivedFabulas = 0;
    this->statistics.receivedBytes = 0;

    ReportInfo("[Listener] Defined listener \"%s\" on %s:%u ",
            listenerName.c_str(),
            (listenerAddress.length() == 0) ? "*" : listenerAddress.c_str(),
            listenerPortNumber);

    this->thread = std::thread(&Fabulatorium::Listener::ThreadHandler, this);
}

void
Fabulatorium::Listener::setConnectionIntervals(
    const unsigned int waitForFirstTransmission,
    const unsigned int waitForTransmissionCompletion)
{
    this->waitForFirstTransmission = waitForFirstTransmission;
    this->waitForTransmissionCompletion = waitForTransmissionCompletion;
}

/**
 * @brief   Thread handler for service.
 */
void
Fabulatorium::Listener::ThreadHandler(Fabulatorium::Listener* listener)
{
    ReportDebug("[Listener] Service thread has been started");

    for (;;)
    {
        try
        {
            listener->disconnect();
            listener->connect();

            for (;;)
            {
                Fabulatorium::Session* session = new Fabulatorium::Session
                {
                    *listener,
                    listener->waitForFirstTransmission,
                    listener->waitForTransmissionCompletion
                };

                std::thread connectionThread
                {
                    &Fabulatorium::Session::ThreadHandler,
                    session
                };

                connectionThread.detach();
            }
        }
        catch (Communicator::SocketError& exception)
        {
            ReportError("[Listener] Exception: %s: errno=%d",
                    exception.what(),
                    exception.errorNumber);

            std::this_thread::sleep_for(
                    std::chrono::seconds { Servus::WaitBeforeNetworkRetry } );

            continue;
        }
    }

    // Make sure the socket is closed.
    //
    listener->disconnect();

    ReportWarning("[Listener] Service thread is going to quit");
}
