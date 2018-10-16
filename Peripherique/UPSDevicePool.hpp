#pragma once

// System definition files.
//
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

// Local definition files.
//
#include "Servus/Peripherique/UPSDevice.hpp"

namespace Peripherique
{
    /**
     * UPS device.
     */
    class UPSDevicePool
    {
    private:
        /**
         * Thread handler of dispatcher thread.
         */
        std::thread thread;

        struct
        {
            std::queue<Peripherique::UPSDevice*>    devices;
            std::mutex                              lock;
            std::condition_variable                 condition;
        }
        list;

    public:
        static Peripherique::UPSDevicePool&
        InitInstance();

        static Peripherique::UPSDevicePool&
        SharedInstance();

    private:
        UPSDevicePool();

        ~UPSDevicePool();

    public:
        void
        start();

        void
        defineUPS(Peripherique::UPSDevice*);

    private:
        static void
        ThreadHandler(Peripherique::UPSDevicePool*);
    };
};
