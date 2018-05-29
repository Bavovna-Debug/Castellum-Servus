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
        std::string         avisoType;

        Toolkit::Timestamp* timestamp;

    public:
        Aviso(const std::string& avisoType);

        Aviso(
            const std::string&  avisoType,
            const std::string&  stamp);

        ~Aviso();

        virtual void
        prepare(RTSP::Datagram&) const;
    };

    class FabulaAviso : public Dispatcher::Aviso
    {
        typedef Dispatcher::Aviso Inherited;

    public:
        std::string         fabulatorName;
        unsigned short      severityLevel;
        bool                notificationFlag;
        std::string         message;

    public:
        FabulaAviso(
            const std::string&      stamp,
            const std::string&      fabulatorName,
            const unsigned short    severityLevel,
            const bool              notificationFlag,
            const std::string&      message);

        virtual void
        prepare(RTSP::Datagram&) const;
    };

    class TemperatureAviso : public Dispatcher::Aviso
    {
        typedef Dispatcher::Aviso Inherited;

    public:
        std::string sensorToken;
        float       temperature;

    public:
        TemperatureAviso(
            const std::string&  sensorToken,
            const float         temperature);

        virtual void
        prepare(RTSP::Datagram&) const;
    };
};
