#pragma once

// System definition files.
//
#include <cstdbool>

// Common definition files.
//
#include "HTTP/Connection.hpp"
#include "HTTP/HTML.hpp"
#include "HTTP/Site.hpp"

namespace WWW
{
    static const std::string PageSystemInformation      = "sysinfo";
    static const std::string PageTherma                 = "therma";
    static const std::string PageRelay                  = "relay";

    static const std::string Images                     = "img";
    static const std::string Download                   = "download";
    static const std::string DownloadSubject            = "subject";
    static const std::string DownloadSubjectAjax        = "ajax.js";

    static const std::string SwitchRelay                = "switch_relay";
    static const std::string RelayState                 = "relay_state";
    static const std::string RelayStateDown             = "down";
    static const std::string RelayStateUp               = "up";

    static const std::string Action                     = "action";
    static const std::string ActionLogin                = "login";

    static const std::string Button                     = "button";
    static const std::string ButtonSubmit               = "submit";
    static const std::string ButtonCancel               = "cancel";

    static const std::string Username                   = "username";
    static const std::string Password                   = "password";

    class Site : public HTTP::Site
    {
    public:
        void
        generateDocument(HTTP::Connection&);

    private:
        void
        processRelays(HTTP::Connection&, HTML::Instance&);

        /**
         * @brief   Generate the north (upper) part of main page.
         *
         * @param[in]   connection  Pointer to HTTP connection.
         * @param[in]   instance    Pointer to HTML instance.
         */
        void
        generateNorth(HTTP::Connection&, HTML::Instance&);

        /**
         * @brief   Generate the south (lower) part of main page.
         *
         * @param[in]   connection  Pointer to HTTP connection.
         * @param[in]   instance    Pointer to HTML instance.
         */
        void
        generateSouth(HTTP::Connection&, HTML::Instance&);

        /**
         * @brief   Generate login form.
         *
         * @param[in]   connection  Pointer to HTTP connection.
         * @param[in]   instance    Pointer to HTML instance.
         */
        void
        generateLogin(HTTP::Connection&, HTML::Instance&);

        /**
         * @brief   Generate HTML page for the 'System Information' tab.
         *
         * @param   connection      HTTP connection.
         * @param   instance        HTML instance.
         */
        void
        pageSystemInformation(HTTP::Connection&, HTML::Instance&);

        /**
         * @brief   Generate HTML page for the 'Therma' tab.
         *
         * @param   connection      HTTP connection.
         * @param   instance        HTML instance.
         */
        void
        pageTherma(HTTP::Connection&, HTML::Instance&);

        /**
         * @brief   Generate HTML page for the 'Relay' tab.
         *
         * @param   connection      HTTP connection.
         * @param   instance        HTML instance.
         */
        void
        pageRelay(HTTP::Connection&, HTML::Instance&);

        /**
         * @brief   Check whether some HTML form has been submitted by user.
         *
         * @param[in]   connection  Pointer to HTTP connection.
         *
         * @return  Boolean true if a form has been submitted by user.
         * @return  Boolean false if current HTTP request is regular request without any submit.
         */
        bool
        formSubmitted(HTTP::Connection&);

        /**
         * @brief   Check whether HTML form has been cancelled by user.
         *
         * @param[in]   connection  Pointer to HTTP connection.
         *
         * @return  Boolean true if a form has been cancelled by user.
         * @return  Boolean false if a form has not been cancelled by user.
         */
        bool
        formCancelled(HTTP::Connection&);
    };
};
