#pragma once

// System definition files.
//
#include <string>
#include <thread>

// Common definition files.
//
#include "Communicator/TCP.hpp"

namespace Fabulatorium
{
    static const unsigned int MaximalFabulaLength = 64 * 1024;

    class Listener : public TCP::Service
    {
        typedef TCP::Service Inherited;

    private:
        /**
         * Thread handler of service thread.
         */
        std::thread         thread;

    private:
        std::string         listenerName;
        std::string         listenerAddress;
        unsigned short      listenerPortNumber;

    public:
        unsigned int        waitForFirstTransmission;
        unsigned int        waitForTransmissionCompletion;

        struct
        {
            unsigned int    receivedFabulas;
            unsigned int    receivedBytes;
        }
        statistics;

    public:
        Listener(
            const std::string&      listenerName,
            const std::string&      listenerAddress,
            const unsigned short    listenerPortNumber);

        ~Listener();

        void
        setConnectionIntervals(
            const unsigned int waitForFirstTransmission,
            const unsigned int waitForTransmissionCompletion);

    private:
        static void
        ThreadHandler(Listener*);
    };
};
