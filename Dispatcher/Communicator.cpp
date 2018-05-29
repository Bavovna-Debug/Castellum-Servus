// System definition files.
//
#include <unistd.h>
#include <chrono>
#include <cstdbool>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

// Common definition files.
//
#include "Communicator/IP.hpp"
#include "Communicator/TCP.hpp"
#include "RTSP/RTSP.hpp"
#include "Toolkit/Report.h"

// Local definition files.
//
#include "Servus/Configuration.hpp"
#include "Servus/Dispatcher/Aviso.hpp"
#include "Servus/Dispatcher/Communicator.hpp"
#include "Servus/Dispatcher/Queue.hpp"
#include "Servus/Dispatcher/Setup.hpp"

static Dispatcher::Communicator* instance = NULL;

Dispatcher::Communicator&
Dispatcher::Communicator::InitInstance()
{
    if (instance != NULL)
        throw std::runtime_error("Communicator already initialized");

    instance = new Dispatcher::Communicator();

    return *instance;
}

Dispatcher::Communicator&
Dispatcher::Communicator::SharedInstance()
{
    if (instance == NULL)
        throw std::runtime_error("Communicator not initialized");

    return *instance;
}

Dispatcher::Communicator::Communicator()
{
    this->setupDone = false;

    this->primus.sleepIfRejectedByPrimus =
            Servus::DefaultPrimusSleepIfRejectedByPrimus;

    this->primus.reconnectInterval =
            Servus::DefaultPrimusReconnectInterval;

    this->primus.waitForResponse =
            Servus::DefaultPrimusWaitForResponse;

    this->primus.waitForDatagramCompletion =
            Servus::DefaultPrimusWaitForDatagramCompletion;

    // Allocate resources to be used for receive buffer.
    //
    this->receiveBuffer = (char*) malloc(Dispatcher::MaximalMessageLength);
    if (this->receiveBuffer == NULL)
    {
        ReportSoftAlert("[Dispatcher] Out of memory");

        throw std::runtime_error("[Dispatcher] Out of memory");
    }
}

Dispatcher::Communicator::~Communicator()
{
    free(this->receiveBuffer);
}

void
Dispatcher::Communicator::setConnectionParameters(
    const std::string&      address,
    const unsigned short    portNumber,
    const std::string&      authenticator)
{
    this->primus.address        = address;
    this->primus.portNumber     = portNumber;
    this->primus.authenticator  = authenticator;
}

void
Dispatcher::Communicator::setConnectionIntervals(
    const unsigned int sleepIfRejectedByPrimus,
    const unsigned int reconnectInterval,
    const unsigned int waitForResponse,
    const unsigned int waitForDatagramCompletion)
{
    this->primus.sleepIfRejectedByPrimus    = sleepIfRejectedByPrimus;
    this->primus.reconnectInterval          = reconnectInterval;
    this->primus.waitForResponse            = waitForResponse;
    this->primus.waitForDatagramCompletion  = waitForDatagramCompletion;
}

void
Dispatcher::Communicator::start()
{
    this->thread = std::thread(&Dispatcher::Communicator::ThreadHandler, this);
}

/**
 * @brief   Thread handler for service.
 */
void
Dispatcher::Communicator::ThreadHandler(Dispatcher::Communicator* communicator)
{
    ReportDebug("[Dispatcher] Communicator thread has been started");

    TCP::Connection connection(
            IP::IPv4,
            communicator->primus.address,
            communicator->primus.portNumber);

    for (;;)
    {
        ReportDebug("[Dispatcher] Connecting to primus on %s:%u",
                communicator->primus.address.c_str(),
                communicator->primus.portNumber);

        try
        {
            connection.connect();
        }
        catch (::Communicator::SocketError& exception)
        {
            ReportNotice("[Dispatcher] Cannot connect to primus, retry in %u seconds: %s",
                    communicator->primus.reconnectInterval,
                    exception.what());

            std::this_thread::sleep_for(
                    std::chrono::seconds { communicator->primus.reconnectInterval } );

            continue;
        }

        try
        {
            Dispatcher::Communicator::HandleSession(communicator, connection);

            connection.disconnect();
        }
        catch (Dispatcher::RejectedByPrimus& exception)
        {
            connection.disconnect();

            ReportError("[Dispatcher] %s, wait %u seconds before retry",
                    exception.what(),
                    communicator->primus.sleepIfRejectedByPrimus);

            std::this_thread::sleep_for(
                    std::chrono::seconds { communicator->primus.sleepIfRejectedByPrimus } );

            continue;
        }
        catch (Dispatcher::Exception& exception)
        {
            connection.disconnect();

            if (exception.errorNumber != 0)
            {
                ReportError("[Dispatcher] " \
                        "Connection to Primus lost (errno: %d), reconnect in %u seconds",
                        exception.errorNumber,
                        communicator->primus.reconnectInterval);
            }
            else
            {
                ReportError("[Dispatcher] " \
                        "Connection to Primus lost, reconnect in %u seconds",
                        communicator->primus.reconnectInterval);
            }

            std::this_thread::sleep_for(
                    std::chrono::seconds { communicator->primus.reconnectInterval } );
        }
    }

    ReportWarning("[Dispatcher] Communicator thread is going to quit");
}

void
Dispatcher::Communicator::HandleSession(
    Dispatcher::Communicator*   communicator,
    TCP::Connection&            connection)
{
    Dispatcher::Queue& queue = Dispatcher::Queue::SharedInstance();

    // Session should begin with CSeq 1 incrementing for each new datagram.
    //
    unsigned int expectedCSeq = 1;

    RTSP::Datagram request;
    RTSP::Datagram response;

    {
        request.reset();

        request["CSeq"] = expectedCSeq;
        request["Authenticator"] = communicator->primus.authenticator;
        request["Agent"] = Servus::SoftwareVersion;
        request.generateRequest("AUTH", "rtsp://primus");

        try
        {
            ::Communicator::Send(
                    connection.socket(),
                    request.payloadBuffer,
                    request.payloadLength);
        }
        catch (std::exception& exception)
        {
            throw exception;
        }

        expectedCSeq++;
    }

    {
        response.reset();

        try
        {
            ::Communicator::Poll(
                    connection.socket(),
                    communicator->primus.waitForResponse);
        }
        catch (::Communicator::PollError& exception)
        {
            throw Dispatcher::Exception("Keep-alive polling did break",
                    exception.errorNumber);
        }
        catch (::Communicator::PollTimeout&)
        {
            throw Dispatcher::Exception("Session timed out");
        }

        // Receive datagram chunks continuously until either complete datagram is received
        // or timeout ocures.
        //
        for (;;)
        {
            unsigned int receivedBytes;

            try
            {
                receivedBytes = ::Communicator::Receive(
                        connection.socket(),
                        communicator->receiveBuffer,
                        Dispatcher::MaximalMessageLength);
            }
            catch (::Communicator::TransmissionError& exception)
            {
                throw Dispatcher::Exception("Connection is broken",
                        exception.errorNumber);
            }
            catch (::Communicator::NothingReceived&)
            {
                throw Dispatcher::Exception("Nothing received");
            }

            try
            {
                response.push(communicator->receiveBuffer, receivedBytes);
            }
            catch (std::exception& exception)
            {
                ReportWarning("[Dispatcher] Exception: %s",
                        exception.what());

                break;
            }

            if (response.headerComplete == true)
                break;

            // Wait until next chunk of datagram is available.
            // Cancel session in case of timeout.
            //
            try
            {
                ::Communicator::Poll(
                        connection.socket(),
                        communicator->primus.waitForDatagramCompletion);
            }
            catch (::Communicator::PollError& exception)
            {
                throw Dispatcher::Exception("Poll for chunk did break",
                        exception.errorNumber);
            }
            catch (::Communicator::PollTimeout&)
            {
                throw Dispatcher::Exception("Poll for chunk timed out");
            }
        }

        if (response.statusCode == RTSP::Unauthorized)
        {
            throw Dispatcher::RejectedByPrimus("Rejected by Primus due to wrong authenticator");
        }

        if (response.statusCode == RTSP::Forbidden)
        {
            throw Dispatcher::RejectedByPrimus("Rejected by Primus (Servus disabled)");
        }

        if (response.statusCode != RTSP::OK)
        {
            throw Dispatcher::Exception("Received unexpected response for authentication");
        }
    }

    if (communicator->setupDone == false)
    {
        {
            request.reset();

            request["CSeq"] = expectedCSeq;
            request["Agent"] = Servus::SoftwareVersion;
            request.generateRequest("SETUP", "rtsp://primus");

            try
            {
                ::Communicator::Send(
                        connection.socket(),
                        request.payloadBuffer,
                        request.payloadLength);
            }
            catch (std::exception& exception)
            {
                throw exception;
            }

            expectedCSeq++;
        }

        {
            response.reset();

            try
            {
                ::Communicator::Poll(
                        connection.socket(),
                        communicator->primus.waitForResponse);
            }
            catch (::Communicator::PollError& exception)
            {
                throw Dispatcher::Exception("Keep-alive polling did break",
                        exception.errorNumber);
            }
            catch (::Communicator::PollTimeout&)
            {
                throw Dispatcher::Exception("Session timed out");
            }

            // Receive datagram chunks continuously until either complete datagram is received
            // or timeout ocures.
            //
            for (;;)
            {
                unsigned int receivedBytes;

                try
                {
                    receivedBytes = ::Communicator::Receive(
                            connection.socket(),
                            communicator->receiveBuffer,
                            Dispatcher::MaximalMessageLength);
                }
                catch (::Communicator::TransmissionError& exception)
                {
                    throw Dispatcher::Exception("Connection is broken",
                            exception.errorNumber);
                }
                catch (::Communicator::NothingReceived&)
                {
                    throw Dispatcher::Exception("Nothing received");
                }

                try
                {
                    response.push(communicator->receiveBuffer, receivedBytes);
                }
                catch (std::exception& exception)
                {
                    ReportWarning("[Dispatcher] Exception: %s",
                            exception.what());

                    break;
                }

                if (response.headerComplete == true)
                    break;

                // Wait until next chunk of datagram is available.
                // Cancel session in case of timeout.
                //
                try
                {
                    ::Communicator::Poll(
                            connection.socket(),
                            communicator->primus.waitForDatagramCompletion);
                }
                catch (::Communicator::PollError& exception)
                {
                    throw Dispatcher::Exception("Poll for chunk did break",
                            exception.errorNumber);
                }
                catch (::Communicator::PollTimeout&)
                {
                    throw Dispatcher::Exception("Poll for chunk timed out");
                }
            }

            if (response.statusCode != RTSP::OK)
            {
                throw Dispatcher::Exception("Received unexpected response for configuration");
            }

            Dispatcher::ProcessConfigurationJSON(response.content);
        }

        communicator->setupDone = true;
    }

    {
        request.reset();

        request["CSeq"] = expectedCSeq;
        request["Agent"] = Servus::SoftwareVersion;
        request.generateRequest("PLAY", "rtsp://primus");

        try
        {
            ::Communicator::Send(
                    connection.socket(),
                    request.payloadBuffer,
                    request.payloadLength);
        }
        catch (std::exception& exception)
        {
            throw exception;
        }

        expectedCSeq++;
    }

    // Manage an endless loop of:
    //   - Authenticate.
    //   - Send Avisos once there are some in a queue.
    //   - Send Neutrinos if there are no Avisos for a predifined period of time.
    //
    for (;;)
    {
        response.reset();

        // Receive datagram chunks continuously until either complete datagram is received
        // or timeout ocures.
        //
        for (;;)
        {
            unsigned int receivedBytes;

            try
            {
                receivedBytes = ::Communicator::Receive(
                        connection.socket(),
                        communicator->receiveBuffer,
                        Dispatcher::MaximalMessageLength);
            }
            catch (::Communicator::TransmissionError& exception)
            {
                throw Dispatcher::Exception("Connection is broken",
                        exception.errorNumber);
            }
            catch (::Communicator::NothingReceived&)
            {
                throw Dispatcher::Exception("Nothing received");
            }

            try
            {
                response.push(communicator->receiveBuffer, receivedBytes);
            }
            catch (std::exception& exception)
            {
                ReportWarning("[Dispatcher] Exception: %s",
                        exception.what());

                break;
            }

            if (response.headerComplete == true)
                break;

            // Wait until next chunk of datagram is available.
            // Cancel session in case of timeout.
            //
            try
            {
                ::Communicator::Poll(
                        connection.socket(),
                        communicator->primus.waitForDatagramCompletion);
            }
            catch (::Communicator::PollError& exception)
            {
                throw Dispatcher::Exception("Poll for chunk did break",
                        exception.errorNumber);
            }
            catch (::Communicator::PollTimeout&)
            {
                throw Dispatcher::Exception("Poll for chunk timed out");
            }
        }

        if (response.statusCode == RTSP::Created)
        {
            try
            {
                unsigned int avisoId = response["Aviso-Id"];

                queue.dequeueAviso(avisoId);
            }
            catch (RTSP::StatementNotFound&)
            {
                throw Dispatcher::Exception("Missing aviso id in response from Primus");
            }
        }

        unsigned int neutrinoInterval;

        try
        {
            neutrinoInterval = response["Neutrino-Interval"];
        }
        catch (RTSP::StatementNotFound&)
        {
            throw Dispatcher::Exception("Broken communication with Primus");
        }

        {
            std::cv_status waitStatus;

            ReportDebug("[Dispatcher] Neutrino interval %u milliseconds",
                    neutrinoInterval);

            if (queue.pendingAvisos() == false)
            {
                waitStatus = queue.wait(std::chrono::milliseconds { neutrinoInterval } );

                if (waitStatus == std::cv_status::timeout)
                {
                    ReportDebug("[Dispatcher] Neutrino timed out");

                    try
                    {
                        request.reset();
                        request["CSeq"] = expectedCSeq;
                        request["Agent"] = Servus::SoftwareVersion;
                        request.generateRequest("NEUTRINO", "rtsp://primus");

                        ::Communicator::Send(
                                connection.socket(),
                                request.payloadBuffer,
                                request.payloadLength);

                        // CSeq for each new datagram should be incremented by one.
                        //
                        expectedCSeq++;
                    }
                    catch (std::exception& exception)
                    {
                        ReportWarning("[Dispatcher] Cannot send neutrino: %s",
                                exception.what());

                        throw exception;
                    }
                }
            }

            {
                try
                {
                    Dispatcher::Aviso* aviso = queue.fetchFirstAviso();

                    request.reset();
                    request["CSeq"] = expectedCSeq;
                    request["Agent"] = Servus::SoftwareVersion;
                    aviso->prepare(request);
                    request.generateRequest(aviso->avisoType, "rtsp://primus");

                    try
                    {
                        ::Communicator::Send(
                                connection.socket(),
                                request.payloadBuffer,
                                request.payloadLength);

                        // CSeq for each new datagram should be incremented by one.
                        //
                        expectedCSeq++;
                    }
                    catch (std::exception& exception)
                    {
                        ReportWarning("[Dispatcher] Cannot transmit aviso: %s",
                                exception.what());
                    }
                }
                catch (Dispatcher::NothingInTheQueue&)
                {
                    // Just skip it.
                }
                catch (std::exception& exception)
                {
                    ReportWarning("[Dispatcher] Cannot handle aviso: %s",
                            exception.what());

                    throw exception;
                }
            }
        }
    }
}
