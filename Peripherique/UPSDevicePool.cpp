// System definition files.
//
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>

// Common definition files.
//
#include "Toolkit/Report.h"

// Local definition files.
//
#include "Servus/Peripherique/UPSDevice.hpp"
#include "Servus/Peripherique/UPSDevicePool.hpp"

static Peripherique::UPSDevicePool* instance = NULL;

Peripherique::UPSDevicePool&
Peripherique::UPSDevicePool::InitInstance()
{
    if (instance != NULL)
        throw std::runtime_error("UPSDevicePool already initialized");

    instance = new Peripherique::UPSDevicePool();

    return *instance;
}

Peripherique::UPSDevicePool&
Peripherique::UPSDevicePool::SharedInstance()
{
    if (instance == NULL)
        throw std::runtime_error("UPSDevicePool not initialized");

    return *instance;
}

Peripherique::UPSDevicePool::UPSDevicePool()
{ }

Peripherique::UPSDevicePool::~UPSDevicePool()
{ }

void
Peripherique::UPSDevicePool::start()
{
    this->thread = std::thread(&Peripherique::UPSDevicePool::ThreadHandler, this);
}

void
Peripherique::UPSDevicePool::defineUPS(Peripherique::UPSDevice* device)
{
    std::unique_lock<std::mutex> queueLock { this->list.lock };

    this->list.devices.push(device);
}

/**
 * @brief   Thread handler for service.
 */
void
Peripherique::UPSDevicePool::ThreadHandler(Peripherique::UPSDevicePool* pool)
{
    ReportDebug("[Peripherique] UPSDevicePool thread has been started");

    ReportWarning("[Peripherique] UPSDevicePool thread is going to quit");
}
