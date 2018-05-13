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
    const char PageNone[]               = "";
    const char PageSystemInformation[]  = "sysinfo";
    const char PageTherma[]             = "therma";
    const char PageRelay[]              = "relay";

    const char Download[]               = "download";
    const char DownloadSubject[]        = "subject";
    const char DownloadSubjectAjax[]    = "ajax.js";

    const char SwitchRelay[]            = "switch_relay";
    const char RelayState[]             = "relay_state";
    const char RelayStateDown[]         = "down";
    const char RelayStateUp[]           = "up";

    const char Button[]                 = "button";
    const char ButtonSubmit[]           = "submit";
    const char ButtonCancel[]           = "cancel";

    class Site : public HTTP::Site
    {
    public:
        void
        generateDocument(HTTP::Connection &);

    private:
        void
        processRelays(HTTP::Connection &, HTML::Instance &);

        void
        generateNorth(HTTP::Connection &, HTML::Instance &);

        void
        generateSouth(HTTP::Connection &, HTML::Instance &);

        /**
         * @brief   Generate HTML page for the 'System Information' tab.
         *
         * @param   connection      HTTP connection.
         * @param   instance        HTML instance.
         */
        void
        generateSystemInformation(HTTP::Connection &, HTML::Instance &);

        /**
         * @brief   Generate HTML page for the 'Therma' tab.
         *
         * @param   connection      HTTP connection.
         * @param   instance        HTML instance.
         */
        void
        generateTherma(HTTP::Connection &, HTML::Instance &);

        /**
         * @brief   Generate HTML page for the 'Relay' tab.
         *
         * @param   connection      HTTP connection.
         * @param   instance        HTML instance.
         */
        void
        generateRelay(HTTP::Connection &, HTML::Instance &);
    };

    /**
     * @brief   Check whether some HTML form has been submitted by user.
     *
     * @param[in]   connection  Pointer to HTTP connection.
     *
     * @return  Boolean true if a form has been submitted by user.
     * @return  Boolean false if current HTTP request is regular request without any submit.
     */
    bool
    FormSubmitted(HTTP::Connection &connection);

    /**
     * @brief   Check whether HTML form has been cancelled by user.
     *
     * @param[in]   connection  Pointer to HTTP connection.
     *
     * @return  Boolean true if a form has been cancelled by user.
     * @return  Boolean false if a form has not been cancelled by user.
     */
    bool
    FormCancelled(HTTP::Connection &connection);
};
