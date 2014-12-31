#ifndef CLICKATELL_STRING_H
#define CLICKATELL_STRING_H

/*
 * Clickatell SMS string function declarations
 *
 *  Basic wrapper on top of C++ std::string functions. The functions in this file are
 *  associated with the 'clickstr' namespace.
 *
 * Martin Beyers <martin.beyers@clickatell.com>
 */

// macro to validate a string
#define CLICK_STR_INVALID(sData)  ((sData).empty())

namespace clickstr
{
    void click_string_append_formatted_cstr(std::string &sDest, const char *cstrFormat, ...);
    void click_string_trim_prefix(std::string &sData, unsigned int iLen);
    void click_string_url_encode(std::string &sData);
}

#endif // CLICKATELL_STRING_H
