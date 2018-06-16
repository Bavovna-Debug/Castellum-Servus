// System definition files.
//
#include <string>
#include <rapidjson/document.h>

// Common definition files.
//
#include "GPIO/Relay.hpp"
#include "GPIO/Therma.hpp"
#include "Toolkit/Report.h"

// Local definition files.
//
#include "Servus/Dispatcher/Setup.hpp"
#include "Servus/Peripherique/ThermiqueSensor.hpp"
#include "Servus/Peripherique/ThermiqueStation.hpp"
#include "Servus/Peripherique/UPSDevice.hpp"
#include "Servus/Peripherique/UPSDevicePool.hpp"

using namespace rapidjson;

void
Dispatcher::ProcessConfigurationJSON(const std::string& json)
{
    Document document;
    document.Parse(json.c_str());

    Value& jsonServus = document["Servus"];

    const std::string servusTitle = jsonServus["Title"].GetString();

    ReportInfo("[Dispatcher] Received configuration for servus '%s'",
            servusTitle.c_str());

    // Process UPS devices.
    //
    {
        Value& jsonUPSList = jsonServus["UPS"];

        assert(jsonUPSList.IsArray());

        for (SizeType upsIndex = 0;
             upsIndex < jsonUPSList.Size();
             upsIndex++)
        {
            Value& jsonUPS = jsonUPSList[upsIndex];

            const std::string token = jsonUPS["Token"].GetString();
            const std::string title = jsonUPS["Title"].GetString();

            ReportInfo("[Dispatcher] Setup UPS '%s'",
                    title.c_str());

            Peripherique::UPSDevice* device = new Peripherique::UPSDevice(token, title);

            Peripherique::UPSDevicePool::SharedInstance().defineUPS(device);
        }
    }

    // Process hosts.
    //
    {
        Value& jsonHostList = jsonServus["Hosts"];

        assert(jsonHostList.IsArray());

        for (SizeType hostIndex = 0;
             hostIndex < jsonHostList.Size();
             hostIndex++)
        {
            Value& jsonHost = jsonHostList[hostIndex];

            const std::string token         = jsonHost["Token"].GetString();
            const std::string hostName      = jsonHost["HostName"].GetString();
            const unsigned int interval     = jsonHost["Interval"].GetInt();
            const unsigned int retries      = jsonHost["Retries"].GetInt();

            ReportInfo("[Dispatcher] Setup host '%s' with interval %u seconds and %u retries",
                    hostName.c_str(),
                    interval,
                    retries);
        }
    }

    // Process relays.
    //
    {
        GPIO::RelayStation& relayStation = GPIO::RelayStation::SharedInstance();

        Value& jsonRelayList = jsonServus["Relays"];

        assert(jsonRelayList.IsArray());

        for (SizeType relayIndex = 0;
             relayIndex < jsonRelayList.Size();
             relayIndex++)
        {
            Value& jsonRelay = jsonRelayList[relayIndex];

            const std::string token         = jsonRelay["Token"].GetString();
            const unsigned int pinNumber    = jsonRelay["Pin"].GetInt();
            const std::string title         = jsonRelay["Title"].GetString();

            ReportInfo("[Dispatcher] Setup relay '%s' on GPIO pin %u",
                    title.c_str(),
                    pinNumber);

            relayStation += new GPIO::Relay(pinNumber, title);
        }
    }

    // Process temperature sensors.
    //
    {
        Peripherique::ThermiqueStation& thermiqueStation = Peripherique::ThermiqueStation::SharedInstance();

        Value& jsonThermaList = jsonServus["Therma"];

        assert(jsonThermaList.IsArray());

        for (SizeType thermaIndex = 0;
             thermaIndex < jsonThermaList.Size();
             thermaIndex++)
        {
            Value& jsonTherma = jsonThermaList[thermaIndex];

            const std::string thermaToken   = jsonTherma["Token"].GetString();
            const std::string deviceId      = jsonTherma["DeviceId"].GetString();
            const float edge                = jsonTherma["Edge"].GetDouble();
            const std::string title         = jsonTherma["Title"].GetString();

            ReportInfo("[Dispatcher] Setup thermique sensor '%s' with device id %s and edge %0.1f",
                    title.c_str(),
                    deviceId.c_str(),
                    edge);

            thermiqueStation += new Peripherique::ThermiqueSensor(thermaToken, deviceId, edge, title);
        }

        thermiqueStation.startService();
    }
}
