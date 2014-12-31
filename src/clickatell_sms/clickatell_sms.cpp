/*
 * clickatell_sms.cpp
 *
 *  Clickatell SMS class library
 *
 *  This library allows a developer to integrate their application with both the
 *  Clickatell REST API and Clickatell HTTP API. The Clickatell REST API does support XML format
 *  for requests and responses, but note that in this library we transmit REST requests in JSON
 *  format and receive corresponding responses in JSON format.
 *  The following public functions are provided for each API type:
 *   -  send MT message(s)
 *   -  get user’s credit balance
 *   -  get message status
 *   -  get message charge
 *   -  get coverage
 *   -  stop a message
 *
 *  The Clickatell SMS object integrates with libcurl (free client-side URL transfer library).
 *  Libcurl is cross-platform, and therefore if running a windows application,
 *  one would just need to download the relevant libcurl resource from
 *  http://curl.haxx.se/download.html. If working with a Linux OS, the "curl-devel" package
 *  should be installed prior to building this library with a Makefile.
 *
 *   Martin Beyers <martin.beyers@clickatell.com>
 */

#include <iostream>
#include <string>
#include <vector>

#include <ctype.h>
#include "curl/curl.h"

#include "clickatell_debug.hpp"
#include "clickatell_string.hpp"
#include "clickatell_sms.hpp"


/* ----------------------------------------------------------------------------- *
 * Types/Macros                                                                  *
 * ----------------------------------------------------------------------------- */

// default cURL request timeout values
#define CLICK_SMS_DEFAULT_APICALL_TIMEOUT          5  // max time allowed for api call to Clickatell
#define CLICK_SMS_DEFAULT_APICALL_CONNECT_TIMEOUT  5  // max connection time allowed for api call to Clickatell

// macro to validate API type
#define VALIDATE_API_TYPE(api)                 ((api) >= CLICK_API_HTTP &&  (api) < CLICK_API_COUNT)

// static member variable assignments
ClickDebug oLocalDebug(CLICK_DEBUG_ON); // shared debug instance
std::string ClickatellSms::sLocalBaseUrl("https://api.clickatell.com/");

// static member functions
eClickApi ClickatellSms::ValidateParamAPI(eClickApi eApiType)
{
    if (!VALIDATE_API_TYPE(eApiType)) {
        std::string sInfo("Invalid API type for ");
        clickstr::click_string_append_formatted_cstr(sInfo, "eClickApi:%d", eApiType);
        throw sInfo;
    }

    return eApiType;
}

std::string ClickatellSms::ValidateApiString(eClickLoginCred eCred, std::string &sParam)
{
    if (CLICK_STR_INVALID(sParam)) {
        std::string sInfo("Invalid ");

        switch (eCred) {
            case CLICK_CRED_USER:
                sInfo.append("user");
                break;
            case CLICK_CRED_PASS:
                sInfo.append("password");
                break;
            case CLICK_CRED_APIKEY:
                sInfo.append("API Key");
                break;
            case CLICK_CRED_APIID:
                sInfo.append("API ID");
                break;
            default:
                sInfo.append("constructor parameter");
                break;
        }

        throw sInfo;
    }

    return sParam;
}

/* ----------------------------------------------------------------------------- *
 * Free (non-class) functions                                                    *
 * ----------------------------------------------------------------------------- */

/*
 * Function:  LocalCurlResponseCallback
 * Info:      This is a cURL callback function prototype. It acts as a cURL response callback
 *            function which is set in function ClickatellSms::LocalCurlConfig() when configuring
 *            the cURL CURLOPT_WRITEFUNCTION.
 *            The 'response' parameter passed back here was set in function
 *            ClickatellSms::LocalCurlExecute() when configuring the cURL CURLOPT_WRITEDATA.
 *            This callback function reads a curl request's response data. In here we
 *            allocate the response data to the corresponding ClickatellSms instance's response
 *            field.
 * Return:    Total size of response data buffer
 */
size_t LocalCurlResponseCallback(void *buffer, size_t iSize, size_t iMemLen, void *response)
{
    int iTotalSize = iMemLen * iSize;

    if (iTotalSize > 0) {
        char *cstrTempData = NULL;

        // use static_cast C-type cast for safe (stricter) casting in order to catch bad casts at compile time
        ClickatellSms *instance_ptr = static_cast<ClickatellSms *>(response);

        if ((cstrTempData = (char *)calloc(iTotalSize + 1, sizeof(char))) != NULL) {
            memcpy(cstrTempData, buffer, iTotalSize);
            cstrTempData[iTotalSize] = '\0';

            instance_ptr->SetResponse(cstrTempData);

            free(cstrTempData);
        }
    }

    return (iTotalSize);
}

/*
 * Function:  << operator overload friend function
 * Info:      Function which overloads the << ostream operator and accesses some private
 *            ClickatellSms class members (for READ purposes only). An external calling function
 *            would use this to output details regarding the last cURL API request that
 *            was made to Clickatell.
 * Inputs:    os        - ostream object
 *            oClickSms - ClickatellSms object which we can pass into the output stream.
 * Return:    std::ostream - output stream
 */
std::ostream& operator<<(std::ostream& os, const ClickatellSms &oClickSms)
{
    std::string sReq((oClickSms.eRequest == CLICK_CURL_POST ? "POST" :
                      (oClickSms.eRequest == CLICK_CURL_GET ? "GET" : "DELETE")));

    os << "Curl " << sReq.c_str() << "-Request URL:\n" << oClickSms.sFullUrl.c_str() << std::endl
       << "Curl HTTP response code:\n" << oClickSms.curlHttpStatus << std::endl
       << "Curl response:\n" << oClickSms.sClickatellResponse.c_str() << std::endl;

    return os;
}

/* ----------------------------------------------------------------------------- *
 * Private function definitions                                                  *
 * ----------------------------------------------------------------------------- */

/*
 * Function:  ClickatellSms::LocalCurlConfig
 * Info:      Initializes private cURL handle using standard libcurl library functions.
 *            This function applies standard cURL configs. For REST/HTTP-specific
 *            cURL configuration logic, please see function ClickatellSms::LocalCurlExecute().
 * Inputs:    iTimeout        - Maximum duration for cURL request to Clickatell server
 *            iConnectTimeout - Maximum timeout for cURL connection to Clickatell server
 * Return:    void
 */
void ClickatellSms::LocalCurlConfig(long iTimeout, long iConnectTimeout)
{
    // set this to 1 for detailed curl debug
    curl_easy_setopt(curlHandle, CURLOPT_VERBOSE, 0);

    // curl version set
    curl_easy_setopt(curlHandle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

    // set here the timeout values for libcurl transfer operation
    curl_easy_setopt(curlHandle,
                     CURLOPT_TIMEOUT,
                     (iTimeout <= 0 ? CLICK_SMS_DEFAULT_APICALL_TIMEOUT : iTimeout));
    curl_easy_setopt(curlHandle,
                     CURLOPT_CONNECTTIMEOUT,
                     (iConnectTimeout <= 0 ? CLICK_SMS_DEFAULT_APICALL_CONNECT_TIMEOUT : iConnectTimeout));

    // Clickatell will write the response data to this write function callback (instead of to stdout)
    curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, LocalCurlResponseCallback);
}

/*
 * Function:  ClickatellSms::LocalCurlExecute
 * Info:      Executes a cURL request using libcurl.
 *            The result of the cURL operation (cURL return code) will be set in the
 *            ClickatellSms instance's 'curlCode' class member.
 *            The cURL operation's response data will be set in the 'sClickatellResponse'
 *            class member of the ClickatellSms instance.
 * Input:     sPostData - cURL 'POST request' data
 * Output:    None
 * Return:    void
 */
void ClickatellSms::LocalCurlExecute(std::string &sPostData)
{
    // add headers if applicable
    if (curlHeaders != NULL)
        curl_easy_setopt(curlHandle, CURLOPT_HTTPHEADER, curlHeaders);
    else // remove headers
        curl_easy_setopt(curlHandle, CURLOPT_HTTPHEADER, NULL);

    // set full URL for curl request and class instance to pass back to response callback
    curl_easy_setopt(curlHandle, CURLOPT_URL, sFullUrl.c_str());
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, this);

    switch (eRequest) {
        case CLICK_CURL_POST:
            // set cURL 'POST request' data if requested and if the post data exists
            if (!CLICK_STR_INVALID(sPostData) && sPostData.length() > 0) {
                curl_easy_setopt(curlHandle, CURLOPT_POST, 1);
                curl_easy_setopt(curlHandle, CURLOPT_POSTFIELDS, sPostData.c_str());
                curl_easy_setopt(curlHandle, CURLOPT_POSTFIELDSIZE, sPostData.length());

                oLocalDebug.Print("Curl post data:\n%s\n", sPostData.c_str());
            }
            break;

        case CLICK_CURL_DELETE:
            curl_easy_setopt(curlHandle, CURLOPT_CUSTOMREQUEST, "DELETE");
            break;

        case CLICK_CURL_GET:
        default:
            curl_easy_setopt(curlHandle, CURLOPT_HTTPGET, 1);
            break;
    }

    // execute curl request
    curlCode = curl_easy_perform(curlHandle);

    // obtain response code
    if (curlCode == CURLE_OK)
        curlCode = curl_easy_getinfo(curlHandle, CURLINFO_RESPONSE_CODE, &curlHttpStatus);
}



/* ----------------------------------------------------------------------------- *
 * Public function definitions                                                   *
 * ----------------------------------------------------------------------------- */

/*
 * Function:  Initialize
 * Info:      Initializes Clickatell SMS class instance after private variables
 *            were initialized within constructor initialization list.
 * Inputs:    iTimeout        - Maximum timeout for API call to take
 *            iConnectTimeout - Maximum timeout for API call connection to take
 * Return:    void
 */
void ClickatellSms::Initialize(long iTimeout, long iConnectTimeout)
{
    if ((curlHandle = curl_easy_init()) == NULL)
        throw (std::string("curl_easy_init failed!"));

    LocalCurlConfig(iTimeout, iConnectTimeout);

    // REST requires API Key only and other APIs (ie HTTP) require username+password for authentication
    if (eUserApiType == CLICK_API_REST) {
        // configure default headers - always ensure first slist append call has NULL headers arg
        curlHeaders = curl_slist_append(NULL, "X-Version: 1");
        curlHeaders = curl_slist_append(curlHeaders, "Content-Type: application/json");
        curlHeaders = curl_slist_append(curlHeaders, "Accept: application/json");

        // the REST API key will be used as the authorization token
        std::string sToken("Authorization: Bearer ");
        sToken.append(sUserApiKey);
        curlHeaders = curl_slist_append(curlHeaders, sToken.c_str());

        // set default headers - can replace them if necessary
        curl_easy_setopt(curlHandle, CURLOPT_HTTPHEADER, curlHeaders);
    }
    else {
        // configure default headers - always ensure first slist append call has NULL headers arg
        curlHeaders = curl_slist_append(NULL, "Connection:keep-alive");
        curlHeaders = curl_slist_append(curlHeaders, "Cache-Control:max-age=0");
        curlHeaders = curl_slist_append(curlHeaders, "Origin:null");

        // set default headers - can replace them if necessary
        curl_easy_setopt(curlHandle, CURLOPT_HTTPHEADER, curlHeaders);
    }
}

/*
 * Function:  ~ClickatellSms
 * Info:      Destructor. Destroy a Clickatell SMS instance.
 *            This function will be called when a Clickatell SMS object
 *            is destroyed.
 * Inputs:    none
 * Return:    none
 */
ClickatellSms::~ClickatellSms()
{
    // free curl resources for this object instance
    if (curlHeaders != NULL) {
        curl_slist_free_all(curlHeaders);
        curlHeaders = NULL;
    }

    if (curlHandle != NULL) {
        curl_easy_cleanup(curlHandle);
        curlHandle = NULL;
    }
}

/*
 * Function:  LocalApiCommandExecute
 * Info:      Common function to execute a Clickatell API call.
 * Inputs:    sPath    - Local URL path to resource which will be appended to base URL
 *            vKeyVals - Vector of key/value pairs. The vector should be empty if no key/value pairs
 *                       will be used.
 *            vMsisdns - Vector of destination addresses (for send message call only).
 *                       If not performing a send message call, this vector should be empty.
 * Return:    void
 */
void ClickatellSms::LocalApiCommandExecute(const std::string &sPath,
                                           const std::vector<ClickKeyVal> &vKeyVals,
                                           const std::vector<std::string> &vMsisdns)
{
    unsigned int i = 0;
    std::string sPostData, sApiParams;

    if (CLICK_STR_INVALID(sPath)) {
        oLocalDebug.Print("%s ERROR: invalid parameter!\n", __func__);
        return;
    }

    // format URL key/value parameters
    if (!vKeyVals.empty()) {
       if (eUserApiType == CLICK_API_HTTP) {
            sApiParams += "?";

            // append all non-"to" parameters first
            for (i = 0; i < vKeyVals.size(); i++) {
                clickstr::click_string_append_formatted_cstr(sApiParams, "%s%s=%s", (i == 0 ? "" : "&"),
                                                             (vKeyVals[i].sKey.c_str()),
                                                             (vKeyVals[i].sVal.c_str()));
            }

            // For send message API calls only: append "to" parameter, example:  &to=2799900001,2799900002
            if (!vMsisdns.empty()) {
                sApiParams.append("&to=");

                for (i = 0; i < vMsisdns.size(); i++) {
                    clickstr::click_string_append_formatted_cstr(sApiParams, "%s%s", (i == 0 ? "" : ","),
                                                                 vMsisdns[i].c_str());
                }
            }
        }
        else { // REST
            sApiParams.append("{"); // the JSON data is enclosed in opening/closing braces

            // append all non-"to" parameters first, example:  {"text":"Test Message","callback":"7"}
            for (i = 0; i < vKeyVals.size(); i++) {
                clickstr::click_string_append_formatted_cstr(sApiParams, "%s\"%s\":\"%s\"", (i == 0 ? "" : ","),
                        (char *)(vKeyVals[i].sKey.c_str()),
                        (char *)(vKeyVals[i].sVal.c_str()));
            }

            // For send message API calls only: append "to" parameter, example:  "to":["2799900001","2799900002"]}'
            if (!vMsisdns.empty()) {
                sApiParams.append(",\"to\":[");

                for (i = 0; i < vMsisdns.size(); i++) {
                    clickstr::click_string_append_formatted_cstr(sApiParams, "%s\"%s\"", (i == 0 ? "" : ","),
                                                                 vMsisdns[i].c_str());
                }

                sApiParams.append("]");
            }
            sApiParams.append("}"); // the JSON data is enclosed in opening/closing braces
        }
    }

    // clear last URL
    sFullUrl.clear();
    // format full URL by combining 1. Clickatell base URL 2. API call script / resource sPath and 3. CGI key/value parameters
    sFullUrl.append(ClickatellSms::sLocalBaseUrl);
    // append API call script (HTTP) / resource sPath (REST)
    sFullUrl.append(sPath);

    // cURL request-specific logic
    if (!vKeyVals.empty()) {
        switch (eRequest) {
            case CLICK_CURL_POST:
                sPostData.append(sApiParams);
                break;

            case CLICK_CURL_DELETE:
                sFullUrl.append(sApiParams);
                break;

            case CLICK_CURL_GET:
            default:
                sFullUrl.append(sApiParams);
                break;
        }
    }

    // execute curl request
    LocalCurlExecute(sPostData);
}

/*
 * Function:  SmsMessageSend
 * Info:      Sends SMSes.
 *            This function will set the URL / sPostData params as follows:
 *            For REST, we need at least 2 key/value pairs:
 *               "text" "to"
 *            For other APIs (ie HTTP), we need at least 5 key/value pairs:
 *               "user" "password" "api_id" "text" "to"
 * Inputs:    sText     - Message Text (Latin1 input format supported in this library)
 *            vMsisdns - Vector of destination mobile number strings
 * Return:    API Message ID or error code if operation unsuccessful or NULL if invalid parameter
 */
std::string ClickatellSms::SmsMessageSend(const std::string &sText, const std::vector<std::string> &vMsisdns)
{
    unsigned int i = 0;
    std::string sPath;                 // API call script file / resource sPath designator
    std::vector<ClickKeyVal> vKeyVals; // array of key/val structures, excluding "to" field

    eRequest = (eUserApiType == CLICK_API_HTTP ? CLICK_CURL_GET : CLICK_CURL_POST);

    if (CLICK_STR_INVALID(sText) || vMsisdns.empty()) {
        oLocalDebug.Print("%s ERROR: invalid parameter!\n", __func__);
        return sClickatellResponse;
    }

    if (eUserApiType == CLICK_API_HTTP) {
        sPath.append("http/sendmsg.php");

        // set URL key/value pairs
        for (i = 0; i < 4; i++)
            vKeyVals.push_back(ClickKeyVal());
        vKeyVals[0].sKey.append("user");
        vKeyVals[0].sVal.append(oUserCred.sUsername);
        vKeyVals[1].sKey.append("password");
        vKeyVals[1].sVal.append(oUserCred.sPassword);
        vKeyVals[2].sKey.append("api_id");
        vKeyVals[2].sVal.append(sUserApiId);
        vKeyVals[3].sKey.append("text");
        vKeyVals[3].sVal.append(sText);

        // URL-encode the URL values
        for (i = 0; i < vKeyVals.size(); i++)
            clickstr::click_string_url_encode(vKeyVals[i].sVal);
    }
    else { // REST
        sPath.append("rest/message");

        // set post data Key/Value pairs
        vKeyVals.push_back(ClickKeyVal());
        vKeyVals[0].sKey.append("text");
        vKeyVals[0].sVal.append(sText);
    }

    // performs formatting of API call and then executes the request
    LocalApiCommandExecute(sPath, vKeyVals, vMsisdns);

    return sClickatellResponse;
}

/*
 * Function:  SmsStatusGet
 * Info:      Obtain current status of an SMS message.
 *            Authentication: This function uses username/password to authenticate for the
 *                            HTTP API, however it is also possible in a session to use a session ID to
 *                            authenticate with the HTTP API. See the Clickatell API docs at
 *                            www.clickatell.com for more details.
 *            URL Encoding: For the HTTP API, The URL parameter values are URL-encoded in
 *                          this function.
 * Inputs:    API Message ID - SMS ID assigned by Clickatell
 * Return:    Status of API message
 */
std::string ClickatellSms::SmsStatusGet(const std::string &sMsgId)
{
    unsigned int i = 0;
    std::string sPath;                 // api call path designator
    std::vector<ClickKeyVal> vKeyVals; // array of key/val structures, excluding "to" field
    std::vector<std::string> vMsisdns; // empty vector to pass through

    eRequest = CLICK_CURL_GET;

    if (CLICK_STR_INVALID(sMsgId)) {
        oLocalDebug.Print("%s ERROR: invalid parameter!\n", __func__);
        return sClickatellResponse;
    }

    if (eUserApiType == CLICK_API_HTTP) {
        sPath.append("http/querymsg.php");

        // set URL key/value pairs
        for (i = 0; i < 4; i++)
            vKeyVals.push_back(ClickKeyVal());
        vKeyVals[0].sKey.append("user");
        vKeyVals[0].sVal.append(oUserCred.sUsername);
        vKeyVals[1].sKey.append("password");
        vKeyVals[1].sVal.append(oUserCred.sPassword);
        vKeyVals[2].sKey.append("api_id");
        vKeyVals[2].sVal.append(sUserApiId);
        vKeyVals[3].sKey.append("apimsgid");
        vKeyVals[3].sVal.append(sMsgId);

        // URL-encode URL values
        for (i = 0; i < vKeyVals.size(); i++)
            clickstr::click_string_url_encode(vKeyVals[i].sVal);
    }
    else { // REST
        // example URL:  http://api.clickatell.com/rest/message/47584bae0165fbec57b18bf47895fece
        sPath.append("rest/message/");
        sPath.append(sMsgId);
    }

    // performs formatting of API call and then executes the request
    LocalApiCommandExecute(sPath, vKeyVals, vMsisdns);

    return sClickatellResponse;
}

/*
 * Function:  SmsBalanceGet
 * Info:      Obtain user's credit balance.
 *            Authentication: This function uses username/password to authenticate for the
 *                            HTTP API, however it is also possible in a session to use a session ID to
 *                            authenticate with the HTTP API. See the Clickatell API docs at
 *                            www.clickatell.com for more details.
 *            URL Encoding: For the HTTP API, The URL parameter values are URL-encoded in
 *                          this function.
 * Inputs:    None
 * Return:    User's current balance.
 */
std::string ClickatellSms::SmsBalanceGet()
{
    unsigned int i = 0;
    std::string sPath;                 // API call path designator
    std::vector<ClickKeyVal> vKeyVals; // array of key/val structures, excluding "to" field
    std::vector<std::string> vMsisdns; // empty vector to pass through

    eRequest = CLICK_CURL_GET;

    if (eUserApiType == CLICK_API_HTTP) {
        sPath.append("http/getbalance.php");

        // set URL key/value pairs
        for (i = 0; i < 3; i++)
            vKeyVals.push_back(ClickKeyVal());
        vKeyVals[0].sKey.append("user");
        vKeyVals[0].sVal.append(oUserCred.sUsername);
        vKeyVals[1].sKey.append("password");
        vKeyVals[1].sVal.append(oUserCred.sPassword);
        vKeyVals[2].sKey.append("api_id");
        vKeyVals[2].sVal.append(sUserApiId);

        // URL-encode URL values
        for (i = 0; i < vKeyVals.size(); i++)
            clickstr::click_string_url_encode(vKeyVals[i].sVal);
    }
    else { // REST
        // example URL:  https://api.clickatell.com/rest/account/balance
        sPath.append("rest/account/balance");
    }

    // performs formatting of API call and then executes the request
    LocalApiCommandExecute(sPath, vKeyVals, vMsisdns);

    return sClickatellResponse;
}

/*
 * Function:  SmsChargeGet
 * Info:      Obtain charge of an SMS message.
 *            Authentication: This function uses username/password to authenticate for the
 *                            HTTP API, however it is also possible in a session to use a session ID to
 *                            authenticate with the HTTP API. See the Clickatell API docs at
 *                            www.clickatell.com for more details.
 *            URL Encoding: For the HTTP API, The URL parameter values are URL-encoded in
 *                          this function.
 * Inputs:    API Message ID - SMS ID assigned by Clickatell
 * Return:    Charge of SMS message.
 */
std::string ClickatellSms::SmsChargeGet(const std::string &sMsgId)
{
    unsigned int i = 0;
    std::string sPath;                 // api call path designator
    std::vector<ClickKeyVal> vKeyVals; // array of key/val structures, excluding "to" field
    std::vector<std::string> vMsisdns; // empty vector to pass through

    eRequest = CLICK_CURL_GET;

    if (CLICK_STR_INVALID(sMsgId)) {
        oLocalDebug.Print("%s ERROR: invalid parameter!\n", __func__);
        return sClickatellResponse;
    }

    if (eUserApiType == CLICK_API_HTTP) {
        sPath.append("http/getmsgcharge.php");

        // set URL key/value pairs
        for (i = 0; i < 4; i++)
            vKeyVals.push_back(ClickKeyVal());
        vKeyVals[0].sKey.append("user");
        vKeyVals[0].sVal.append(oUserCred.sUsername);
        vKeyVals[1].sKey.append("password");
        vKeyVals[1].sVal.append(oUserCred.sPassword);
        vKeyVals[2].sKey.append("api_id");
        vKeyVals[2].sVal.append(sUserApiId);
        vKeyVals[3].sKey.append("apimsgid");
        vKeyVals[3].sVal.append(sMsgId);

        // URL-encode URL values
        for (i = 0; i < vKeyVals.size(); i++)
            clickstr::click_string_url_encode(vKeyVals[i].sVal);
    }
    else { // REST
        // example URL:  http://api.clickatell.com/rest/message/47584bae0165fbec57b18bf47895fece
        sPath.append("rest/message/");
        sPath.append(sMsgId);
    }

    // performs formatting of API call and then executes the request
    LocalApiCommandExecute(sPath, vKeyVals, vMsisdns);

    return sClickatellResponse;
}

/*
 * Function:  SmsCoverageGet
 * Info:      Enables users to check Clickatell coverage of a network/number, without sending
 *            a message to that number
 *            Authentication: This function uses username/password to authenticate for the
 *                            HTTP API, however it is also possible in a session to use a session ID to
 *                            authenticate with the HTTP API. See the Clickatell API docs at
 *                            www.clickatell.com for more details.
 *            URL Encoding: For the HTTP API, The URL parameter values are URL-encoded in
 *                          this function.
 * Inputs:    sMsisdn - single msisdn for which Clickatell will verify has supported coverage
 * Return:    Prefix is currently supported or prefix is not supported by Clickatell.
 */
std::string ClickatellSms::SmsCoverageGet(const std::string &sMsisdn)
{
    unsigned int i = 0;
    std::string sPath;                 // api call path designator
    std::vector<ClickKeyVal> vKeyVals; // array of key/val structures, excluding "to" field
    std::vector<std::string> vMsisdns; // empty vector to pass through

    eRequest = CLICK_CURL_GET;

    if (CLICK_STR_INVALID(sMsisdn)) {
        oLocalDebug.Print("%s ERROR: invalid parameter!\n", __func__);
        return sClickatellResponse;
    }

    if (eUserApiType == CLICK_API_HTTP) {
        sPath.append("utils/routecoverage.php");

        // set URL key/value pairs
        for (i = 0; i < 4; i++)
            vKeyVals.push_back(ClickKeyVal());
        vKeyVals[0].sKey.append("user");
        vKeyVals[0].sVal.append(oUserCred.sUsername);
        vKeyVals[1].sKey.append("password");
        vKeyVals[1].sVal.append(oUserCred.sPassword);
        vKeyVals[2].sKey.append("api_id");
        vKeyVals[2].sVal.append(sUserApiId);
        vKeyVals[3].sKey.append("msisdn");
        vKeyVals[3].sVal.append(sMsisdn);

        // URL-encode URL values
        for (i = 0; i < vKeyVals.size(); i++)
            clickstr::click_string_url_encode(vKeyVals[i].sVal);
    }
    else { // REST
        // example URL:  https://api.clickatell.com/rest/coverage/27999123456
        sPath.append("rest/coverage/");
        sPath.append(sMsisdn);
    }

    // performs formatting of API call and then executes the request
    LocalApiCommandExecute(sPath, vKeyVals, vMsisdns);

    return sClickatellResponse;
}

/*
 * Function:  SmsMessageStop
 * Info:      Attempt to stop the delivery of an SMS message. This command can only stop messages
 *            which may be queued within the Clickatell system and not messages which have already
 *            been delivered to an SMSC.
 *            Authentication: This function uses username/password to authenticate for the
 *                            HTTP API, however it is also possible in a session to use a session ID to
 *                            authenticate with the HTTP API. See the Clickatell API docs at
 *                            www.clickatell.com for more details.
 *            URL Encoding: For the HTTP API, The URL parameter values are URL-encoded in
 *                          this function.
 * Inputs:    API Message ID - SMS ID assigned by Clickatell
 * Return:    ID with status or an error number with error description.
 */
std::string ClickatellSms::SmsMessageStop(const std::string &sMsgId)
{
    unsigned int i = 0;
    std::string sPath;                 // api call path designator
    std::vector<ClickKeyVal> vKeyVals; // array of key/val structures, excluding "to" field
    std::vector<std::string> vMsisdns; // empty vector to pass through

    eRequest = (eUserApiType == CLICK_API_HTTP ? CLICK_CURL_GET : CLICK_CURL_DELETE);

    if (CLICK_STR_INVALID(sMsgId)) {
        oLocalDebug.Print("%s ERROR: invalid parameter!\n", __func__);
        return sClickatellResponse;
    }

    if (eUserApiType == CLICK_API_HTTP) {
        sPath.append("http/delmsg.php");

        // set URL key/value pairs
        for (i = 0; i < 4; i++)
            vKeyVals.push_back(ClickKeyVal());
        vKeyVals[0].sKey.append("user");
        vKeyVals[0].sVal.append(oUserCred.sUsername);
        vKeyVals[1].sKey.append("password");
        vKeyVals[1].sVal.append(oUserCred.sPassword);
        vKeyVals[2].sKey.append("api_id");
        vKeyVals[2].sVal.append(sUserApiId);
        vKeyVals[3].sKey.append("apimsgid");
        vKeyVals[3].sVal.append(sMsgId);

        // URL-encode URL values
        for (i = 0; i < vKeyVals.size(); i++)
            clickstr::click_string_url_encode(vKeyVals[i].sVal);
    }
    else { // REST
        // example URL:  https://api.clickatell.com/rest/message/47584bae0165fbec57b18bf47895fece
        sPath.append("rest/message/");
        sPath.append(sMsgId);
    }

    // performs formatting of API call and then executes the request
    LocalApiCommandExecute(sPath, vKeyVals, vMsisdns);

    return sClickatellResponse;
}

/*
 * Function:  ClickatellSms::SetResponse
 * Info:      Function to set the 'sClickatellResponse' class member. Can be called for
 *            example from an external callback function.
 * Inputs:    cstr - character string designating new response to set to private class
 *            member.
 * Return:    void
 */
inline void ClickatellSms::SetResponse(char *chStr)
{
    sClickatellResponse.clear();
    sClickatellResponse.assign(chStr);
}
