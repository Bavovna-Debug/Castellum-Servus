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
#include <iomanip>
#include <sstream>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

// Common definition files.
//
#include "Raspberry/LCD.hpp"
#include "Toolkit/Report.h"
#include "Toolkit/Times.hpp"

// Local definition files.
//
#include "Servus/Configuration.hpp"
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

    Raspberry::LCD &lcd = Raspberry::LCD::SharedInstance();

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

                if (sensor->changedTemperature == true)
                {
                    sensor->changedTemperature = false;

                    Dispatcher::DSTemperatureAviso* aviso = new Dispatcher::DSTemperatureAviso(
                            sensor->token,
                            sensor->temperature);

                    Dispatcher::Queue& queue = Dispatcher::Queue::SharedInstance();

                    queue.enqueueAviso(aviso);

                    try
                    {
                        ReportDebug("[Périphérique] Refreshing LCD");

                        std::ostringstream stringStream;
                        stringStream << std::setiosflags(std::ios::left);
                        stringStream << std::setfill(' ') << std::setw(lcd.numberOfRows - 4);
                        stringStream << sensor->title;
                        stringStream << std::fixed << std::setprecision(1);
                        stringStream << sensor->temperature;

                        lcd << stringStream.str();
                    }
                    catch (std::exception &exception)
                    {
                        ReportWarning("[Périphérique] Exception on LCD refresh: %s",
                                exception.what());
                    }
                }

                std::this_thread::sleep_for(
                        std::chrono::milliseconds { Servus::WaitBetweenDSSensors } );
            }
        }
        catch (std::exception &exception)
        {
            ReportWarning("[Périphérique] Exception on thermique sensors refresh: %s",
                    exception.what());
        }

        std::this_thread::sleep_for(
                std::chrono::milliseconds { Servus::WaitAfterAllDSSensors } );
    }

    ReportWarning("[Périphérique] Thermique station thread is going to quit");
}

Peripherique::ThermiqueStation&
Peripherique::ThermiqueStation::operator+=(Peripherique::ThermiqueSensor* sensor)
{
    ReportInfo("[Périphérique] Defined thermique sensor '%s'",
            sensor->title.c_str());

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

    directory = opendir(Raspberry::DS1820::DevicesPath.c_str());
    if (directory == NULL)
    {
        ReportError("[Périphérique] Cannot get access to '%s': errno=%d",
                Raspberry::DS1820::DevicesPath.c_str(),
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

        if (strlen(directoryEntry->d_name) != Raspberry::DS1820::DeviceIdNameLength)
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
                    0,
                    Peripherique::DefaultThermiqueSensorName);

            *this += sensor;

            ReportInfo("[Périphérique] Detected non-configured thermique sensor %s",
                    sensor->deviceId.c_str());
        }
    }

    closedir(directory);
}
