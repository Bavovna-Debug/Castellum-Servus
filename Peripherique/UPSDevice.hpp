#pragma once

// System definition files.
//
#include <string>

namespace Peripherique
{
    /**
     * UPS device.
     */
    class UPSDevice
    {
    public:
        std::string token;
        std::string title;

    public:
        UPSDevice(
            const std::string&  token,
            const std::string&  title);
    };
};
