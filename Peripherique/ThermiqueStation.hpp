#pragma once

// System definition files.
//
#include <string>
#include <thread>
#include <vector>

// Local definition files.
//
#include "Servus/Peripherique/ThermiqueSensor.hpp"

namespace Peripherique
{
    static const std::string DefaultThermiqueSensorName = "Anonymous";

    /**
     * Thermique station.
     */
    class ThermiqueStation
    {
    private:
        /**
         * Thread handler of service thread.
         */
        std::thread thread;

        std::vector<Peripherique::ThermiqueSensor*> sensors;

    public:
        static Peripherique::ThermiqueStation&
        InitInstance();

        static Peripherique::ThermiqueStation&
        SharedInstance();

        ThermiqueStation();

        ~ThermiqueStation();

        void
        startService(),
        stopService();

    private:
        static void
        ThreadHandler(Peripherique::ThermiqueStation*);

    public:
        ThermiqueStation&
        operator+=(Peripherique::ThermiqueSensor*);

        Peripherique::ThermiqueSensor*
        operator[](const unsigned int);

        unsigned int
        size();

        void
        getListOfSensors();
    };
};
