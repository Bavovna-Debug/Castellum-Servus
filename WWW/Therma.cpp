// Commmon definition files.
//
#include "HTTP/Connection.hpp"
#include "HTTP/HTML.hpp"
#include "HTTP/HTTP.hpp"

// Local definition files.
//
#include "Servus/Peripherique/ThermiqueSensor.hpp"
#include "Servus/Peripherique/ThermiqueStation.hpp"
#include "Servus/WWW/Home.hpp"

/**
 * @brief   Generate HTML page for the 'Therma' tab.
 *
 * @param   connection      HTTP connection.
 * @param   instance        HTML instance.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void
WWW::Site::pageTherma(HTTP::Connection& connection, HTML::Instance& instance)
{
    HTML::Division division(instance, HTML::Nothing, "workspace");

    division.meta("refresh", "10");

    { // HTML.Division
        HTML::Division division(instance, "full", "slice");

        { // HTML.HeadingText
            HTML::HeadingText headingText(instance, HTML::H2, HTML::Left);

            headingText.plain("Therma");
        } // HTML.HeadingText

        {
            HTML::Table table(instance);

            {
                HTML::TableHeader tableHeader(instance);

                {
                    HTML::TableRow tableRow(instance);

                    {
                        HTML::TableDataCell tableDataCell(instance);

                        tableDataCell.plain("Sensor");
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "centered");

                        tableDataCell.plain("Tief");
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "centered");

                        tableDataCell.plain("Delta");
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "centered");

                        tableDataCell.plain("Aktuell");
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "centered");

                        tableDataCell.plain("Delta");
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "centered");

                        tableDataCell.plain("Hoch");
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "centered");

                        tableDataCell.plain("Präzision");
                    }
                }
            }

            Peripherique::ThermiqueStation& thermiqueStation = Peripherique::ThermiqueStation::SharedInstance();

            {
                HTML::TableBody tableBody(instance);

                for (unsigned int sensorIndex = 0;
                     sensorIndex < thermiqueStation.size();
                     sensorIndex++)
                {
                    Peripherique::ThermiqueSensor* thermiqueSensor = thermiqueStation[sensorIndex];

                    HTML::TableRow tableRow(instance);

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "label");

                        tableDataCell.plain(thermiqueSensor->title);
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "blue");

                        tableDataCell.plain("%4.2f &#x2103;",
                                thermiqueSensor->lastKnown.lowest);
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "blue");

                        tableDataCell.plain("[-%4.2f &#x2103;]",
                                thermiqueSensor->lastKnown.current - thermiqueSensor->lastKnown.lowest);
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "green");

                        tableDataCell.plain("%4.2f &#x2103;",
                                thermiqueSensor->lastKnown.current);
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "red");

                        tableDataCell.plain("[+%4.2f &#x2103;]",
                                thermiqueSensor->lastKnown.highest - thermiqueSensor->lastKnown.current);
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "red");

                        tableDataCell.plain("%4.2f &#x2103;",
                                thermiqueSensor->lastKnown.highest);
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "value");

                        tableDataCell.plain("%3.2f &#x2103;",
                                thermiqueSensor->edge);
                    }
                }
            }
        }
    } // HTML.Division
}
#pragma GCC diagnostic pop
