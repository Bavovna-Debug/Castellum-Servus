// System definition files.
//
#include <unistd.h>
#include <libconfig.h++>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

// Common definition files.
//
#include "Communicator/IP.hpp"
#include "GPIO/GPIO.hpp"
#include "GPIO/LCD.hpp"
#include "GPIO/Relay.hpp"
#include "GPIO/Strip.hpp"
#include "GPIO/Therma.hpp"
#include "HTTP/Service.hpp"
#include "MODBUS/Service.hpp"
#include "Toolkit/MMPS.hpp"
#include "Toolkit/Report.h"
#include "Toolkit/Signals.hpp"

// Local definition files.
//
#include "Servus/Configuration.hpp"
#include "Servus/GKrellM.hpp"
#include "Servus/Kernel.hpp"
#include "Servus/WWW/Home.hpp"

using namespace libconfig;

static void
OwnSignalHandler(int signalNumber);

static Workspace::Kernel* instance = NULL;

Workspace::Kernel&
Workspace::Kernel::InitInstance()
{
    if (instance != NULL)
        throw std::runtime_error("Workspace already initialized");

    instance = new Workspace::Kernel();

    return *instance;
}

Workspace::Kernel&
Workspace::Kernel::SharedInstance()
{
    if (instance == NULL)
        throw std::runtime_error("Workspace not initialized");

    return *instance;
}

Workspace::Kernel::Kernel() :
Inherited()
{
    try
    {
        GPIO::RelayStation::InitInstance();
        GPIO::Strip::InitInstance();
        GPIO::LCD::InitInstance(GPIO::LineLength2004);
        Therma::Service::InitInstance();
    }
    catch (std::exception &exception)
    {
        ReportWarning("[Kernel] Exception: %s", exception.what());
    }
}

/**
 * @brief   Initialize MMPS pools.
 *
 * @return  0 upon successful completion.
 * @return  Error code (negative value), in case of error.
 */
void
Workspace::Kernel::initializeMMPS()
{
    this->http->mmps = MMPS::InitPool(1);
    if (this->http->mmps == NULL)
    {
        ReportError("[Kernel] Cannot create MMPS pool");

        throw std::runtime_error("MMPS");
    }

    int rc;

    rc = MMPS::InitBank(
            this->http->mmps,
            0,
            1024,
            0,
            1000);
    if (rc != 0)
    {
        ReportError("[Kernel] Cannot create MMPS bank: rc=%d", rc);

        throw std::runtime_error("MMPS");
    }

    rc = MMPS::AllocateImmediately(
            this->http->mmps,
            0);
    if (rc != 0)
    {
        ReportError("[Kernel] Cannot allocate memory for MMPS bank: rc=%d", rc);

        throw std::runtime_error("MMPS");
    }
}

/**
 * @brief   1st part of application kernel - initialize all resources.
 *
 * @return  0 upon successful completion.
 * @return  Error code (negative value), in case of error.
 */
void
Workspace::Kernel::kernelInit()
{
    // Start GKrellM service.
    //
    {
        int rc = GKrellM_Start(&this->gkrellm);
        if (rc != 0)
        {
            ReportError("[Kernel] Cannot start GKrellM service - quit");

            throw std::runtime_error("Cannot start GKrellM service");
        }
    }

    this->modbus = new MODBUS::Service(Configuration::MODBUSPortNumber);

    try
    {
        Config config;
        config.readFile("/opt/servus/servus.conf");

        Setting& gpio = config.lookup("GPIO");

        Setting& relays = gpio.lookup("Relays");

        {
            GPIO::RelayStation &relayStation = GPIO::RelayStation::SharedInstance();

            for (int relayIndex = 0;
                 relayIndex < relays.getLength();
                 relayIndex++)
            {
                Setting& relay = relays[relayIndex];

                const char*     relayName = relay.lookup("Name");
                unsigned int    pinNumber = relay.lookup("GPIO");

                relayStation += new GPIO::Relay(pinNumber, relayName);
            }
        }

        Setting& therma = gpio.lookup("Therma");

        {
            Therma::Service& thermaService = Therma::Service::SharedInstance();

            for (int sensorIndex = 0;
                 sensorIndex < therma.getLength();
                 sensorIndex++)
            {
                Setting& sensor = therma[sensorIndex];

                const char*     thermaId = sensor.lookup("Id");
                const char*     thermaName = sensor.lookup("Name");
                unsigned int    modbusUnitId = sensor.lookup("MODBUS");

                thermaService += new Therma::Sensor(thermaId, thermaName, modbusUnitId);
            }
        }
    }
    catch (std::exception& exception)
    {
        ReportWarning("[Kernel] Exception on configuration: %s", exception.what());
    }

    // Start HTTP service.
    //
    {
        this->http = new HTTP::Service(new WWW::Site(),
                IP::IPv4,
                "",
                9000,
                0x9000,
                0xAA00,
                0xDEADBEEF);

        this->initializeMMPS();
    }

    Toolkit::SetSignalCaptureOn(SIGINT, OwnSignalHandler);
    Toolkit::SetSignalCaptureOn(SIGTERM, OwnSignalHandler);
}

const unsigned int rows[4] = { 05, 06, 12, 13 }; //{ 29, 31, 32, 33 };
const unsigned int cols[4] = { 16, 17, 18, 19 }; //{ 36, 11, 12, 35 };

/**
 * @brief   2nd part of application kernel - start all subcomponents.
 *
 * @return  0 upon successful completion.
 * @return  Error code (negative value), in case of error.
 */
void
Workspace::Kernel::kernelExec()
{
#if 0
    {
        ReportInfo("ZZZ");

        unsigned int row, col, value;

        for (;;)
        {
            usleep(200000);

            for (col = 0; col < 4; col++)
            {
                GPIO_Unexport(cols[col]);
                GPIO_Export(cols[col]);
                GPIO_Direction(cols[col], GPIO_OUT);
                GPIO_Set(cols[col], GPIO_LOW);
            }

            for (row = 0; row < 4; row++)
            {
                GPIO_Unexport(rows[row]);
                GPIO_Export(rows[row]);
                GPIO_Direction(rows[row], GPIO_IN);
                GPIO_Edge(rows[row], GPIO_EDGE_FALLING);
            }

            for (row = 0; row < 4; row++)
            {
                GPIO_Get(rows[row], &value);
                if (value == GPIO_LOW)
                {
                    ReportInfo("ZZZ row=%u", row);
                    break;
                }
            }

            if (row == 4)
                continue;

            for (col = 0; col < 4; col++)
            {
                GPIO_Direction(cols[col], GPIO_IN);
                //GPIO_Edge(cols[col], GPIO_EDGE_RISING);
            }

            //GPIO_Direction(rows[row], GPIO_OUT);
            GPIO_Set(rows[row], GPIO_HIGH);

            for (col = 0; col < 4; col++)
            {
                GPIO_Get(cols[col], &value);
                if (value == GPIO_HIGH)
                {
                    ReportInfo("ZZZ col=%u", col);
                    break;
                }
            }
        }
    }
#endif

    try
    {
        GPIO::Strip& stripService = GPIO::Strip::SharedInstance();

        stripService.startService();

        Therma::Service& thermaService = Therma::Service::SharedInstance();

        thermaService.startService();

        this->http->startService();
    }
    catch (...)
    {
        ReportError("[Kernel] Error has occurred starting services - quit!");
    }
}

/**
 * @brief   3rd part of application kernel - waiting for completion of services.
 *
 * @return  0 upon successful completion.
 * @return  Error code (negative value), in case of error.
 */
void
Workspace::Kernel::kernelWait()
{
    try
    {
        this->http->waitForService();
    }
    catch (...)
    {
        ReportError("[Kernel] Error has occurred waiting for services - quit!");
    }
}

/**
 * @brief   4th part of application kernel - release all resources.
 *
 * @return  0 upon successful completion.
 * @return  Error code (negative value), in case of error.
 */
void
Workspace::Kernel::kernelDone()
{
    Toolkit::SetSignalCaptureOff(SIGINT);
    Toolkit::SetSignalCaptureOff(SIGTERM);

    try
    {
        this->http->stopService();
    }
    catch (...)
    {
        ReportError("[Kernel] Error has occurred trying to stop services");
    }
}

/**
 * @brief   POSIX signal handler.
 *
 * Handle POSIX process signals to let procurator release all resources.
 *
 * @param   signalNumber    POSIX signal number.
 */
static void
OwnSignalHandler(int signalNumber)
{
    ReportNotice("[Kernel] Received signal to quit: signal=%d", signalNumber);

    // Raise signal again to let it be handled by default signal handler.
    //
    raise(SIGHUP);
}
