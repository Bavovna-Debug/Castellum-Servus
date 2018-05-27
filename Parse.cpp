// System definition files.
//
#include <libconfig.h++>
#include <stdexcept>
#include <string>

// Common definition files.
//
#include "Toolkit/Report.h"

// Local definition files.
//
#include "Servus/Configuration.hpp"
#include "Servus/Dispatcher/Communicator.hpp"
#include "Servus/Fabulatorium/Fabulator.hpp"
#include "Servus/Fabulatorium/Listener.hpp"

using namespace libconfig;

void
Servus::Configuration::load()
{
    Config& config = this->open();

    try
    {
        Setting& rootSetting = config.getRoot();

        Setting& servusSetting = rootSetting["Servus"];

        unsigned int modbusPortNumber = servusSetting["MODBUS_PortNumberIPv4"];
        unsigned int httpPortNumber = servusSetting["HTTP_PortNumberIPv4"];

        this->modbusPortNumber = modbusPortNumber;
        this->httpPortNumber = httpPortNumber;

        {
            Dispatcher::Communicator& communicator = Dispatcher::Communicator::SharedInstance();

            Setting& primusSetting = servusSetting["Primus"];

            const std::string   address          = primusSetting["Address"];
            const unsigned int  portNumber       = primusSetting["PortNumber"];
            const std::string   authenticator  = primusSetting["Authenticator"];

            communicator.setConnectionParameters(
                    address,
                    portNumber,
                    authenticator);

            try
            {
                Setting& connectionSetting = primusSetting["Connection"];

                communicator.setConnectionIntervals(
                        connectionSetting["SleepIfRejectedByPrimus"],
                        connectionSetting["ReconnectInterval"],
                        connectionSetting["WaitForResponse"],
                        connectionSetting["WaitForDatagramCompletion"]);
            }
            catch (SettingNotFoundException& exception)
            { }
        }

        {
            Setting& fabulatoriumSetting = servusSetting["Fabulatorium"];

            {
                Setting& listenersSetting = fabulatoriumSetting["Listeners"];

                for (int listenerIndex = 0;
                     listenerIndex < listenersSetting.getLength();
                     listenerIndex++)
                {
                    Setting& listenerSetting = listenersSetting[listenerIndex];

                    const std::string listenerName  = listenerSetting["ListenerName"];
                    const std::string interface     = listenerSetting["Interface"];
                    const unsigned int portNumber   = listenerSetting["PortNumberIPv4"];

                    Fabulatorium::Listener *listener = new Fabulatorium::Listener(
                            listenerName,
                            interface,
                            portNumber);

                    try
                    {
                        Setting& connectionSetting = listenerSetting["Connection"];

                        listener->setConnectionIntervals(
                                connectionSetting["WaitForFirstTransmission"],
                                connectionSetting["WaitForTransmissionCompletion"]);
                    }
                    catch (SettingNotFoundException& exception)
                    { }
                }
            }

            {
                Setting& fabulatorsSetting = fabulatoriumSetting["Fabulators"];

                for (int fabulatorIndex = 0;
                     fabulatorIndex < fabulatoriumSetting.getLength();
                     fabulatorIndex++)
                {
                    Setting& fabulatorSetting = fabulatorsSetting[fabulatorIndex];

                    const std::string fabulatorName          = fabulatorSetting["FabulatorName"];
                    const unsigned int defaultSeverityLevel  = fabulatorSetting["DefaultSeverity"];
                    const bool defaultNotificationFlag       = fabulatorSetting["DefaultNotificationFlag"];

                    new Fabulatorium::Fabulator(
                            fabulatorName,
                            defaultSeverityLevel,
                            defaultNotificationFlag);
                }
            }
        }
    }
    catch (SettingNotFoundException& exception)
    {
        ReportError("[Workspace] Using default for \"%s\"",
                exception.getPath());
    }
    catch (SettingTypeException& exception)
    {
        ReportError("[Workspace] Exception on configuration: %s (%s)",
                exception.what(),
                exception.getPath());
    }
    catch (std::exception& exception)
    {
        ReportError("[Workspace] Exception on configuration: %s",
                exception.what());
    }

    this->close(config);
}
