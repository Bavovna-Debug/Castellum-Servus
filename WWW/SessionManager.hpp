#pragma once

// System definition files.
//
#include <cstdbool>
#include <string>
#include <vector>

namespace WWW
{
    class Session;

    class SessionManager
    {
    private:
        std::vector<WWW::Session*> sessions;

    public:
        static WWW::SessionManager&
        InitInstance();

        static WWW::SessionManager&
        SharedInstance();

        SessionManager();

        bool
        login(
            const std::string& guestAddress,
            const std::string& clearTextPassword);

        bool
        permitted(const std::string& guestAddress);
    };

    class Session
    {
    public:
        std::string guestAddress;

        /**
         * UNIX version 6 timestamp representing the time when session has been touched last time
         * (in seconds since 1970-01-01 00:00:00 GMT).
         */
        unsigned long lastTouch;

    public:
        Session(const std::string& guestAddress);

        void
        touch();

        unsigned long
        lifeTime();
    };
};
