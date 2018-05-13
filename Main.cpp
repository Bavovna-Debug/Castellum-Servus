// System definition files.
//
#include <stdexcept>

// Common definition files.
//
#include "Toolkit/Report.h"

// Local definition files.
//
#include "Servus/Workspace.hpp"

int
main(void)
{
    try
    {
        Workspace::Servus &workspace = Workspace::Servus::InitInstance();

        workspace.activateSysLog(Workspace::Daemon, Workspace::Debug, "servus");

        workspace.daemonize("/opt/servus/");

        workspace.run();
    }
    catch (std::exception &exception)
    {
        ReportWarning("[Main] Exception: %s", exception.what());
    }

    return EXIT_SUCCESS;
}
