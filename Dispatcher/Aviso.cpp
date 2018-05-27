// System definition files.
//
#include <cstdbool>

// Common definition files.
//
#include "RTSP/RTSP.hpp"
#include "Toolkit/Times.hpp"

// Local definition files.
//
#include "Servus/Dispatcher/Aviso.hpp"

Dispatcher::Aviso::Aviso(
    const std::string&      stamp,
    const std::string&      fabulatorName,
    const unsigned short    severityLevel,
    const bool              notificationFlag,
    const std::string&      message) :
fabulatorName(fabulatorName),
severityLevel(severityLevel),
notificationFlag(notificationFlag),
message(message)
{
    this->avisoId = 0;

    if (stamp.length() == 0)
    {
        this->timestamp = new Toolkit::Timestamp();
    }
    else
    {
        this->timestamp = new Toolkit::Timestamp(stamp);
    }
}

Dispatcher::Aviso::~Aviso()
{
    delete this->timestamp;
}

void
Dispatcher::Aviso::prepare(RTSP::Datagram& datagram)
{
    datagram["Aviso-Id"]        = this->avisoId;
    datagram["Timestamp"]       = this->timestamp->floatString();
    datagram["Severity"]        = this->severityLevel;
    datagram["Notification"]    = this->notificationFlag;
    datagram["Originator"]      = this->fabulatorName;
    datagram["Message"]         = this->message;
}
