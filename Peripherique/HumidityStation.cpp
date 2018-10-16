// System definition files.
//
#include <cstdbool>
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
#include "Servus/Peripherique/HumiditySensor.hpp"
#include "Servus/Peripherique/HumidityStation.hpp"

static Peripherique::HumidityStation *instance = NULL;

Peripherique::HumidityStation&
Peripherique::HumidityStation::InitInstance()
{
    if (instance != NULL)
        throw std::runtime_error("[Périphérique] Already initialized");

    instance = new Peripherique::HumidityStation();

    return *instance;
}

Peripherique::HumidityStation&
Peripherique::HumidityStation::SharedInstance()
{
    if (instance == NULL)
        throw std::runtime_error("[Périphérique] Not initialized");

    return *instance;
}

Peripherique::HumidityStation::HumidityStation()
{ }

Peripherique::HumidityStation::~HumidityStation()
{ }

void
Peripherique::HumidityStation::startService()
{
    this->thread = std::thread(&this->ThreadHandler, this);
}

void
Peripherique::HumidityStation::stopService()
{
    pthread_cancel(this->thread.native_handle());

    this->thread.join();
}

/**
 * @brief   Thread handler for therma service.
 */
void
Peripherique::HumidityStation::ThreadHandler(Peripherique::HumidityStation* humidityStation)
{
    ReportNotice("[Périphérique] Humidity station thread has been started");

    Raspberry::LCD &lcd = Raspberry::LCD::SharedInstance();

    for (;;)
    {
        ReportDebug("[Périphérique] Refreshing humidity sensors");

        try
        {
            for (unsigned int sensorIndex = 0;
                 sensorIndex < humidityStation->size();
                 sensorIndex++)
            {
                Peripherique::HumiditySensor *sensor = (*humidityStation)[sensorIndex];

                sensor->refresh();

                if (sensor->changedHumidity == true)
                {
                    sensor->changedHumidity = false;

                    Dispatcher::DHTHumidityAviso* aviso = new Dispatcher::DHTHumidityAviso(
                            sensor->token,
                            sensor->humidity);

                    Dispatcher::Queue& queue = Dispatcher::Queue::SharedInstance();

                    queue.enqueueAviso(aviso);
                }

                if (sensor->changedTemperature == true)
                {
                    sensor->changedTemperature = false;

                    Dispatcher::DHTTemperatureAviso* aviso = new Dispatcher::DHTTemperatureAviso(
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

                    std::this_thread::sleep_for(
                            std::chrono::milliseconds { Servus::WaitBetweenDHTSensors } );
                }
            }
        }
        catch (std::exception &exception)
        {
            ReportWarning("[Périphérique] Exception on humidity sensors refresh: %s",
                    exception.what());
        }

        std::this_thread::sleep_for(
                std::chrono::milliseconds { Servus::WaitAfterAllDHTSensors } );
    }

    ReportWarning("[Périphérique] Humidity station thread is going to quit");
}

Peripherique::HumidityStation&
Peripherique::HumidityStation::operator+=(Peripherique::HumiditySensor* sensor)
{
    ReportInfo("[Périphérique] Defined humidity sensor '%s'",
            sensor->title.c_str());

    this->sensors.push_back(sensor);

    return *this;
}

Peripherique::HumiditySensor*
Peripherique::HumidityStation::operator[](const unsigned int sensorIndex)
{
    return this->sensors[sensorIndex];
}

unsigned int
Peripherique::HumidityStation::size()
{
    return this->sensors.size();
}
