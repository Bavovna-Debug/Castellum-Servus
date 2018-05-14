#ifdef DEBUG

// System definition files.
//
#include <cstdio>
#include <cstdlib>

// Common definition files.
//
#include "Toolkit/Report.h"
#include "Toolkit/Types.h"

// Local definition files.
//
#include "Servus/Tools/Debug.hpp"
#include "Servus/Tools/Memory.hpp"

/**
 * @brief   Dump data in hexadecimal form.
 *
 * This function could be used for debugging purposes to dump binary data
 * in human readable form to the log file. This function has to be used
 * with care - it issues malloc() at each invocation, which may cause
 * performance issue.
 *
 * @param[in]   label   Text label to be printed at the beginning of line.
 * @param[in]   buffer  Pointer to data.
 * @param[in]   length  Size of data (in bytes) to dump out.
 */
void
Dump(const char* label, void* buffer, unsigned int length)
{
    static char*    string = NULL;
    char*           cursorOverBuffer;
    char*           cursorOverString;
    unsigned char   c;
    unsigned int    index;

    if (string == NULL)
    {
        string = (char*) malloc(4 * KB);
    }

    cursorOverBuffer = (char*) buffer;
    cursorOverString = string;

    for (index = 0;
         index < length;
         index++)
    {
        if ((index % 4) == 0)
        {
            cursorOverString += sprintf(cursorOverString, " ");
        }

        c = *cursorOverBuffer;

        cursorOverString += sprintf(cursorOverString, "%02X", c);

        cursorOverBuffer++;
    }

    ReportDump("%s:%s", label, string);
}

#endif
