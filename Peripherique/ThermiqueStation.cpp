// System definition files.
//
#include <dirent.h>
#include <endian.h>
#include <sys/types.h>
#include <cerrno>
#include <cstdbool>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <system_error>
#include <thread>
#include <vector>

// Common definition files.
//
#include "GPIO/LCD.hpp"
#include "Toolkit/Report.h"
#include "Toolkit/Times.hpp"

// Local definition files.
//
#include "Servus/Dispatcher/Aviso.hpp"
#include "Servus/Dispatcher/Queue.hpp"
#include "Servus/Peripherique/ThermiqueSensor.hpp"
#include "Servus/Peripherique/ThermiqueStation.hpp"

static Peripherique::ThermiqueStation *instance = NULL;

Peripherique::ThermiqueStation&
Peripherique::ThermiqueStation::InitInstance()
{
    if (instance != NULL)
        throw std::runtime_error("[Périphérique] Already initialized");

    instance = new Peripherique::ThermiqueStation();

    return *instance;
}

Peripherique::ThermiqueStation&
Peripherique::ThermiqueStation::SharedInstance()
{
    if (instance == NULL)
        throw std::runtime_error("[Périphérique] Not initialized");

    return *instance;
}

Peripherique::ThermiqueStation::ThermiqueStation()
{ }

Peripherique::ThermiqueStation::~ThermiqueStation()
{ }

void
Peripherique::ThermiqueStation::startService()
{
    this->thread = std::thread(&this->ThreadHandler, this);
}

void
Peripherique::ThermiqueStation::stopService()
{
    pthread_cancel(this->thread.native_handle());

    this->thread.join();
}

/**
 * @brief   Thread handler for therma service.
 */
void
Peripherique::ThermiqueStation::ThreadHandler(Peripherique::ThermiqueStation* thermiqueStation)
{
    ReportNotice("[Périphérique] Thermique station thread has been started");

    GPIO::LCD &lcd = GPIO::LCD::SharedInstance();

    thermiqueStation->getListOfSensors();

    for (;;)
    {
        ReportDebug("[Périphérique] Refreshing thermique sensors");

        try
        {
            for (unsigned int sensorIndex = 0;
                 sensorIndex < thermiqueStation->size();
                 sensorIndex++)
            {
                Peripherique::ThermiqueSensor *sensor = (*thermiqueStation)[sensorIndex];

                sensor->refresh();

                if (sensor->changed == true)
                {
                    sensor->changed = false;

                    Dispatcher::TemperatureAviso* aviso = new Dispatcher::TemperatureAviso(
                            sensor->token,
                            sensor->temperature);

                    Dispatcher::Queue& queue = Dispatcher::Queue::SharedInstance();

                    queue.enqueueAviso(aviso);
                }
            }
        }
        catch (std::exception &exception)
        {
            ReportWarning("[Périphérique] Exception on thermique sensors refresh: %s",
                    exception.what());
        }

        ReportDebug("[Périphérique] Refreshing LCD");

        try
        {
            lcd.refresh();
        }
        catch (std::exception &exception)
        {
            ReportWarning("[Périphérique] Exception on LCD refresh: %s",
                    exception.what());
        }
    }

    ReportWarning("[Périphérique] Thermique station thread is going to quit");
}

Peripherique::ThermiqueStation&
Peripherique::ThermiqueStation::operator+=(Peripherique::ThermiqueSensor* sensor)
{
    ReportInfo("[Périphérique] Defined thermique sensor '%s'", sensor->name.c_str());

    this->sensors.push_back(sensor);

    return *this;
}

Peripherique::ThermiqueSensor*
Peripherique::ThermiqueStation::operator[](const unsigned int sensorIndex)
{
    return this->sensors[sensorIndex];
}

unsigned int
Peripherique::ThermiqueStation::size()
{
    return this->sensors.size();
}

void
Peripherique::ThermiqueStation::getListOfSensors()
{
    DIR             *directory;
    struct dirent   *directoryEntry;

    directory = opendir(Therma::SensorsPath.c_str());
    if (directory == NULL)
    {
        ReportError("[Périphérique] Cannot get access to '%s': errno=%d",
                Therma::SensorsPath.c_str(),
                errno);

        throw;
    }

    for (;;)
    {
        directoryEntry = readdir(directory);
        if (directoryEntry == NULL)
        {
            if (errno == 0)
            {
                break;
            }
            else
            {
                ReportError("[Périphérique] Cannot get directory entry: errno=%d",
                        errno);

                throw;
            }
        }

        if (directoryEntry->d_type != DT_LNK)
            continue;

        if (strlen(directoryEntry->d_name) != Therma::DeviceIdNameLength)
            continue;

        Peripherique::ThermiqueSensor* sensor = NULL;

        for (unsigned int sensorIndex = 0;
             sensorIndex < this->size();
             sensorIndex++)
        {
            sensor = (*this)[sensorIndex];
            if (strcmp(sensor->deviceId.c_str(), directoryEntry->d_name) == 0)
                break;
        }

        if (sensor == NULL)
        {
            sensor = new Peripherique::ThermiqueSensor(
                    "",
                    directoryEntry->d_name,
                    Peripherique::DefaultThermiqueSensorName,
                    0);

            *this += sensor;

            ReportInfo("[Périphérique] Detected non-configured thermique sensor %s",
                    sensor->deviceId.c_str());
        }
    }

    closedir(directory);
}
