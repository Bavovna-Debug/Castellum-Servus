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

Dispatcher::Aviso::Aviso(const std::string& avisoType) :
avisoType(avisoType)
{
    this->avisoId = 0;

    this->timestamp = new Toolkit::Timestamp();
}

Dispatcher::Aviso::Aviso(
    const std::string&  avisoType,
    const std::string&  stamp) :
avisoType(avisoType)
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
Dispatcher::Aviso::prepare(RTSP::Datagram& datagram) const
{
    datagram["Aviso-Id"]        = this->avisoId;
    datagram["Timestamp"]       = this->timestamp->floatString();
}

Dispatcher::FabulaAviso::FabulaAviso(
    const std::string&      stamp,
    const std::string&      fabulatorName,
    const unsigned short    severityLevel,
    const bool              notificationFlag,
    const std::string&      message) :
Inherited("FABULA", stamp),
fabulatorName(fabulatorName),
severityLevel(severityLevel),
notificationFlag(notificationFlag),
message(message)
{ }

void
Dispatcher::FabulaAviso::prepare(RTSP::Datagram& datagram) const
{
    Inherited::prepare(datagram);

    datagram["Severity"]        = this->severityLevel;
    datagram["Notification"]    = this->notificationFlag;
    datagram["Originator"]      = this->fabulatorName;
    datagram["Message"]         = this->message;
}

Dispatcher::TemperatureAviso::TemperatureAviso(
    const std::string&  sensorToken,
    const float         temperature) :
Inherited("TEMPERATURE"),
sensorToken(sensorToken),
temperature(temperature)
{ }

void
Dispatcher::TemperatureAviso::prepare(RTSP::Datagram& datagram) const
{
    Inherited::prepare(datagram);

    datagram["Sensor-Token"]    = this->sensorToken;
    datagram["Temperature"]     = this->temperature;
}
