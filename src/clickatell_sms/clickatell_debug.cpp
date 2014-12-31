/*
 * clickatell_debug.cpp
 *
 *  Simple debug module used by the Clickatell SMS library.
 *
 *  Martin Beyers <martin.beyers@clickatell.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "clickatell_debug.hpp"


/* ----------------------------------------------------------------------------- *
 * Public function definitions                                                   *
 * ----------------------------------------------------------------------------- */

/*
 * Function:  ~ClickDebug
 * Info:      Destructor. Does nothing for now.
 * Inputs:    None
 * Outputs:   None
 * Return:    void
 */
ClickDebug::~ClickDebug()
{}

/*
 * Function:  SetOption
 * Info:      Turns debug ON or OFF
 * Inputs:    eDebugOption - debug setting
 * Outputs:   None
 * Return:    void
 */
void ClickDebug::SetOption(eClickDebugOption eDebugOption)
{
    if (eDebugOption >= 0 && eDebugOption < CLICK_DEBUG_COUNT)
        eLocalDebugOption = eDebugOption;
}

/*
 * Function:  Print
 * Info:      Formats and prints out a variable argument list. At present this functionality
 *            is similar to using a vprintf call except that the function will not output
 *            debug if debug was disabled.
 * Inputs:    chFormat - name of the last parameter before the variable argument list.
 *            args     - variable number of args
 * Outputs:   None
 * Return:    void
 */
void ClickDebug::Print(const char *chFormat, ...)
{
    if (chFormat == NULL || eLocalDebugOption != CLICK_DEBUG_ON)
        return;

    va_list arg_list;

    va_start(arg_list, chFormat);
    vprintf(chFormat, arg_list);

    va_end(arg_list);
}
