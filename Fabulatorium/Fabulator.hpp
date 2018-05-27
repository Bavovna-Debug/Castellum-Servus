#pragma once

// System definition files.
//
#include <cstdbool>
#include <stdexcept>
#include <string>
#include <thread>

namespace Fabulatorium
{
    class Fabulator
    {
    private:
        std::string     fabulatorName;
        unsigned short  defaultSeverityLevel;
        bool            defaultNotificationFlag;

    public:
        Fabulator(
            const std::string&      fabulatorName,
            const unsigned short    defaultSeverityLevel,
            const bool              defaultNotificationFlag);
    };
};
