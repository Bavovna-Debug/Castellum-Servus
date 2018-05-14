#pragma once

#ifdef DEBUG

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
Dump(const char* label, void* buffer, unsigned int length);

#endif
