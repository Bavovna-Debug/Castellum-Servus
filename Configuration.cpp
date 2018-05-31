// System definition files.
//
#include <libconfig.h++>
#include <stdexcept>

// Local definition files.
//
#include "Servus/Configuration.hpp"

static Servus::Configuration* instance = NULL;

Servus::Configuration&
Servus::Configuration::InitInstance(const std::string& configurationFilePath)
{
    if (instance != NULL)
        throw std::runtime_error("Configuration already initialized");

    instance = new Servus::Configuration(configurationFilePath);

    return *instance;
}

Servus::Configuration&
Servus::Configuration::SharedInstance()
{
    if (instance == NULL)
        throw std::runtime_error("Configuration not initialized");

    return *instance;
}

Servus::Configuration::Configuration(const std::string& configurationFilePath) :
configurationFilePath(configurationFilePath)
{
    this->modbus.portNumber     = Servus::DefaultMODBUSPortNumberIPv4;
    this->http.portNumber       = Servus::DefaultHTTPPortNumberIPv4;
    this->http.keepAliveSession = Servus::DefaultHTTPKeepAliveSession;
}

libconfig::Config&
Servus::Configuration::open()
{
    libconfig::Config* config = new libconfig::Config;
    config->readFile(this->configurationFilePath.c_str());
    config->setTabWidth(4);
    return *config;
}

void
Servus::Configuration::close(libconfig::Config& config)
{
    config.~Config();
}

void
Servus::Configuration::flush(libconfig::Config& config)
{
    config.writeFile(this->configurationFilePath.c_str());
}
