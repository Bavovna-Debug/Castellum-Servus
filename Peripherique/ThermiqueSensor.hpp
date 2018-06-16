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
        float       edge;
        std::string title;

        bool        changed;

        struct
        {
            float   current;
            float   lowest;
            float   highest;
        }
        lastKnown;

    public:
        ThermiqueSensor(
            const std::string&  token,
            const std::string&  deviceId,
            const float         edge,
            const std::string&  title);

        void
        refresh();
    };
};
