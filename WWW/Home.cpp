// System definition files.
//
#include <cstdbool>
#include <stdexcept>

// Common definition files.
//
#include "GPIO/Exception.hpp"
#include "GPIO/Relay.hpp"
#include "HTTP/Connection.hpp"
#include "HTTP/HTML.hpp"
#include "HTTP/HTTP.hpp"
#include "HTTP/Site.hpp"
#include "Toolkit/Report.h"

// Local definition files.
//
#include "Servus/WWW/Home.hpp"

#define SCRIPT_UPDATE_SYSTEM_INFORMATION \
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
WWW::Site::generateDocument(HTTP::Connection& connection)
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
                        WWW::Download.c_str(),
                        WWW::DownloadSubject.c_str(),
                        WWW::DownloadSubjectAjax.c_str());

                HTML::Script script(instance, "text/javascript", ajaxURL);
            }
        } // HTML.Head

        { // HTML.Body
            HTML::Body body(instance);

            // Static upper part of document.
            //
            { // HTML.Division
                HTML::Division division(instance, "content-north", HTML::Nothing);

                // Contents should be located inside of inliner, which defines fixed location on a page.
                //
                { // HTML.Division
                    HTML::Division division(instance, HTML::Nothing, "inliner");

                    this->generateNorth(connection, instance);
                } // HTML.Division
            } // HTML.Division

            // Static lower part of document.
            //
            { // HTML.Division
                HTML::Division division(instance, "content-south", HTML::Nothing);

                // Contents should be located inside of inliner, which defines fixed location on a page.
                //
                { // HTML.Division
                    HTML::Division division(instance, HTML::Nothing, "inliner");

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
WWW::Site::processRelays(HTTP::Connection& connection, HTML::Instance& instance)
{
    try
    {
        const unsigned long relayIndex = connection[WWW::SwitchRelay];

        GPIO::RelayStation& relayStation = GPIO::RelayStation::SharedInstance();

        try
        {
            GPIO::Relay* relay = relayStation[relayIndex];

            try
            {
                const std::string relayState = connection[WWW::RelayState];

                if (relayState == WWW::RelayStateDown)
                {
                    relay->switchOff();
                }
                else if (relayState == WWW::RelayStateUp)
                {
                    relay->switchOn();
                }
            }
            catch (HTTP::ArgumentDoesNotExist&)
            {
                relay->switchOver();
            }
        }
        catch (GPIO::Exception)
        {
            instance.errorMessage("Relais #%s ist nicht bekannt", relayIndex);
        }
        catch (...)
        {
            instance.errorMessage("Relais #%s kann nicht geschaltet werden", relayIndex);
        }
    }
    catch (HTTP::ArgumentDoesNotExist&)
    { }
}

/**
 * @brief   Generate the north (upper) part of main page.
 *
 * @param[in]   connection  Pointer to HTTP connection.
 * @param[in]   instance    Pointer to HTML instance.
 */
void
WWW::Site::generateNorth(HTTP::Connection& connection, HTML::Instance& instance)
{
    { // HTML.Division
        HTML::Division division(instance, HTML::Nothing, "tabs");

        // Tabs line.
        //
        { // HTML.UnorderedList
            HTML::UnorderedList unorderedList(instance, HTML::Nothing, "tabs_list");

            // 'Start' tab.
            //
            { // HTML.ListItem
                HTML::ListItem listItem(instance,
                        HTML::Nothing,
                        ((connection.pageName().empty() == true) ||
                        (connection.pageName() == WWW::PageSystemInformation))
                                ? "tabs_item active"
                                : "tabs_item");

                { // HTML.URL
                    HTML::URL url(instance, WWW::PageSystemInformation);

                    { // HTML.Span
                        HTML::Span span(instance, HTML::Nothing, "title");

                        span.plain("Start");
                    } // HTML.Span

                    { // HTML.Span
                        HTML::Span span(instance, HTML::Nothing, "subtitle");

                        span.plain("Systeminformation");
                    } // HTML.Span
                } // HTML.URL
            }

            // 'Therma' tab.
            //
            { // HTML.ListItem
                HTML::ListItem listItem(instance,
                        HTML::Nothing,
                        (connection.pageName() == WWW::PageTherma)
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
                        (connection.pageName() == WWW::PageRelay)
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
WWW::Site::generateSouth(HTTP::Connection& connection, HTML::Instance& instance)
{
    if (connection.pageName().empty() == true)
    {
        this->pageSystemInformation(connection, instance);
    }
    else if (connection.pageName() == WWW::PageSystemInformation)
    {
        this->pageSystemInformation(connection, instance);
    }
    else if (connection.pageName() == WWW::PageTherma)
    {
        this->pageTherma(connection, instance);
    }
    else if (connection.pageName() == WWW::PageRelay)
    {
        this->pageRelay(connection, instance);
    }
    else
    {
        this->pageSystemInformation(connection, instance);
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
WWW::Site::formSubmitted(HTTP::Connection& connection)
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
WWW::Site::formCancelled(HTTP::Connection& connection)
{
    return connection.argumentPairExists(WWW::Button, WWW::ButtonCancel);
}
