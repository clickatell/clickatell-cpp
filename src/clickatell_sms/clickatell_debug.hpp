#ifndef CLICKATELL_DEBUG_H
#define CLICKATELL_DEBUG_H

/*
 * clickatell_debug.h
 *
 *  Simple debug module used by the Clickatell SMS library.
 *
 *  Martin Beyers <martin.beyers@clickatell.com>
 */

// global enumeration to specify debug on/off
enum eClickDebugOption {
    CLICK_DEBUG_ON,
    CLICK_DEBUG_OFF,
    CLICK_DEBUG_COUNT
};

class ClickDebug
{
private:
    eClickDebugOption eLocalDebugOption;

public:
    // constructor declaration with initialization list
    ClickDebug(eClickDebugOption eDebugOption)
               : eLocalDebugOption(CLICK_DEBUG_ON) {}
    // destructor
    ~ClickDebug();
    // public functions
    void SetOption(eClickDebugOption eDebugOption);
    void Print(const char *chFormat, ...);
};

#endif // CLICKATELL_DEBUG_H
