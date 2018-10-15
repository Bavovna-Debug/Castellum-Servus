// System definition files.
//
#include <cstdbool>
#include <system_error>

// Common definition files.
//
#include "GPIO/DHT.hpp"
#include "GPIO/LCD.hpp"
#include "GPIO/Therma.hpp"
#include "Toolkit/Report.h"

// Local definition files.
//
#include "Servus/Peripherique/HumiditySensor.hpp"

Peripherique::HumiditySensor::HumiditySensor(
    const std::string&  token,
    const unsigned int  gpioPinNumber,
    const float         humidityEdge,
    const float         temperatureEdge,
    const std::string&  title) :
Inherited(gpioPinNumber),
token(token),
humidityEdge(humidityEdge),
temperatureEdge(temperatureEdge),
title(title)
{
    this->lastKnownHumidity.current = 0.0;
    this->lastKnownHumidity.lowest = Therma::DefaultMinimalHumidity;
    this->lastKnownHumidity.highest = Therma::DefaultMaximalHumidity;

    this->lastKnownTemperature.current = 0.0;
    this->lastKnownTemperature.lowest = Therma::DefaultMinimalThemperature;
    this->lastKnownTemperature.highest = Therma::DefaultMaximalThemperature;

    this->changedHumidity = false;
    this->changedTemperature = false;
}

void
Peripherique::HumiditySensor::refresh()
{
    Inherited::refresh();

    {
        if (this->humidity < this->lastKnownHumidity.lowest)
        {
            this->lastKnownHumidity.lowest = this->humidity;
        }

        if (this->humidity > this->lastKnownHumidity.highest)
        {
            this->lastKnownHumidity.highest = this->humidity;
        }

        if (this->lastKnownHumidity.current != this->humidity)
        {
            if ((this->humidity - this->lastKnownHumidity.current >= this->humidityEdge) ||
                (this->humidity - this->lastKnownHumidity.current <= -(this->humidityEdge)))
            {
                ReportDebug("[Périphérique] Humidity of '%s' has changed: %.2f -> %.2f (%.2f / %.2f)",
                        this->name.c_str(),
                        this->lastKnownHumidity.current,
                        this->humidity,
                        this->lastKnownHumidity.lowest,
                        this->lastKnownHumidity.highest);

                this->lastKnownHumidity.current = this->humidity;

                this->changedHumidity = true;
            }
        }
    }

    {
        if (this->temperature < this->lastKnownTemperature.lowest)
        {
            this->lastKnownTemperature.lowest = this->temperature;
        }

        if (this->temperature > this->lastKnownTemperature.highest)
        {
            this->lastKnownTemperature.highest = this->temperature;
        }

        if (this->lastKnownTemperature.current != this->temperature)
        {
            if ((this->temperature - this->lastKnownTemperature.current >= this->temperatureEdge) ||
                (this->temperature - this->lastKnownTemperature.current <= -(this->temperatureEdge)))
            {
                ReportDebug("[Périphérique] Temperature of '%s' has changed: %.2f -> %.2f (%.2f / %.2f)",
                        this->name.c_str(),
                        this->lastKnownTemperature.current,
                        this->temperature,
                        this->lastKnownTemperature.lowest,
                        this->lastKnownTemperature.highest);

                this->lastKnownTemperature.current = this->temperature;

                this->changedTemperature = true;
            }
        }
    }
}
