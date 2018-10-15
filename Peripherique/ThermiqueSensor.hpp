#pragma once

// System definition files.
//
#include <cstdbool>
#include <string>

// Common definition files.
//
#include "GPIO/Therma.hpp"

namespace Peripherique
{
    /**
     * Thermique sensor.
     */
    class ThermiqueSensor : public Therma::Sensor
    {
        typedef Therma::Sensor Inherited;

    public:
        std::string token;
        float       temperatureEdge;
        std::string title;

        bool        changedTemperature;

        struct
        {
            float   current;
            float   lowest;
            float   highest;
        }
        lastKnownTemperature;

    public:
        ThermiqueSensor(
            const std::string&  token,
            const std::string&  deviceId,
            const float         temperatureEdge,
            const std::string&  title);

        void
        refresh();
    };
};
