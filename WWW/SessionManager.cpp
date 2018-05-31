// System definition files.
//
#include <cstdbool>
#include <stdexcept>
#include <string>
#include <vector>

// Common definition files.
//
#include "Toolkit/Report.h"
#include "Toolkit/Times.hpp"

// Local definition files.
//
#include "Quasar/Configuration.hpp"
#include "Quasar/WWW/SessionManager.hpp"

static WWW::SessionManager* instance = NULL;

WWW::SessionManager&
WWW::SessionManager::InitInstance()
{
    if (instance != NULL)
        throw std::runtime_error("[SessionManager] Already initialized");

    instance = new WWW::SessionManager();

    return *instance;
}

WWW::SessionManager&
WWW::SessionManager::SharedInstance()
{
    if (instance == NULL)
        throw std::runtime_error("[SessionManager] Not initialized");

    return *instance;
}

WWW::SessionManager::SessionManager()
{ }

bool
WWW::SessionManager::login(
    const std::string& guestAddress,
    const std::string& clearTextPassword)
{
    Quasar::Configuration& configuration = Quasar::Configuration::SharedInstance();

    if (clearTextPassword != configuration.http.password)
    {
        ReportWarning("[WWW] Forbid access for %s with password '%s'",
                guestAddress.c_str(),
                clearTextPassword.c_str());

        return false;
    }
    else
    {
        ReportNotice("[WWW] Permitted login for %s", guestAddress.c_str());

        WWW::Session* session = new WWW::Session(guestAddress);

        this->sessions.push_back(session);

        return true;
    }
}

bool
WWW::SessionManager::permitted(const std::string& guestAddress)
{
    Quasar::Configuration& configuration = Quasar::Configuration::SharedInstance();

    for (std::vector<WWW::Session*>::size_type sessionIndex = 0;
         sessionIndex < this->sessions.size();
         sessionIndex++)
    {
        WWW::Session* session = this->sessions[sessionIndex];

        if (guestAddress == session->guestAddress)
        {
            if (session->lifeTime() > configuration.http.keepAliveSession)
            {
                ReportNotice("[WWW] Session for %s has expired", guestAddress.c_str());

                this->sessions.erase(this->sessions.begin() + sessionIndex);

                return false;
            }
            else
            {
                ReportNotice("[WWW] Found session for %s", guestAddress.c_str());

                session->touch();

                return true;
            }
        }
    }

    return false;
}

WWW::Session::Session(const std::string& guestAddress) :
guestAddress(guestAddress)
{
    this->lastTouch = Toolkit::TimestampSeconds();
}

void
WWW::Session::touch()
{
    this->lastTouch = Toolkit::TimestampSeconds();
}

unsigned long
WWW::Session::lifeTime()
{
    return Toolkit::TimestampSeconds() - this->lastTouch;
}
