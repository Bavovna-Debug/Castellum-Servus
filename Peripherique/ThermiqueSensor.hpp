#pragma once

// System definition files.
//
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
        std::string name;

        float       edge;

        struct
        {
            float   current;
            float   lowest;
            float   highest;
        }
        lastKnown;

    public:
        ThermiqueSensor(
            const std::string&  deviceId,
            const std::string&  token,
            const std::string&  name,
            const float         edge);

        void
        refresh();
    };
};
