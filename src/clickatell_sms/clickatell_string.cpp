/*
 * clickatell_string.c
 *
 *  Basic wrapper on top of C++ std::string functions. The functions in this file will
 *  be associated with the 'clickstr' namespace.
 *
 *  Martin Beyers <martin.beyers@clickatell.com>
 */

#include <stdarg.h>
#include <string>

#include "clickatell_debug.hpp"
#include "clickatell_string.hpp"

/* ----------------------------------------------------------------------------- *
 * Macros/Types                                                                  *
 * ----------------------------------------------------------------------------- */

// character array designating a URL-safe character range
#define URL_ENCODE_SAFE_CHAR(c) (((c) >= '0' && (c) <= '9') || \
                                 ((c) >= 'A' && (c) <= 'Z') || \
                                 ((c) >= 'a' && (c) <= 'z') || \
                                 (c) == '-' || (c) == '_' || (c) == '.' || (c) == '~')

static ClickDebug oDebug(CLICK_DEBUG_ON);

/*
 * Function:  click_string_append_formatted_cstr
 * Info:      Appends a formatted source character string to a destination std::string
 * Inputs:    sData      - target string to append to
 *            cstrFormat - character string with variable list of args to append
 * Return:    void
 */
void clickstr::click_string_append_formatted_cstr(std::string &sData, const char *cstrFormat, ...)
{
    if (sData.empty()) {
        oDebug.Print("%s ERROR: Invalid parameter!\n", __func__);
        return;
    }

    int iAppendLen = 0;
    va_list argList, argListCopy;

    /* obtain length of formatted characters for dynamic malloc operation, then
     * format the string. The 2nd argument of vsnprintf() here is set to zero to ensure
     * vsnprintf() does not write to the 1st argument */
    va_start(argList, cstrFormat);

    va_copy(argListCopy, argList); // can only pass va_list once to vsnprintf(), so make a copy for the 2nd vsnprintf() call
    iAppendLen = vsnprintf(NULL,
                        0,
                        cstrFormat,
                        argList);
    char *chTemp = (char *)malloc(iAppendLen + 1);
    vsnprintf(chTemp, iAppendLen + 1, cstrFormat, argListCopy);

    va_end(argList);

    sData.append(chTemp);
    free(chTemp);
}

/*
 * Function:  click_string_trim_prefix
 * Info:      Remove prefix string from a std::string.
 *            If prefix string iLength is the same or longer, all characters in the
 *            string will be removed.
 * Inputs:    sData - std::string containing prefix to remove
 *            iLen - iLength of prefix data to remove
 * Return:    void
 */
void clickstr::click_string_trim_prefix(std::string &sData, unsigned int iLen)
{
    if (sData.empty())
        oDebug.Print("%s ERROR: Invalid parameter!\n", __func__);
    else
        sData.erase(0, iLen);
}

/*
 * Function:  click_string_url_encode
 * Info:      URL-encodes a std::string.
 * Inputs:    sData - string to URL-encode
 * Return:    void
 */
void clickstr::click_string_url_encode(std::string &sData)
{
    if (CLICK_STR_INVALID(sData)) {
        oDebug.Print("%s ERROR: Invalid parameter!\n", __func__);
        return;
    }

    std::string sEncData;                           // url-encoded output string
    std::string::iterator iterData = sData.begin(); // pointer to original string
    unsigned int iDecimalVal = 0;
    char chBuf[3];

    // traverse string searching for characters to URL encode
    while(*iterData != '\0') {
        if (URL_ENCODE_SAFE_CHAR(*iterData)) {
            sEncData.push_back(*iterData); // safe characters remain as is
        }
        else {
            /* http://www.w3.org/Addressing/URL/uri-spec.html#z5 states:
             * "Within the query string, the plus sign is reserved as shorthand notation for a space."
             * note this also saves space in the SMS message instead of using 3 characters "%20" per space
             * we instead utilize just one "+"
             */
            if (*iterData == ' ') {
                sEncData.push_back('+'); // use + instead of %20
            }
            else {
                // add URL-encoded 3-character replacement, i.e.  +  becomes  %2B
                sEncData.push_back('%');

                iDecimalVal = ((*iterData >> 4) & 0xf);
                sprintf(chBuf, "%x", iDecimalVal); // convert upper nibble to lowercase hex char
                sEncData.push_back(chBuf[0]);

                iDecimalVal = ((*iterData) & 0xf);
                sprintf(chBuf, "%x", iDecimalVal); // convert lower nibble to lowercase hex char
                sEncData.push_back(chBuf[0]);
            }
        }

        iterData++;
    }

    sData.swap(sEncData); // clear original string and replace with URL-encoded string
}
