// System definition files.
//
#include <unistd.h>
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
#include "HTTP/Service.hpp"
#include "MODBUS/Service.hpp"
#include "Toolkit/Report.h"
#include "Toolkit/Signals.hpp"
#include "Toolkit/Times.hpp"

// Local definition files.
//
#include "Servus/Configuration.hpp"
#include "Servus/GKrellM.hpp"
#include "Servus/Kernel.hpp"
#include "Servus/Dispatcher/Communicator.hpp"
#include "Servus/Dispatcher/Queue.hpp"
#include "Servus/Peripherique/ThermiqueSensor.hpp"
#include "Servus/Peripherique/ThermiqueStation.hpp"
#include "Servus/Peripherique/UPSDevicePool.hpp"
#include "Servus/WWW/Home.hpp"
#include "Servus/WWW/SessionManager.hpp"

static void
OwnSignalHandler(int signalNumber);

static Workspace::Kernel* instance = NULL;

Workspace::Kernel&
Workspace::Kernel::InitInstance()
{
    if (instance != NULL)
        throw std::runtime_error("Kernel already initialized");

    instance = new Workspace::Kernel();

    return *instance;
}

Workspace::Kernel&
Workspace::Kernel::SharedInstance()
{
    if (instance == NULL)
        throw std::runtime_error("Kernel not initialized");

    return *instance;
}

Workspace::Kernel::Kernel() :
Inherited(Servus::InstanceName)
{
    this->timestampOfStart = new Toolkit::Timestamp();
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
    int rc = GKrellM_Start(&this->gkrellm);
    if (rc != 0)
    {
        ReportError("[Kernel] Cannot start GKrellM service - quit");

        throw std::runtime_error("Cannot start GKrellM service");
    }

    try
    {
        GPIO::RelayStation::InitInstance();
        GPIO::Strip::InitInstance();
        GPIO::LCD::InitInstance(GPIO::LineLength2004);
        Peripherique::ThermiqueStation::InitInstance();
        Peripherique::UPSDevicePool::InitInstance();
        WWW::SessionManager::InitInstance();
    }
    catch (std::exception& exception)
    {
        ReportWarning("[Kernel] Exception: %s", exception.what());
    }

    try
    {
        Servus::Configuration::InitInstance("/opt/castellum/servus.conf");
        Dispatcher::Communicator::InitInstance();
        Dispatcher::Queue::InitInstance();
    }
    catch (std::exception& exception)
    {
        ReportWarning("[Workspace] Exception: %s", exception.what());
    }

    Servus::Configuration& configuration = Servus::Configuration::SharedInstance();

    configuration.load();

    this->modbus = new MODBUS::Service(configuration.modbus.portNumber);

    // Start HTTP service.
    //
    this->http = new HTTP::Service(new WWW::Site(),
            IP::IPv4,
            "",
            configuration.http.portNumber,
            0x9000,
            0xAA00,
            this->timestampOfStart->seconds());

#if 0
    Servus::Service* fabulatorium = new Servus::Service(
        "Fabulatorium",
        Communicator::IPv4,
        configuration.fabulatoriumPortNumber);

    fabulatorium->startService();
#endif

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
        GPIO::Strip::SharedInstance().startService();

        Dispatcher::Communicator::SharedInstance().start();

        Peripherique::UPSDevicePool::SharedInstance().start();

        this->http->startService();
    }
    catch (...)
    {
        ReportError("[Workspace] Error has occurred starting services");
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
        ReportError("[Workspace] Error has occurred waiting for services");
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
        ReportError("[Workspace] Error has occurred trying to stop services");
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
    ReportNotice("[Workspace] Received signal to quit: signal=%d", signalNumber);

    // Raise signal again to let it be handled by default signal handler.
    //
    raise(SIGHUP);
}
