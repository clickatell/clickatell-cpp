/*
 * test_clickatell_sms.cpp
 *
 * Contains sample code demonstrating how to utilize Clickatell HTTP and REST APIs.
 * This file calls public functions from the Clickatell SMS library.
 * This file executes common API Calls for the following API types:
 * - HTTP API using username/password as authentication
 * - REST API using a Clickatell REST API Key as authentication
 *
 *  Martin Beyers <martin.beyers@clickatell.com>
 */

#include <string>
#include <vector>
#include <iostream>

#include "clickatell_sms/clickatell_debug.hpp"
#include "clickatell_sms/clickatell_string.hpp"
#include "clickatell_sms/clickatell_sms.hpp"

/* ----------------------------------------------------------------------------- *
 * Input configuration values                                                    *
 * NOTE: Please modify these values and replace them with your own credentials.  *
 * ----------------------------------------------------------------------------- */

// insert your HTTP API credentials here
#define CFG_HTTP_USERNAME           "myusernamehere" // insert your Clickatell account username here
#define CFG_HTTP_PASSWORD           "mypasswordhere" // insert your Clickatell account password here
#define CFG_HTTP_APIID              "3518209"        // insert your Clickatell HTTP API ID here

// insert your REST API credentials here
#define CFG_REST_APIKEY             "uJqYpaWlUNPUhEDsuptRJCk5nGZD.Fwx8vHQOUjoTXTdFghXERUsZDvoK1SiF" // insert your Clickatell REST API Key here
#define CFG_REST_APIID              "2517153" // insert your Clickatell REST API ID here

// insert your destination addresses here
#define CFG_SAMPLE_MSISDN1          "2991000000" // insert your first desired destination mobile number here
#define CFG_SAMPLE_MSISDN2          "2991000001" // insert your second desired destination mobile number here
#define CFG_SAMPLE_MSISDN3          "2991000002" // insert your third desired destination mobile number here
#define CFG_SAMPLE_COVERAGE_MSISDN  "2991000000" // insert your coverage destination mobile number here

// insert your SMS message text here
#define CFG_SAMPLE_MSG_TEXT         "This is example SMS message text; -> insert your own text here."

// timeout values - these can be modified or left as is
#define CFG_APICALL_TIMEOUT         5 // Config: Maximum time in seconds (long value) for API call to take
#define CFG_APICALL_CONNECT_TIMEOUT 2 // Config: maximum time in seconds (long value) that API call takes to connect to Clickatell server

/* ----------------------------------------------------------------------------- *
 * Fixed Macros                                                                  *
 * ----------------------------------------------------------------------------- */

// common print functions
#define PRINT_MAIN_TEST_SEPARATOR   { std::cout << "\n===============================================================================================\n"; }
#define PRINT_SUB_TEST_SEPARATOR    { std::cout << "\n\n"; }

/* ----------------------------------------------------------------------------- *
 * Forward declarations                                                          *
 * ----------------------------------------------------------------------------- */

void run_common_tests(eClickApi eApiType);
void run_common_api_calls(eClickApi eApiType, ClickatellSms &oClickSms);

/* ----------------------------------------------------------------------------- *
 * Local function definitions                                                    *
 * ----------------------------------------------------------------------------- */

/*
 * Function:  run_common_tests
 * Info:      Runs series of API calls.
 *            The try block ensures exceptions thrown from the ClickatellSms object's
 *            constructor are handled and that no further processing is applied to the
 *            object if said constructor did fail.
 * Inputs:    API type
 * Return:    void
 */
void run_common_tests(eClickApi eApiType)
{
    PRINT_MAIN_TEST_SEPARATOR

    int arg = eApiType; // remove 'enum not handled in switch' warnings - enum is list of names so must assign to a variable

    switch (arg) {
        case CLICK_API_HTTP: {
            std::cout << "Executing HTTP API Tests with username/password as authentication method\n\n";

            std::string sHttpUser(CFG_HTTP_USERNAME);
            std::string sHttpPass(CFG_HTTP_PASSWORD);
            std::string sHttpApiId(CFG_HTTP_APIID);

            // catch any exceptions thrown from constructor initialization
            try {
                ClickatellSms oClickSms(CLICK_DEBUG_ON,
                                        eApiType,
                                        sHttpUser,sHttpPass,
                                        sHttpApiId,
                                        CFG_APICALL_TIMEOUT,
                                        CFG_APICALL_CONNECT_TIMEOUT);

                run_common_api_calls(eApiType, oClickSms);
            }
            catch (std::string sErr) {
                std::cout << "Exception occurred when constructing ClickatellSms object. Exception: " << sErr << '\n';
            }
            break;
        }
        case CLICK_API_REST: {
            std::cout << "Executing REST API Tests with api_key as authentication method\n\n";

            std::string restApiId(CFG_REST_APIID);
            std::string sRestApiKey(CFG_REST_APIKEY);

            // catch any exceptions thrown from constructor initialization
            try {
                ClickatellSms oClickSms(CLICK_DEBUG_ON,
                                        eApiType,
                                        sRestApiKey,
                                        restApiId,
                                        CFG_APICALL_TIMEOUT,
                                        CFG_APICALL_CONNECT_TIMEOUT);

                run_common_api_calls(eApiType, oClickSms);
            }
            catch (std::string sErr) {
                std::cout << "Exception occurred when constructing ClickatellSms object. Exception: " << sErr << '\n';
            }
            break;
        }
        default: {
            std::cout << "ERROR: Invalid API type selected!\n";
            break;
        }
    }
}

/*
 * Function:  run_common_api_calls
 * Info:      Executes common API calls.
 *            Ensure oClickSms constructor was called prior to this function.
 * Inputs:    eApiType  - API call type
 *            oClickSms - ClickatellSms object instance
 * Return:    void
 */
void run_common_api_calls(eClickApi eApiType, ClickatellSms &oClickSms)
{
    std::string sResponse;
    std::string msgText(CFG_SAMPLE_MSG_TEXT);

    // ----------------------------------------------------------------------------------------
    // send a message to multiple mobile handsets
    // uncomment this if you wish to send a message to multiple handsets.
    // ----------------------------------------------------------------------------------------
/*    std::cout << "[" <<  (eApiType == CLICK_API_HTTP ? "HTTP" : "REST") << ": Send multiple SMSes]\n\n";
    std::vector<std::string> vMsisdnsMultiple;
    vMsisdnsMultiple.push_back(CFG_SAMPLE_MSISDN1);
    vMsisdnsMultiple.push_back(CFG_SAMPLE_MSISDN2);
    vMsisdnsMultiple.push_back(CFG_SAMPLE_MSISDN3);

    std::string msgIds = oClickSms.SmsMessageSend(msgText, vMsisdnsMultiple);
    std::cout << oClickSms;
    PRINT_SUB_TEST_SEPARATOR
*/
    // ----------------------------------------------------------------------------------------
    // send a message to one handset
    // ----------------------------------------------------------------------------------------
    std::cout << "[" <<  (eApiType == CLICK_API_HTTP ? "HTTP" : "REST") << ": Send SMS]\n\n";
    std::vector<std::string> vMsisdnsSingle;
    vMsisdnsSingle.push_back(CFG_SAMPLE_MSISDN1);

    std::string msgIdResponse = oClickSms.SmsMessageSend(msgText, vMsisdnsSingle);
    std::cout << oClickSms;
    PRINT_SUB_TEST_SEPARATOR

    // retrieve apiMessageId field from 'sResponse' string
    std::string msgId;
    if (eApiType == CLICK_API_HTTP) {
        /* A successful sResponse should look similar to this:   ID: 205e85d0578314037a96175249fc6a2b
         * which means we need to remove the 'ID:' prefix text and space character from the
         * 'sResponse' string
         */
        clickstr::click_string_trim_prefix(msgIdResponse, 4);
        msgId.append(msgIdResponse);
    }
    else { // REST
        size_t iPosStart = 0, iPosEnd = 0;

        /* A successful JSON response should look similar to this:
         *     {"data":{"message":[{"accepted":true,"to":"2771000000","apiMessageId":"77a4a70428f984d9741001e6f17d02b4"}]}}
         * We need to search for the apiMessageId field, and then retrieve its value.
         */
        if ((iPosStart = msgIdResponse.find("apiMessageId", 0)) != std::string::npos) {
            if ((iPosEnd = msgIdResponse.find("}", (iPosStart + 14))) != std::string::npos) {
                // obtain size of message ID
                size_t iMsgId_size = (iPosEnd - 1 - (iPosStart + 15));

                // retrieve message ID substring
                msgId = msgIdResponse.substr((iPosStart + 15), iMsgId_size);
            }
            else
                msgId = "MSG NOT FOUND";
        }
        else
            msgId = "MSG NOT FOUND";
    }

    // ----------------------------------------------------------------------------------------
    // get sms status (using message id received from 'send message' call)
    // ----------------------------------------------------------------------------------------
    std::cout << "[" <<  (eApiType == CLICK_API_HTTP ? "HTTP" : "REST") << ": Get SMS status]\n\n";
    sResponse = oClickSms.SmsStatusGet(msgId);
    std::cout << oClickSms;
    sResponse.clear();
    PRINT_SUB_TEST_SEPARATOR

    // ----------------------------------------------------------------------------------------
    // get user account balance
    // ----------------------------------------------------------------------------------------
    std::cout << "[" <<  (eApiType == CLICK_API_HTTP ? "HTTP" : "REST") << ": Get account balance]\n\n";
    sResponse = oClickSms.SmsBalanceGet();
    std::cout << oClickSms;
    sResponse.clear();
    PRINT_SUB_TEST_SEPARATOR

    // ----------------------------------------------------------------------------------------
    // get sms charge (using message id received from 'send message' call)
    // ----------------------------------------------------------------------------------------
    std::cout << "[" <<  (eApiType == CLICK_API_HTTP ? "HTTP" : "REST") << ": Get SMS charge]\n\n";
    sResponse = oClickSms.SmsChargeGet(msgId);
    std::cout << oClickSms;
    sResponse.clear();
    PRINT_SUB_TEST_SEPARATOR

    // ----------------------------------------------------------------------------------------
    // get coverage of route or number (using message id received from 'send message' call)
    // ----------------------------------------------------------------------------------------
    std::cout << "[" <<  (eApiType == CLICK_API_HTTP ? "HTTP" : "REST") << ": Get coverage]\n\n";
    std::string coverage_msisdn(CFG_SAMPLE_COVERAGE_MSISDN);
    sResponse = oClickSms.SmsCoverageGet(coverage_msisdn);
    std::cout << oClickSms;
    sResponse.clear();
    PRINT_SUB_TEST_SEPARATOR

    // ----------------------------------------------------------------------------------------
    // stop delivery of a message coverage (using message id received from 'send message' call)
    // ----------------------------------------------------------------------------------------
    std::cout << "[" <<  (eApiType == CLICK_API_HTTP ? "HTTP" : "REST") << ": Stop an SMS]\n\n";
    sResponse = oClickSms.SmsMessageStop(msgId);
    std::cout << oClickSms;
    sResponse.clear();
    PRINT_SUB_TEST_SEPARATOR
}

/* ----------------------------------------------------------------------------- *
 * Main function which tests the Clickatell SMS library                          *
 * ----------------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
    std::cout << "========= Clickatell SMS module test application =========" << std::endl;

    // initialize cURL library
    if (curl_global_init(CURL_GLOBAL_ALL) != 0) {
        std::cout << "curl_global_init failed!" << std::endl;
    }
    else {
        // run Clickatell HTTP common API calls (with username/password as authentication)
        run_common_tests(CLICK_API_HTTP);

        // run Clickatell REST common API calls (with REST api_key as authentication)
        run_common_tests(CLICK_API_REST);

        // shutdown cURL library
        curl_global_cleanup();
    }

    return 0;
}
