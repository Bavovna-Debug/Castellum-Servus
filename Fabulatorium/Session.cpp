// System definition files.
//
#include <algorithm>
#include <cstdbool>
#include <cstdlib>
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
#include "Servus/Dispatcher/Queue.hpp"
#include "Servus/Fabulatorium/Listener.hpp"
#include "Servus/Fabulatorium/Session.hpp"

Fabulatorium::Session::Session(
    TCP::Service&       service,
    const unsigned int  waitForFirstTransmission,
    const unsigned int  waitForTransmissionCompletion) :
Inherited(service),
waitForFirstTransmission(waitForFirstTransmission),
waitForTransmissionCompletion(waitForTransmissionCompletion)
{
    this->receiveBuffer = (char*) malloc(Fabulatorium::MaximalFabulaLength);
    if (this->receiveBuffer == NULL)
    {
        ReportSoftAlert("[Fabulatorium] Out of memory");

        throw std::runtime_error("[Fabulatorium] Out of memory");
    }
}

Fabulatorium::Session::~Session()
{
    free(this->receiveBuffer);
}

void
Fabulatorium::Session::ThreadHandler(Fabulatorium::Session* session)
{
    Dispatcher::Queue& queue = Dispatcher::Queue::SharedInstance();

    bool sessionOK = true;

    // Session should begin with CSeq 1 incrementing for each new datagram.
    // Any other value means that either servus had a problem or be interpreted as intrusion attack.
    //
    unsigned int expectedCSeq = 1;

    RTSP::Datagram request;
    RTSP::Datagram response;

    // Wait for the beginning of transmission (it should not explicitly begin immediately).
    // Cancel session in case of timeout.
    //
    try
    {
        Communicator::Poll(
                session->socket(),
                session->waitForFirstTransmission);
    }
    catch (Communicator::PollError& exception)
    {
        ReportNotice("[Fabulatorium] Poll for transmission did break");

        goto out;
    }
    catch (Communicator::PollTimeout& exception)
    {
        ReportNotice("[Fabulatorium] Poll for transmission timed out");

        goto out;
    }

    // Manage an endless loop of:
    //   - Receive datagram - if necessary receive several chunks until datagram is complete.
    //   - Process datagram.
    //   - Based on received fabula generate aviso.
    //   - Enqueue aviso.
    //   - Generate response.
    //   - Send response.
    //
    do
    {
        request.reset();

        // Receive datagram chunks continuously until either complete datagram
        // is received or timeout ocures.
        //
        for (;;)
        {
            unsigned int receivedBytes;

            try
            {
                receivedBytes = Communicator::Receive(
                        session->socket(),
                        session->receiveBuffer,
                        Fabulatorium::MaximalFabulaLength);
            }
            catch (Communicator::TransmissionError& exception)
            {
                ReportWarning("[Fabulatorium] Connection is broken: errno=%d",
                        exception.errorNumber);

                goto out;
            }
            catch (Communicator::NothingReceived& exception)
            {
                ReportDebug("[Fabulatorium] Disconnected");

                goto out;
            }

            // Statistics.
            //
            {
                //listener->statistics.receivedBytes += receivedBytes;
            }

            try
            {
                request.push(session->receiveBuffer, receivedBytes);
            }
            catch (std::exception& exception)
            {
                ReportWarning("[Fabulatorium] Rejected fabula: %s", exception.what());

                break;
            }

            if (request.datagramComplete() == true)
            {
                break;
            }

            // Wait until next chunk of datagram is available.
            // Cancel session in case of timeout.
            //
            try
            {
                Communicator::Poll(
                        session->socket(),
                        session->waitForTransmissionCompletion);
            }
            catch (Communicator::PollError& exception)
            {
                ReportWarning("[Fabulatorium] Poll for chunk did break");

                goto out;
            }
            catch (Communicator::PollTimeout& exception)
            {
                ReportWarning("[Fabulatorium] Poll for chunk timed out");

                goto out;
            }
        }

        // Statistics.
        //
        {
            //listener->statistics.receivedFabulas++;
        }

        try
        {
            try
            {
                const unsigned int providedCSeq = request["CSeq"];

                if (providedCSeq != expectedCSeq)
                {
                    response.reset();
                    response["CSeq"] = expectedCSeq;
                    response["Agent"] = Servus::SoftwareVersion;
                    response["Reason"] = "Unexpected CSeq";
                    response.generateResponse(RTSP::BadRequest);

                    throw Fabulatorium::RejectDatagram("Unexpected CSeq");
                }
            }
            catch (RTSP::StatementNotFound& exception)
            {
                response.reset();
                response["CSeq"] = expectedCSeq;
                response["Agent"] = Servus::SoftwareVersion;
                response["Reason"] = "Missing CSeq";
                response.generateResponse(RTSP::BadRequest);

                throw Fabulatorium::RejectDatagram("Missing CSeq");
            }

            if (request.methodIs("FABULA") == true)
            {
                try
                {
                    const std::string     timestamp         = request["Timestamp"];
                    const std::string     fabulatorName     = request["Originator"];
                    const unsigned short  severityLevel     = request["Severity"];
                    const bool            notificationFlag  = request["Notification"];

                    if (fabulatorName.length() == 0)
                    {
                        response.reset();
                        response["CSeq"] = expectedCSeq;
                        response["Agent"] = Servus::SoftwareVersion;
                        response["Reason"] = "Missing fabulator";
                        response.generateResponse(RTSP::BadRequest);

                        throw Fabulatorium::RejectDatagram("Missing fabulator");
                    }

                    if (request.payloadLength() == 0)
                    {
                        response.reset();
                        response["CSeq"] = expectedCSeq;
                        response["Agent"] = Servus::SoftwareVersion;
                        response["Reason"] = "Missing payload";
                        response.generateResponse(RTSP::BadRequest);

                        throw Fabulatorium::RejectDatagram("Missing payload");
                    }

                    ReportDebug("[Fabulatorium] Received fabula from \"%s\"",
                            fabulatorName.c_str());

                    Dispatcher::FabulaAviso* aviso = new Dispatcher::FabulaAviso(
                            timestamp,
                            fabulatorName,
                            severityLevel,
                            notificationFlag,
                            request.payload());

                    queue.enqueueAviso(aviso);

                    response.reset();
                    response["CSeq"] = expectedCSeq;
                    response["Agent"] = Servus::SoftwareVersion;
                    response.generateResponse(RTSP::Created);
                }
                catch (std::exception& exception)
                {
                    response.reset();
                    response["CSeq"] = expectedCSeq;
                    response["Agent"] = Servus::SoftwareVersion;
                    response["Reason"] = "Error by parsing";
                    response.generateResponse(RTSP::BadRequest);

                    throw Fabulatorium::RejectDatagram("Error by parsing");
                }
            }
            else
            {
                response.reset();
                response["CSeq"] = expectedCSeq;
                response["Agent"] = Servus::SoftwareVersion;
                response.generateResponse(RTSP::MethodNotAllowed);

                throw Fabulatorium::RejectDatagram("Unknown method");
            }
        }
        catch (Fabulatorium::RejectDatagram& exception)
        {
            ReportWarning("[Fabulatorium] Rejected: %s", exception.what());

            sessionOK = false;
        }

        try
        {
            Communicator::Send(
                    session->socket(),
                    response.contentBuffer,
                    response.contentLength);
        }
        catch (...)
        { }

        if (sessionOK == false)
            break;

        // Wait until next datagram is available.
        //
        try
        {
            Communicator::Poll(session->socket());
        }
        catch (Communicator::PollError&)
        {
            ReportNotice("[Fabulatorium] Keep-alive polling did break");

            goto out;
        }
        catch (Communicator::PollTimeout&)
        {
            ReportNotice("[Fabulatorium] Session timed out");

            goto out;
        }

        // CSeq for each new datagram should be incremented by one.
        //
        expectedCSeq++;
    }
    while (sessionOK == true);

out:
    delete session;
}
