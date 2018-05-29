// System definition files.
//
#include <chrono>
#include <condition_variable>
#include <cstdbool>
#include <cstring>
#include <mutex>
#include <queue>

// Common definition files.
//
#include "Toolkit/Report.h"

// Local definition files.
//
#include "Servus/Dispatcher/Aviso.hpp"
#include "Servus/Dispatcher/Queue.hpp"

static Dispatcher::Queue* instance = NULL;

Dispatcher::Queue&
Dispatcher::Queue::InitInstance()
{
    if (instance != NULL)
        throw std::runtime_error("[Dispatcher] Already initialized");

    instance = new Dispatcher::Queue();

    return *instance;
}

Dispatcher::Queue&
Dispatcher::Queue::SharedInstance()
{
    if (instance == NULL)
        throw std::runtime_error("[Dispatcher] Not initialized");

    return *instance;
}

Dispatcher::Queue::Queue()
{
    this->lastAvisoId = 0;
}

Dispatcher::Queue::~Queue()
{ }

std::cv_status
Dispatcher::Queue::wait(const std::chrono::milliseconds duration)
{
    std::unique_lock<std::mutex> queueLock { this->queue.lock, std::defer_lock };

    std::cv_status status;

    queueLock.lock();

    if (this->queue.avisos.empty() == true)
    {
        status = this->queue.condition.wait_for(queueLock, duration);
    }
    else
    {
        status = std::cv_status::no_timeout;
    }

    queueLock.unlock();

    return status;
}

bool
Dispatcher::Queue::pendingAvisos()
{
    std::unique_lock<std::mutex> queueLock { this->queue.lock, std::defer_lock };

    queueLock.lock();

    bool answer = (this->queue.avisos.empty() == false);

    queueLock.unlock();

    return answer;
}

void
Dispatcher::Queue::enqueueAviso(Dispatcher::Aviso* aviso)
{
    std::unique_lock<std::mutex> queueLock { this->queue.lock, std::defer_lock };

    queueLock.lock();

    {
        this->lastAvisoId++;

        aviso->avisoId = this->lastAvisoId;
    }

    this->queue.avisos.push(aviso);

    ReportInfo("[Dispatcher] Enqueued aviso #%u",
            aviso->avisoId);

    this->queue.condition.notify_one();

    queueLock.unlock();
}

void
Dispatcher::Queue::dequeueAviso(const unsigned int avisoId)
{
    std::unique_lock<std::mutex> queueLock { this->queue.lock };

    Dispatcher::Aviso* aviso = this->queue.avisos.front();

    if (aviso->avisoId != avisoId)
    {
        ReportWarning("[Dispatcher] Unexpected aviso in front of the queue: " \
                "expected #%u, found #%u",
                avisoId,
                aviso->avisoId);
    }

    this->queue.avisos.pop();
}

Dispatcher::Aviso*
Dispatcher::Queue::fetchFirstAviso()
{
    std::unique_lock<std::mutex> queueLock { this->queue.lock, std::defer_lock };

    queueLock.lock();

    if (this->queue.avisos.empty() == true)
    {
        queueLock.unlock();

        throw Dispatcher::NothingInTheQueue();
    }

    Dispatcher::Aviso* aviso = this->queue.avisos.front();

    if (this->queue.avisos.empty() == true)
    {
        ReportInfo("[Dispatcher] Fetched aviso #%u from the queue",
                aviso->avisoId);
    }
    else
    {
        ReportInfo("[Dispatcher] Fetched aviso #%u from the queue, %u left",
                aviso->avisoId,
                (unsigned int) this->queue.avisos.size());
    }

    queueLock.unlock();

    return aviso;
}
