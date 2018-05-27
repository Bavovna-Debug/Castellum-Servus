#pragma once

// System definition files.
//
#include <stdexcept>

// Common definition files.
//
#include "Communicator/TCP.hpp"

namespace Fabulatorium
{
    class Session : public TCP::Connection
    {
        typedef TCP::Connection Inherited;

    private:
        unsigned int        waitForFirstTransmission;
        unsigned int        waitForTransmissionCompletion;

        char*               receiveBuffer;

    public:
        Session(
            TCP::Service&,
            const unsigned int waitForFirstTransmission,
            const unsigned int waitForTransmissionCompletion);

        ~Session();

        static void
        ThreadHandler(Session*);
    };

    class RejectDatagram : public std::runtime_error
    {
    public:
        RejectDatagram(const char* const reason) throw() :
        std::runtime_error(reason)
        { }
    };
};
