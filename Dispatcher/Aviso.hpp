#pragma once

// System definition files.
//
#include <cstdbool>
#include <string>

// Common definition files.
//
#include "RTSP/RTSP.hpp"
#include "Toolkit/Times.hpp"

namespace Dispatcher
{
    class Aviso
    {
    public:
        unsigned int        avisoId;

        Toolkit::Timestamp* timestamp;

        std::string         fabulatorName;
        unsigned short      severityLevel;
        bool                notificationFlag;
        std::string         message;

    public:
        Aviso(
            const std::string&      stamp,
            const std::string&      fabulatorName,
            const unsigned short    severityLevel,
            const bool              notificationFlag,
            const std::string&      message);

        ~Aviso();

        void
        prepare(RTSP::Datagram&);
    };
};
