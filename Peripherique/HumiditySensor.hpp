#pragma once

// System definition files.
//
#include <cstdbool>
#include <string>

// Common definition files.
//
#include "Raspberry/DHT.hpp"

namespace Peripherique
{
    /**
     * Humidity sensor.
     */
    class HumiditySensor : public Raspberry::DHT
    {
        typedef Raspberry::DHT Inherited;

    public:
        std::string token;
        float       humidityEdge;
        float       temperatureEdge;
        std::string title;

        bool        changedHumidity;
        bool        changedTemperature;

        struct
        {
            float   current;
            float   lowest;
            float   highest;
        }
        lastKnownHumidity;

        struct
        {
            float   current;
            float   lowest;
            float   highest;
        }
        lastKnownTemperature;

    public:
        HumiditySensor(
            const std::string&  token,
            const unsigned int  gpioPinNumber,
            const float         humidityEdge,
            const float         temperatureEdge,
            const std::string&  title);

        void
        refresh();
    };
};
