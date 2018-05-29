#pragma once

// System definition files.
//
#include <libconfig.h++>
#include <string>

namespace Servus
{
    static const std::string SoftwareVersion = "Servus 0.1 [180529]";

    static const unsigned DefaultMODBUSPortNumber                       = 502;
    static const unsigned DefaultHTTPPortNumber                         = 12080;

    static const unsigned DefaultPrimusSleepIfRejectedByPrimus          = 120;  /**< Seconds. */
    static const unsigned DefaultPrimusReconnectInterval                = 5;    /**< Seconds. */
    static const unsigned DefaultPrimusWaitForResponse                  = 5000; /**< Milliseconds. */
    static const unsigned DefaultPrimusWaitForDatagramCompletion        = 2000; /**< Milliseconds. */
    static const unsigned DefaultListenerWaitForFirstTransmission       = 1000; /**< Milliseconds. */
    static const unsigned DefaultListenerWaitForTransmissionCompletion  = 500;  /**< Milliseconds. */

    static const unsigned WaitBeforeNetworkRetry                        = 60;   /**< Seconds. */

    class Configuration
    {
    public:
        std::string         configurationFilePath;

        unsigned short      modbusPortNumber;
        unsigned short      httpPortNumber;

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
