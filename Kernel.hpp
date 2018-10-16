#pragma once

// System definition files.
//
#include <string>

// Common definition files.
//
#include "HTTP/Service.hpp"
#include "MODBUS/Service.hpp"
#include "Toolkit/Workspace.hpp"
#include "Toolkit/Times.hpp"

// Local definition files.
//
#include "Servus/GKrellM.hpp"

namespace Workspace
{
    static const std::string RootPath = "/opt/castellum/";

    class Kernel : public Application
    {
        typedef Application Inherited;

    public:
        /**
         * UNIX version 6 timestamp representing the time the Servus has been started
         * (in seconds since 1970-01-01 00:00:00 GMT).
         */
        Toolkit::Timestamp*     timestampOfStart;

        std::string             systemName;

        struct GKrellM_Service  gkrellm;
        MODBUS::Service*        modbus;
        HTTP::Service*          http;

    public:
        static Kernel&
        InitInstance();

        static Kernel&
        SharedInstance();

        Kernel();

    protected:
        void
        kernelInit(),
        kernelExec(),
        kernelWait(),
        kernelDone();
    };
};
