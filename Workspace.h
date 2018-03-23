#pragma once

// Common definition files.
//
#include "GPIO/Therma.h"
#include "HTTP/Service.h"
#include "MMPS/MMPS.h"
#include "MODBUS/Service.h"
#include "Toolkit/Workspace.h"

// Local definition files.
//
#include "Servus/GKrellM.h"

namespace Workspace
{
    class Servus : public Application
    {
        typedef Application Inherited;

    public:
        /**
         * UNIX version 6 timestamp representing the time the Servus has been started
         * (in seconds since 1970-01-01 00:00:00 GMT).
         */
        unsigned long           timestampOfStart;

        struct GKrellM_Service  gkrellm;
        MODBUS::Service         *modbus;
        HTTP::Service           *http;

    public:
        static Servus &
        InitInstance();

        static Servus &
        SharedInstance();

        Servus();

    public:
        void
        initializeMMPS();

    protected:
        void
        kernelInit(),
        kernelExec(),
        kernelWait(),
        kernelDone();
    };
};
