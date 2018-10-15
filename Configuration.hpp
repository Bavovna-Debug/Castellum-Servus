#pragma once

// System definition files.
//
#include <libconfig.h++>
#include <string>

namespace Servus
{
    static const std::string InstanceName       = "servus";

    static const std::string SoftwareVersion    = "Servus 0.4 [180610]";

    static const unsigned DefaultMODBUSPortNumberIPv4               = 502;

    static const unsigned DefaultHTTPPortNumberIPv4                 = 15080;
    static const unsigned DefaultHTTPKeepAliveSession               = 3600;     /**< Seconds. */

    static const unsigned DefaultPrimusSleepIfRejectedByPrimus      = 120;      /**< Seconds. */
    static const unsigned DefaultPrimusReconnectInterval            = 5;        /**< Seconds. */
    static const unsigned DefaultPrimusWaitForResponse              = 5000;     /**< Milliseconds. */
    static const unsigned DefaultPrimusWaitForDatagramCompletion    = 2000;     /**< Milliseconds. */
    static const unsigned DefaultListenerWaitForFirstTransmission   = 1000;     /**< Milliseconds. */
    static const unsigned DefaultListenerWaitForTransmissionCompletion = 500;   /**< Milliseconds. */

    static const unsigned WaitBeforeNetworkRetry                    = 60;       /**< Seconds. */

    class Configuration
    {
    public:
        std::string             configurationFilePath;

        struct
        {
            unsigned short      portNumber;
        }
        modbus;

        struct
        {
            unsigned short      portNumber;
            std::string         passwordMD5;
            unsigned int        keepAliveSession;
        }
        http;

    public:
        static Configuration&
        InitInstance(const std::string& configurationFilePath);

        static Configuration&
        SharedInstance();

    private:
        Configuration(const std::string& configurationFilePath);

    public:
        void
        load();

    private:
        libconfig::Config&
        open();

        void
        close(libconfig::Config&);

        void
        flush(libconfig::Config&);
    };
};
