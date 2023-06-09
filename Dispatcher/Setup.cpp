// System definition files.
//
#include <cstdbool>
#include <string>
#include <rapidjson/document.h>

// Common definition files.
//
#include "Raspberry/DS1820.hpp"
#include "Raspberry/Relay.hpp"
#include "Toolkit/Report.h"

// Local definition files.
//
#include "Servus/Dispatcher/Setup.hpp"
#include "Servus/Peripherique/HumiditySensor.hpp"
#include "Servus/Peripherique/HumidityStation.hpp"
#include "Servus/Peripherique/ThermiqueSensor.hpp"
#include "Servus/Peripherique/ThermiqueStation.hpp"
#include "Servus/Peripherique/UPSDevice.hpp"
#include "Servus/Peripherique/UPSDevicePool.hpp"
#include "Servus/Kernel.hpp"

using namespace rapidjson;

void
Dispatcher::ProcessConfigurationJSON(const std::string& json)
{
    Document document;
    document.Parse(json.c_str());

    Value& jsonServus = document["Servus"];

    // Get servus name.
    //
    {
        const std::string servusTitle = jsonServus["Title"].GetString();

        ReportInfo("[Dispatcher] Received configuration for servus '%s'",
                servusTitle.c_str());

        Workspace::Kernel& kernel = Workspace::Kernel::SharedInstance();

        kernel.systemName = servusTitle;
    }

    // Process UPS devices.
    //
    {
        Value& jsonUPSList = jsonServus["UPS"];

        if (jsonUPSList.IsNull() == true)
        {
            ReportInfo("[Dispatcher] Nothing defined for 'UPS'");
        }
        else
        {
            ReportInfo("[Dispatcher] Parsing 'UPS'");

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
    }

    // Process hosts.
    //
    {
        Value& jsonHostList = jsonServus["Hosts"];

        if (jsonHostList.IsNull() == true)
        {
            ReportInfo("[Dispatcher] Nothing defined for 'Hosts'");
        }
        else
        {
            ReportInfo("[Dispatcher] Parsing 'Hosts'");

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
    }

    // Process relays.
    //
    {
        Raspberry::RelayStation& relayStation = Raspberry::RelayStation::SharedInstance();

        Value& jsonRelayList = jsonServus["Relays"];

        if (jsonRelayList.IsNull() == true)
        {
            ReportInfo("[Dispatcher] Nothing defined for 'Relays'");
        }
        else
        {
            ReportInfo("[Dispatcher] Parsing 'Relays'");

            assert(jsonRelayList.IsArray());

            for (SizeType relayIndex = 0;
                 relayIndex < jsonRelayList.Size();
                 relayIndex++)
            {
                Value& jsonRelay = jsonRelayList[relayIndex];

                const std::string token         = jsonRelay["Token"].GetString();
                const unsigned int pinNumber    = jsonRelay["PinNumber"].GetInt();
                const bool defaultState         = jsonRelay["DefaultState"].GetBool();
                const std::string title         = jsonRelay["Title"].GetString();

                ReportInfo("[Dispatcher] Setup relay '%s' on GPIO pin %u",
                        title.c_str(),
                        pinNumber);

                Raspberry::Relay* relay = new Raspberry::Relay(pinNumber, title);

                relayStation += relay;

                if (defaultState == false)
                {
                    relay->switchOff();
                }
                else
                {
                    relay->switchOn();
                }
            }
        }
    }

    // Process humidity sensors.
    //
    {
        Peripherique::HumidityStation& humidityStation = Peripherique::HumidityStation::SharedInstance();

        Value& jsonListOfDHT = jsonServus["DHT22"];

        if (jsonListOfDHT.IsNull() == true)
        {
            ReportInfo("[Dispatcher] Nothing defined for 'DHT11/DHT22'");
        }
        else
        {
            ReportInfo("[Dispatcher] Parsing 'DHT11/DHT22'");

            assert(jsonListOfDHT.IsArray());

            for (SizeType sensorIndex = 0;
                 sensorIndex < jsonListOfDHT.Size();
                 sensorIndex++)
            {
                Value& jsonSensor = jsonListOfDHT[sensorIndex];

                const std::string token         = jsonSensor["Token"].GetString();
                const unsigned int pinNumber    = jsonSensor["PinNumber"].GetInt();
                const float humidityEdge        = jsonSensor["HumidityEdge"].GetDouble();
                const float temperatureEdge     = jsonSensor["TemperatureEdge"].GetDouble();
                const std::string title         = jsonSensor["Title"].GetString();

                ReportInfo("[Dispatcher] Setup DHT11/DHT22 sensor '%s' on GPIO pin #%u and edges %0.1f/%0.1f",
                        title.c_str(),
                        pinNumber,
                        humidityEdge,
                        temperatureEdge);

                humidityStation += new Peripherique::HumiditySensor(
                        token,
                        pinNumber,
                        humidityEdge,
                        temperatureEdge,
                        title);
            }

            humidityStation.startService();
        }
    }

    // Process temperature sensors.
    //
    {
        Peripherique::ThermiqueStation& thermiqueStation = Peripherique::ThermiqueStation::SharedInstance();

        Value& jsonListOfDS = jsonServus["DS1820"];

        if (jsonListOfDS.IsNull() == true)
        {
            ReportInfo("[Dispatcher] Nothing defined for 'DS1820'");
        }
        else
        {
            ReportInfo("[Dispatcher] Parsing 'DS1820'");

            assert(jsonListOfDS.IsArray());

            for (SizeType sensorIndex = 0;
                 sensorIndex < jsonListOfDS.Size();
                 sensorIndex++)
            {
                Value& jsonSensor = jsonListOfDS[sensorIndex];

                const std::string token         = jsonSensor["Token"].GetString();
                const std::string deviceId      = jsonSensor["DeviceId"].GetString();
                const float temperatureEdge     = jsonSensor["TemperatureEdge"].GetDouble();
                const std::string title         = jsonSensor["Title"].GetString();

                ReportInfo("[Dispatcher] Setup DS1820 sensor '%s' with device id %s and edge %0.1f",
                        title.c_str(),
                        deviceId.c_str(),
                        temperatureEdge);

                thermiqueStation += new Peripherique::ThermiqueSensor(
                        token,
                        deviceId,
                        temperatureEdge,
                        title);
            }

            thermiqueStation.startService();
        }
    }
}
