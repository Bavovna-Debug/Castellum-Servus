#pragma once

// System definition files.
//
#include <cstdbool>
#include <stdexcept>
#include <string>
#include <thread>

// Common definition files.
//
#include "Communicator/TCP.hpp"

namespace Dispatcher
{
    static const unsigned int MaximalMessageLength = 2048;

    class Communicator
    {
    private:
        /**
         * Thread handler of dispatcher thread.
         */
        std::thread         thread;

    private:
        struct
        {
            std::string     address;
            unsigned short  portNumber;
            std::string     authenticator;
            unsigned int    sleepIfRejectedByPrimus;
            unsigned int    reconnectInterval;
            unsigned int    waitForResponse;
            unsigned int    waitForDatagramCompletion;
        }
        primus;

        char*               receiveBuffer;

    public:
        static Communicator&
        InitInstance();

        static Communicator&
        SharedInstance();

    private:
        Communicator();

        ~Communicator();

    public:
        void
        setConnectionParameters(
            const std::string&      address,
            const unsigned short    portNumber,
            const std::string&      authenticator);

        void
        setConnectionIntervals(
            const unsigned int sleepIfRejectedByPrimus,
            const unsigned int reconnectInterval,
            const unsigned int waitForResponse,
            const unsigned int waitForDatagramCompletion);

        void
        start();

    private:
        static void
        ThreadHandler(Communicator*);

        static void
        HandleSession(Communicator*, TCP::Connection&);
    };

    class Exception : public std::runtime_error
    {
    public:
        int errorNumber;

    public:
        Exception(const char* const reason) throw() :
        std::runtime_error(reason),
        errorNumber(0)
        { }

        Exception(const char* const reason, int errorNumber) throw() :
        std::runtime_error(reason),
        errorNumber(errorNumber)
        { }
    };

    class RejectedByPrimus : public std::runtime_error
    {
    public:
        RejectedByPrimus(const char* const reason) throw() :
        std::runtime_error(reason)
        { }
    };
};
