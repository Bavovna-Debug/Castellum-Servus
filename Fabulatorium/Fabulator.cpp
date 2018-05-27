// System definition files.
//
#include <unistd.h>
#include <cstdbool>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <thread>

// Common definition files.
//
#include "Toolkit/Report.h"

// Local definition files.
//
#include "Servus/Fabulatorium/Fabulator.hpp"

Fabulatorium::Fabulator::Fabulator(
    const std::string&      fabulatorName,
    const unsigned short    defaultSeverityLevel,
    const bool              defaultNotificationFlag) :
fabulatorName(fabulatorName),
defaultSeverityLevel(defaultSeverityLevel),
defaultNotificationFlag(defaultNotificationFlag)
{
    ReportInfo("[Fabulator] Defined fabulator \"%s\" with defaults: severity %u %s notification",
            fabulatorName.c_str(),
            defaultSeverityLevel,
            (defaultNotificationFlag == true) ? "with" : "without");
}
