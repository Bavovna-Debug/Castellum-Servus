#pragma once

// System definition files.
//
#include <chrono>
#include <condition_variable>
#include <cstdbool>
#include <mutex>
#include <queue>
#include <stdexcept>

// Local definition files.
//
#include "Servus/Dispatcher/Aviso.hpp"

namespace Dispatcher
{
    class Queue
    {
    private:
        struct
        {
            std::queue<Dispatcher::Aviso*> avisos;
            std::mutex              lock;
            std::condition_variable condition;
        }
        queue;

        unsigned int                lastAvisoId;

    public:
        static Dispatcher::Queue&
        InitInstance();

        static Dispatcher::Queue&
        SharedInstance();

        Queue();

        ~Queue();

        std::cv_status
        wait(const std::chrono::milliseconds);

        bool
        pendingAvisos();

        void
        enqueueAviso(Dispatcher::Aviso*);

        void
        dequeueAviso(const unsigned int avisoId);

        Dispatcher::Aviso*
        fetchFirstAviso();
    };

    class NothingInTheQueue : public std::runtime_error
    {
    public:
        NothingInTheQueue() throw() :
        std::runtime_error("[Queue] Nothing in the queue")
        { }
    };
};
