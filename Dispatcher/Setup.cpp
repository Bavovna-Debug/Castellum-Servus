// System definition files.
//
#include <string>
#include <rapidjson/document.h>

// Common definition files.
//
#include "GPIO/Relay.hpp"
#include "GPIO/Therma.hpp"

// Local definition files.
//
#include "Servus/Dispatcher/Setup.hpp"
#include "Servus/Peripherique/ThermiqueSensor.hpp"
#include "Servus/Peripherique/ThermiqueStation.hpp"

using namespace rapidjson;

void
Dispatcher::ProcessConfigurationJSON(const std::string& json)
{
    Document document;
    document.Parse(json.c_str());

    Value& jsonServus = document["Servus"];

    {
        GPIO::RelayStation& relayStation = GPIO::RelayStation::SharedInstance();

        Value& jsonRelayList = jsonServus["Relays"];

        assert(jsonRelayList.IsArray());

        for (SizeType relayIndex = 0;
             relayIndex < jsonRelayList.Size();
             relayIndex++)
        {
            Value& jsonRelay = jsonRelayList[relayIndex];

            const unsigned int pinNumber = jsonRelay["Pin"].GetInt();
            const std::string relayName = jsonRelay["Name"].GetString();
            relayStation += new GPIO::Relay(pinNumber, relayName);
        }
    }

    {
        Peripherique::ThermiqueStation& thermiqueStation = Peripherique::ThermiqueStation::SharedInstance();

        Value& jsonThermaList = jsonServus["Therma"];

        assert(jsonThermaList.IsArray());

        for (SizeType thermaIndex = 0;
             thermaIndex < jsonThermaList.Size();
             thermaIndex++)
        {
            Value& jsonTherma = jsonThermaList[thermaIndex];

            const std::string deviceId      = jsonTherma["DeviceId"].GetString();
            const std::string thermaToken   = jsonTherma["Token"].GetString();
            const std::string thermaName    = jsonTherma["Name"].GetString();
            const float edge                = jsonTherma["Edge"].GetDouble();

            thermiqueStation += new Peripherique::ThermiqueSensor(
                    deviceId,
                    thermaToken,
                    thermaName,
                    edge);
        }

        thermiqueStation.startService();
    }
}
