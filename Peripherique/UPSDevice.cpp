// System definition files.
//
#include <string>

// Local definition files.
//
#include "Servus/Peripherique/UPSDevice.hpp"

Peripherique::UPSDevice::UPSDevice(
    const std::string&  token,
    const std::string&  title) :
token(token),
title(title)
{ }
