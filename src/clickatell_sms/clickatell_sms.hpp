#ifndef CLICKATELL_SMS_H
#define CLICKATELL_SMS_H

/*
 * clickatell_sms.h
 *
 * Clickatell SMS class library:
 *
 * This library allows a developer to integrate their application
 * with the both the Clickatell REST API and Clickatell HTTP API.
 * The following public functions are provided.
 *   -  send MT message(s)
 *   -  get user’s credit balance
 *   -  get message status
 *   -  get message ClickSmsStringge
 *   -  get coverage
 *   -  stop message
 *
 *  Martin Beyers <martin.beyers@clickatell.com>
 */
#include <curl/curl.h>

// enumeration designating Clickatell APIs supported in this class library
enum eClickApi {
    CLICK_API_HTTP,   // HTTP API using username+password to authenticate
    CLICK_API_REST,   // REST API using api_key (an auth token) to authenticate
    CLICK_API_COUNT
}; // count of supported APIs

// enum designating cURL request types
enum eClickCurlRequestType{CLICK_CURL_GET,     // REST or HTTP
                           CLICK_CURL_POST,    // REST or HTTP
                           CLICK_CURL_DELETE}; // REST API only

// enumeration designating possible login credentials
enum eClickLoginCred {
    CLICK_CRED_USER,   // API using username for Clickatell APIs such as HTTP
    CLICK_CRED_PASS,   // API using password for Clickatell APIs such as HTTP
    CLICK_CRED_APIKEY, // API token for the Clickatell REST API
    CLICK_CRED_APIID   // API ID for a Clickatell API
}; // count of supported APIs

// key/value pair container
struct ClickKeyVal {
    std::string sKey; // parameter key string
    std::string sVal; // parameter value string
};

// username/password container
struct ClickUserPass {
    std::string sUsername; // user's Clickatell API username
    std::string sPassword; // user's Clickatell API password

    ClickUserPass(){ }
    ClickUserPass(std::string sUsername_, std::string sPassword_)
                  : sUsername(sUsername_),
                    sPassword(sPassword_) { }
};

// Clickatell SMS class
class ClickatellSms
{
private:
    // ---------------------------------------------------------------------------------------------
    // private class functions

    void Initialize(long iTimeout, long iConnectTimeout);
    void LocalCurlConfig(long iTimeout, long iConnectTimeout);
    void LocalCurlExecute(std::string &sPostData);
    void LocalApiCommandExecute(const std::string &sPath,
                                const std::vector<ClickKeyVal> &vKeyVals,
                                const std::vector<std::string> &vMsisdns);

    // ---------------------------------------------------------------------------------------------
    // private class members

    // static initialization list parameter validation functions
    static eClickApi ValidateParamAPI(eClickApi eApiType);
    static std::string ValidateApiString(eClickLoginCred eCred, std::string &sParam);

    // static URL
    static std::string sLocalBaseUrl; // shared base URL

    // input configuration
    eClickApi eUserApiType;  // API type
    std::string sUserApiId;  // user's Clickatell API ID when user creates a new API in Clickatell central.
    ClickUserPass oUserCred; // username+password login credentials
    std::string sUserApiKey; // REST API Key login credential
    std::string sFullUrl;    // URL request to Clickatell

    ClickDebug oLocalDebug;  // local debug instance

    eClickCurlRequestType eRequest; // Type of request (i.e. POST, GET, DELETE)

    // output data
    std::string sClickatellResponse; // Clickatell API response string

    // cURL-request class members
    struct curl_slist *curlHeaders; // cURL header data
    long     curlHttpStatus;        // HTTP status code
    CURL    *curlHandle;            // libcurl handle
    CURLcode curlCode;              // return code from recent curl request

public:
    // ---------------------------------------------------------------------------------------------
    // public functions

    /* ClickatellSms constructor declaration with initialization list
     * This constructor creates an HTTP Clickatell SMS object which requires an HTTP username and
     * HTTP password to be set as arguments to the constructor.
     */
    ClickatellSms(eClickDebugOption eDebugOpt, eClickApi eApiType, std::string sUsername, std::string sPassword, std::string &sApiId,
                  long iTimeout, long iConnectTimeout)
                  : eUserApiType((ValidateParamAPI(eApiType))),
                    sUserApiId((ValidateApiString(CLICK_CRED_APIID, sApiId))),
                    oUserCred(((ValidateApiString(CLICK_CRED_USER, sUsername))),
                              ((ValidateApiString(CLICK_CRED_PASS, sPassword)))),
                    oLocalDebug(eDebugOpt)
                  {Initialize(iTimeout, iConnectTimeout);}

    /* ClickatellSms constructor declaration with initialization list
     * This constructor creates a REST Clickatell SMS object which requires a REST API Key (token)
     * to be set as an argument to the constructor.
     */
    ClickatellSms(eClickDebugOption eDebugOpt, eClickApi eApiType, std::string &sApiKey, std::string &sApiId,
                  long iTimeout, long iConnectTimeout)
                  : eUserApiType((ValidateParamAPI(eApiType))),
                    sUserApiId((ValidateApiString(CLICK_CRED_APIID, sApiId))),
                    sUserApiKey((ValidateApiString(CLICK_CRED_APIKEY, sApiKey))),
                    oLocalDebug(eDebugOpt)
                  {Initialize(iTimeout, iConnectTimeout);}

    ~ClickatellSms();

    // Clickatell API functions
    std::string SmsMessageSend(const std::string &sText, const std::vector<std::string> &vMsisdns);
    std::string SmsStatusGet(const std::string &sMsgId);
    std::string SmsBalanceGet();
    std::string SmsChargeGet(const std::string &sMsgId);
    std::string SmsCoverageGet(const std::string &sMsisdn);
    std::string SmsMessageStop(const std::string &sMsgId);

    void SetResponse(char *chStr); // setter function (can be called from a callback, so it needs to be public)

    friend std::ostream& operator<<(std::ostream& os, const ClickatellSms &oClickSms);
};

#endif // CLICKATELL_SMS_H
