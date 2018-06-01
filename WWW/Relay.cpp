// Common definition files.
//
#include "GPIO/Relay.hpp"
#include "HTTP/Connection.hpp"
#include "HTTP/HTML.hpp"
#include "HTTP/HTTP.hpp"

// Local definition files.
//
#include "Servus/WWW/Home.hpp"

/**
 * @brief   Generate HTML page for the 'Relay' tab.
 *
 * @param   connection      HTTP connection.
 * @param   instance        HTML instance.
 */
void
WWW::Site::pageRelay(HTTP::Connection& connection, HTML::Instance& instance)
{
    HTML::Division division(instance, HTML::Nothing, "workspace");

    { // HTML.Division
        HTML::Division division(instance, "full", "slice");

        { // HTML.HeadingText
            HTML::HeadingText headingText(instance, HTML::H2, HTML::Left);

            headingText.plain("Relaisstation");
        } // HTML.HeadingText

        {
            HTML::Table table(instance);

            {
                HTML::TableHeader tableHeader(instance);

                {
                    HTML::TableRow tableRow(instance);

                    {
                        HTML::TableDataCell tableDataCell(instance);

                        tableDataCell.plain("Bezeichnung");
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance);

                        tableDataCell.plain("Stand");
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance,
                                HTML::Nothing,
                                HTML::Nothing,
                                2);
                    }
                }
            }

            GPIO::RelayStation& relayStation = GPIO::RelayStation::SharedInstance();

            {
                HTML::TableBody tableBody(instance);

                for (unsigned int relayIndex = 0;
                     relayIndex < relayStation.size();
                     relayIndex++)
                {
                    GPIO::Relay* relay = relayStation[relayIndex];

                    HTML::TableRow tableRow(instance);

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "label");

                        tableDataCell.plain(relay->name);
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing,
                                (relay->isOff()) ? "red" : "green");

                        tableDataCell.plain((relay->isOff()) ? "Aus" : "Ein");
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "action");

                        { // HTML.URL
                            char urlString[200];

                            snprintf(urlString, sizeof(urlString),
                                    "%s?%s=%u&%s=%s",
                                    connection.pageName().c_str(),
                                    WWW::SwitchRelay.c_str(),
                                    relayIndex,
                                    WWW::RelayState.c_str(),
                                    WWW::RelayStateUp.c_str());

                            HTML::URL url(instance,
                                    urlString,
                                    "Schalte Relais ein.");

                            url.image("img/enable.png", "Ein");

                            { // HTML.Span
                                HTML::Span span(instance, HTML::Nothing, HTML::Nothing);

                                span.plain("Ein");
                            } // HTML.Span
                        } // HTML.URL
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "action");

                        { // HTML.URL
                            char urlString[200];

                            snprintf(urlString, sizeof(urlString),
                                    "%s?%s=%u&%s=%s",
                                    connection.pageName().c_str(),
                                    WWW::SwitchRelay.c_str(),
                                    relayIndex,
                                    WWW::RelayState.c_str(),
                                    WWW::RelayStateDown.c_str());

                            HTML::URL url(instance,
                                    urlString,
                                    "Schalte Relais aus.");

                            url.image("img/disable.png", "Aus");

                            { // HTML.Span
                                HTML::Span span(instance, HTML::Nothing, HTML::Nothing);

                                span.plain("Aus");
                            } // HTML.Span
                        } // HTML.URL
                    }
                }
            }
        }
    } // HTML.Division
}
