// System definition files.
//
#include <cstdbool>
#include <system_error>

// Common definition files.
//
#include "GPIO/LCD.hpp"
#include "GPIO/Therma.hpp"
#include "Toolkit/Report.h"

// Local definition files.
//
#include "Servus/Peripherique/ThermiqueSensor.hpp"

Peripherique::ThermiqueSensor::ThermiqueSensor(
    const std::string&  token,
    const std::string&  deviceId,
    const float         temperatureEdge,
    const std::string&  title) :
Inherited(deviceId),
token(token),
temperatureEdge(temperatureEdge),
title(title)
{
    this->lastKnownTemperature.current = 0.0;
    this->lastKnownTemperature.lowest = Therma::DefaultMinimalThemperature;
    this->lastKnownTemperature.highest = Therma::DefaultMaximalThemperature;

    this->changedTemperature = false;
}

void
Peripherique::ThermiqueSensor::refresh()
{
    Inherited::refresh();

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
