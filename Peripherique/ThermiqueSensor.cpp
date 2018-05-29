// System definition files.
//
#include <dirent.h>
#include <endian.h>
#include <sys/types.h>
#include <cerrno>
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
#include "GPIO/Therma.hpp"
#include "Toolkit/Report.h"

// Local definition files.
//
#include "Servus/Peripherique/ThermiqueSensor.hpp"

Peripherique::ThermiqueSensor::ThermiqueSensor(
    const std::string&  deviceId,
    const std::string&  token,
    const std::string&  name,
    const float         edge) :
Inherited(deviceId),
token(token),
name(name),
edge(edge)
{
    this->lastKnown.current = 0.0;
    this->lastKnown.lowest = Therma::DefaultMinimalThemperature;
    this->lastKnown.highest = Therma::DefaultMaximalThemperature;
}

void
Peripherique::ThermiqueSensor::refresh()
{
    Inherited::refresh();

    if (this->temperature < this->lastKnown.lowest)
    {
        this->lastKnown.lowest = this->temperature;
    }

    if (this->temperature > this->lastKnown.highest)
    {
        this->lastKnown.highest = this->temperature;
    }

    if (this->lastKnown.current != this->temperature)
    {
        if ((temperature - this->lastKnown.current >= this->edge) ||
            (temperature - this->lastKnown.current <= -this->edge))
        {
            ReportDebug("[Périphérique] Temperature of '%s' has changed: %.2f -> %.2f (%.2f / %.2f)",
                    this->name.c_str(),
                    this->lastKnown.current,
                    this->temperature,
                    this->lastKnown.lowest,
                    this->lastKnown.highest);

            this->lastKnown.current = this->temperature;
        }
    }
}
