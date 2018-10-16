#pragma once

// System definition files.
//
#include <string>
#include <thread>
#include <vector>

// Local definition files.
//
#include "Servus/Peripherique/HumiditySensor.hpp"

namespace Peripherique
{
    /**
     * Humidity station.
     */
    class HumidityStation
    {
    private:
        /**
         * Thread handler of service thread.
         */
        std::thread thread;

        std::vector<Peripherique::HumiditySensor*> sensors;

    public:
        static Peripherique::HumidityStation&
        InitInstance();

        static Peripherique::HumidityStation&
        SharedInstance();

        HumidityStation();

        ~HumidityStation();

        void
        startService(),
        stopService();

    private:
        static void
        ThreadHandler(Peripherique::HumidityStation*);

    public:
        Peripherique::HumidityStation&
        operator+=(Peripherique::HumiditySensor*);

        Peripherique::HumiditySensor*
        operator[](const unsigned int);

        unsigned int
        size();
    };
};
