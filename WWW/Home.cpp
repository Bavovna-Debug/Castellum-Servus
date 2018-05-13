// System definition files.
//
#include <cstdbool>
#include <stdexcept>

// Common definition files.
//
#include "GPIO/Relay.hpp"
#include "HTTP/Connection.hpp"
#include "HTTP/HTML.hpp"
#include "HTTP/HTTP.hpp"
#include "HTTP/Site.hpp"
#include "Toolkit/Report.h"

// Local definition files.
//
#include "Servus/WWW/Home.hpp"

using namespace HTML;

#define CHIMAERA_SCRIPT_UPDATE_SYSTEM_INFORMATION \
"function update() " \
"{ " \
    "$(\"#info_sheet\").html('Loading...'); " \
    "$.ajax( " \
    "{ " \
        "type: 'GET', " \
        "url: 'system_information_sheet', " \
        "timeout: 1000, " \
        "success: function(data) " \
        "{ " \
            "$(\"#info_sheet\").html(data); " \
            "window.setTimeout(update, 1000); " \
        "}, " \
        "error: function (XMLHttpRequest, textStatus, errorThrown) " \
        "{ " \
            "$(\"#info_sheet\").html('Lost TCP connection to ECU'); " \
            "window.setTimeout(update, 5000); " \
        "} " \
    "}); " \
"} " \
"$(document).ready(function() " \
"{ " \
    "update(); " \
"});"

/**
 * @brief   Generate HTML main page.
 *
 * @param[in]   connection  Pointer to HTTP connection.
 */
void
WWW::Site::generateDocument(HTTP::Connection &connection)
{
    HTML::Instance instance(connection);

    this->processRelays(connection, instance);

    { // HTML.Document
        HTML::Document document(instance);

        { // HTML.Head
            HTML::Head head(instance);

            { // HTML::Title
                HTML::Title title(instance);

                title.plain("Servus");
            } // HTML::Title

            head.meta("Content-Type", "text/html; charset=utf-8");

            head.styleSheet("layout.css", "text/css", "screen,projection");
            head.styleSheet("tabs.css", "text/css");
            head.styleSheet("messages.css", "text/css");
            head.styleSheet("workspace.css", "text/css");
            head.styleSheet("form.css", "text/css");

            {
                char ajaxURL[200];

                snprintf(ajaxURL, sizeof(ajaxURL) - sizeof(char),
                        "%s?%s=%s",
                        WWW::Download,
                        WWW::DownloadSubject,
                        WWW::DownloadSubjectAjax);

                HTML::Script script(instance, "text/javascript", ajaxURL);
            }
        } // HTML.Head

        { // HTML.Body
            HTML::Body body(instance);

            // Static upper part of document.
            //
            { // HTML.Division
                HTML::Division division(instance, "content-north");

                // Contents should be located inside of inliner, which defines fixed location on a page.
                //
                { // HTML.Division
                    HTML::Division division(instance, NULL, "inliner");

                    this->generateNorth(connection, instance);
                } // HTML.Division
            } // HTML.Division

            // Static lower part of document.
            //
            { // HTML.Division
                HTML::Division division(instance, "content-south");

                // Contents should be located inside of inliner, which defines fixed location on a page.
                //
                { // HTML.Division
                    HTML::Division division(instance, NULL, "inliner");

                    this->generateSouth(connection, instance);
                } // HTML.Division

{ // HTML.Division
    HTML::Division division(instance, "notice_div", "notice_div");
} // HTML.Division
{ // HTML.Division
    HTML::Division division(instance, "some_div", "some_div");
} // HTML.Division
            } // HTML.Division

            {
                HTML::Script script(instance, "text/javascript", NULL);

                script.plain(
                        "function update() "
                        "{ "
                            "$(\"#notice_div\").html('Loading..'); "
                            "$.ajax( "
                            "{ "
                                "type: 'GET', "
                                "url: 'upload_status', "
                                "timeout: 1000, "
                                "success: function(data) "
                                "{ "
                                    "$(\"#some_div\").html(data); "
                                    "window.setTimeout(update, 1000); "
                                    "}, "
                                "error: function (XMLHttpRequest, textStatus, errorThrown) "
                                "{ "
                                    "$(\"#notice_div\").html('Lost TCP connection to ECU'); "
                                    "window.setTimeout(update, 5000); "
                                "} "
                            "}); "
                        "} "
                        "$(document).ready(function() "
                        "{ "
                            "update(); "
                        "});");
            }
        } // HTML.Body
    } // HTML.Document
}

void
WWW::Site::processRelays(HTTP::Connection &connection, HTML::Instance &instance)
{
    const char  *relayIndex;
    const char  *relayState;

    relayIndex = connection.argumentByName(WWW::SwitchRelay);
    if (relayIndex != NULL)
    {
        GPIO::RelayStation &relayStation = GPIO::RelayStation::SharedInstance();

        try
        {
            GPIO::Relay *relay = relayStation[strtol(relayIndex, NULL, 10)];

            relayState = connection.argumentByName(WWW::RelayState);
            if (relayState == NULL)
            {
                relay->switchOver();
            }
            else
            {
                if (strcmp(relayState, WWW::RelayStateDown) == 0)
                {
                    relay->switchOff();
                }
                else if (strcmp(relayState, WWW::RelayStateUp) == 0)
                {
                    relay->switchOn();
                }
            }
        }
        catch (std::out_of_range)
        {
            instance.errorMessage("Relais #%s ist nicht bekannt", relayIndex);
        }
        catch (...)
        {
            instance.errorMessage("Relais #%s kann nicht geschaltet werden", relayIndex);
        }
    }
}

/**
 * @brief   Generate the north (upper) part of main page.
 *
 * @param[in]   connection  Pointer to HTTP connection.
 * @param[in]   instance    Pointer to HTML instance.
 */
void
WWW::Site::generateNorth(HTTP::Connection &connection, HTML::Instance &instance)
{
#if 0

    // This is the right place for kinds of messages.
    //
    {
        if (HTTP_ArgumentPairExists(connection, CHIMAERA_ACTION, CHIMAERA_ACTION_ORBIS_SAVE) == true)
        {
            if (WWW::FormSubmitted(connection) == true)
            {
                WWW_SaveOrbisOption(connection);
            }
        }

        if (WWW_OrbisModified() == true)
        {
            HTML::WarningMessage(connection,
                    "There are options with modified values. Save options in MTA flash before switching off the device. Otherwise, modified values will be lost.");
        }

        if (connection.upload.requested == true)
        {
            if (connection.upload.uploaded == false)
            {
                HTML::ErrorMessage(connection, "Cannot upload file");
            }
            else
            {
                HTML::SuccessMessage(connection, "File has been successfully uploaded");

                switch (connection.upload.fileType)
                {
                    case HTTP_UPLOAD_FDASY_MASTER:
                    case HTTP_UPLOAD_FDASY_SLAVE:
                    case HTTP_UPLOAD_MDASY_MASTER:
                    case HTTP_UPLOAD_MDASY_SLAVE:
                    {
                        HTML::NoticeMessage(connection, "Uploaded file has been skipped");

                        break;
                    }

                    case HTTP_UPLOAD_FPGA:
                    {
                        if (connection.upload.flashed == false)
                        {
                            HTML::ErrorMessage(connection, "Error has occurred trying to copy uploaded file to FPGA");
                        }
                        else
                        {
                            HTML::SuccessMessage(connection, "Uploaded file has been copied to FPGA");
                        }

                        break;
                    }
                }
            }

            connection.upload.requested = false;
        }
    }

#endif

#if 0

    // Process forms.
    //
    {
        if (HTTP_ArgumentPairExists(connection, CHIMAERA_ACTION, CHIMAERA_ACTION_ORBIS_EDIT) == true)
        {
            WWW_GenerateOrbisEditForm(connection);
        }
    }

#endif

    { // HTML.Division
        HTML::Division division(instance, NULL, "tabs");

        // Tabs line.
        //
        { // HTML.UnorderedList
            HTML::UnorderedList unorderedList(instance, NULL, "tabs_list");

            // 'Start' tab.
            //
            { // HTML.ListItem
                HTML::ListItem listItem(instance,
                        NULL,
                        ((connection.pageIsEqualTo(WWW::PageNone) == true) ||
                        (connection.pageIsEqualTo(WWW::PageSystemInformation) == true))
                                ? "tabs_item active"
                                : "tabs_item");

                { // HTML.URL
                    HTML::URL url(instance, WWW::PageSystemInformation);

                    { // HTML.Span
                        HTML::Span span(instance, NULL, "title");

                        span.plain("Start");
                    } // HTML.Span

                    { // HTML.Span
                        HTML::Span span(instance, NULL, "subtitle");

                        span.plain("Systeminformation");
                    } // HTML.Span
                } // HTML.URL
            } // HTML.ListItem

            // 'Therma' tab.
            //
            { // HTML.ListItem
                HTML::ListItem listItem(instance,
                        NULL,
                        (connection.pageIsEqualTo(WWW::PageTherma) == true)
                                ? "tabs_item active"
                                : "tabs_item");

                { // HTML.URL
                    HTML::URL url(instance, WWW::PageTherma);

                    { // HTML.Span
                        HTML::Span span(instance, NULL, "title");

                        span.plain("Therma");
                    } // HTML.Span

                    { // HTML.Span
                        HTML::Span span(instance, NULL, "subtitle");

                        span.plain("Temperaturen");
                    } // HTML.Span
                } // HTML.URL
            } // HTML.ListItem

            // 'Relay' tab.
            //
            { // HTML.ListItem
                HTML::ListItem listItem(instance,
                        NULL,
                        (connection.pageIsEqualTo(WWW::PageRelay) == true)
                                ? "tabs_item active"
                                : "tabs_item");

                { // HTML.URL
                    HTML::URL url(instance, WWW::PageRelay);

                    { // HTML.Span
                        HTML::Span span(instance, NULL, "title");

                        span.plain("Relais");
                    } // HTML.Span

                    { // HTML.Span
                        HTML::Span span(instance, NULL, "subtitle");

                        span.plain("Relais-Schalttaffel");
                    } // HTML.Span
                } // HTML.URL
            } // HTML.ListItem
        } // HTML.UnorderedList
    } // HTML.Division
}

/**
 * @brief   Generate the south (lower) part of main page.
 *
 * @param[in]   connection  Pointer to HTTP connection.
 * @param[in]   instance    Pointer to HTML instance.
 */
void
WWW::Site::generateSouth(HTTP::Connection &connection, HTML::Instance &instance)
{
    if (connection.pageIsEqualTo(WWW::PageNone) == true)
    {
        this->generateSystemInformation(connection, instance);
    }
    else if (connection.pageIsEqualTo(WWW::PageSystemInformation) == true)
    {
        this->generateSystemInformation(connection, instance);
    }
    else if (connection.pageIsEqualTo(WWW::PageTherma) == true)
    {
        this->generateTherma(connection, instance);
    }
    else if (connection.pageIsEqualTo(WWW::PageRelay) == true)
    {
        this->generateRelay(connection, instance);
    }
    else
    {
        this->generateSystemInformation(connection, instance);
    }
}

/**
 * @brief   Check whether some HTML form has been submitted by user.
 *
 * @param[in]   connection  Pointer to HTTP connection.
 *
 * @return  Boolean true if a form has been submitted by user.
 * @return  Boolean false if current HTTP request is regular request without any submit.
 */
bool
WWW::FormSubmitted(HTTP::Connection &connection)
{
    return connection.argumentPairExists(WWW::Button, WWW::ButtonSubmit);
}

/**
 * @brief   Check whether HTML form has been cancelled by user.
 *
 * @param[in]   connection  Pointer to HTTP connection.
 *
 * @return  Boolean true if a form has been cancelled by user.
 * @return  Boolean false if a form has not been cancelled by user.
 */
bool
WWW::FormCancelled(HTTP::Connection &connection)
{
    return connection.argumentPairExists(WWW::Button, WWW::ButtonCancel);
}
