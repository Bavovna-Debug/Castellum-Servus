// Commmon definition files.
//
#include "GPIO/Therma.hpp"
#include "HTTP/Connection.hpp"
#include "HTTP/HTML.hpp"
#include "HTTP/HTTP.hpp"

// Local definition files.
//
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
                }
            }

            Therma::Service& thermaService = Therma::Service::SharedInstance();

            {
                HTML::TableBody tableBody(instance);

                for (unsigned int sensorIndex = 0;
                     sensorIndex < thermaService.size();
                     sensorIndex++)
                {
                    Therma::Sensor* sensor = thermaService[sensorIndex];

                    HTML::TableRow tableRow(instance);

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "label");

                        tableDataCell.plain(sensor->name);
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "blue");

                        tableDataCell.plain("%4.2f &#x2103;",
                                sensor->temperature.lowest);
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "blue");

                        tableDataCell.plain("[-%4.2f &#x2103;]",
                                sensor->temperature.current - sensor->temperature.lowest);
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "green");

                        tableDataCell.plain("%4.2f &#x2103;",
                                sensor->temperature.current);
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "red");

                        tableDataCell.plain("[+%4.2f &#x2103;]",
                                sensor->temperature.highest - sensor->temperature.current);
                    }

                    {
                        HTML::TableDataCell tableDataCell(instance, HTML::Nothing, "red");

                        tableDataCell.plain("%4.2f &#x2103;",
                                sensor->temperature.highest);
                    }
                }
            }
        }
    } // HTML.Division
}
#pragma GCC diagnostic pop
