Clickatell C++ library that integrates with Clickatell HTTP and REST APIs
==============================================================

You can see our other libraries and more documentation at the [Clickatell APIs and Libraries Project](http://clickatell.github.io/).

------------------------------------

Description of contents:
------------------------
About:            
This package allows one to build a Clickatell SMS library that can be linked to your C++ application. Said library provides public functions which make calls to Clickatell's HTTP and REST APIs, allowing one to send SMSes, query their user credit balance, query an SMS status, query the cost of an SMS,check SMS route coverage and stop an SMS.

The package also contains a simple C++ test application that when compiled, links with the Clickatell SMS library. This test application indicates how to test SMS functionality of the Clickatell SMS library.

Makefiles:    
2 Makefiles - one builds the Clickatell SMS library, and the other builds the test application

Test application:    
test_clickatell_sms (test binary that calls public functions from the clickatell_sms.a library)

Library:    
clickatell_sms.a    (Library that can be linked with your C++ application)

Author:    
Martin Beyers - martin.beyers@clickatell.com

Company:    
Clickatell

Date:    
2014-12-30

Environment:
------------
This readme assumes a Linux environment is used to compile the library and test program. However, the code is cross-platform 
compatible and so the steps in this readme still pertain if your application runs on another OS (i.e. Win64).

File Listing:
-------------
    ./readme.txt                                    : Readme file
    ./src/clickatell_sms/clickatell_debug.hpp       : Basic debug header file
    ./src/clickatell_sms/clickatell_debug.cpp       : Basic debug source file
    ./src/clickatell_sms/clickatell_string.hpp      : Basic string functions header file
    ./src/clickatell_sms/clickatell_string.cpp      : Basic string functions source file
    ./src/clickatell_sms/Makefile                   : Makefile used to build Clickatell SMS library
    ./src/clickatell_sms/make_lib.sh                : shortcut script to build Makefile
    ./src/clickatell_sms/clickatell_sms.hpp         : Clickatell SMS library header file
    ./src/clickatell_sms/clickatell_sms.cpp         : Clickatell SMS library source file
    ./src/make_test_application.sh                  : shortcut script to build Makefile
    ./src/Makefile                                  : Makefile used to build the simple test application
    ./src/test_clickatell_sms.cpp                   : Simple test application which links with the Clickatell 
                                                      SMS library (clickatell_sms.a). This test application 
                                                      when run will cycle through the Clickatell SMS library 
                                                      public functions, testing common API calls from the Clickatell 
                                                      HTTP and REST APIs.
                            
                           
Request Format:
---------------
HTTP: Requests are performed using GET operations. API parameters are passed as Key/Value pairs appended to 
      the https://api.clickatell.com/###.php base URL.

REST: The Clickatell REST API does support XML format for transmission/reception, but in this library for 
      REST we transmit post data in JSON format and receive Clickatell response data in JSON format. 

Shared Library:
---------------
The Clickatell SMS library integrates with libcurl (free client-side URL transfer library).
Libcurl is cross-platform, and the relevant libcurl resource can be downloaded from 
http://curl.haxx.se/download.html. 

You will need to ensure that the correct version of cURL is installed on your platform.
For Linux environments, install the 'curl-devel' package. 
For Windows, download the relevant libcurl resource from http://curl.haxx.se/download.html.

Steps on how to use this sample code:
---------------
### Building the Clickatell SMS library:
1. Ensure the cURL package is installed in your environment. See 'Shared Library' above for 
      more details.
2. Download this package from github to your local machine.
3. Navigate to clickatell_sms/ folder:

        cd src/clickatell_sms/

4. Build the clickatell_sms library by running 'make':

        make

Once the clickatell_sms.a library is built, it should exist in the following folder:      
src/lib/libclickatell_sms.a
  
### Configuring the Test Application:
1. Ensure that you have signed up for an HTTP or REST (or both) Clickatell product. You will 
   need the login credentials to send SMS messages with the Clickatell SMS library.
   The login credentials are explained in step 2.
2. Edit file src/test_clickatell_sms.cpp, and under section "Input configuration values", 
   please insert your own Clickatell HTTP/REST API login credentials. For the destination 
   number CFG_SAMPLE_MSISDN1, assign this to the destination number (in international number 
   format) you would like to send an SMS to.
      * If using HTTP:
        * CFG_HTTP_USERNAME: assign this to your Clickatell HTTP API username
        * CFG_HTTP_PASSWORD: assign this to your Clickatell HTTP API password
        * CFG_HTTP_APIID:    assign this to your Clickatell HTTP API number
      * If using REST: 
        * CFG_REST_APIKEY:   assign this to your Clickatell REST API Key 
        * CFG_REST_APIID:    assign this to your Clickatell REST API number          
    
### Building the Test Application:
1. Navigate to src folder (which contains the script file 'make_test_application.sh')    

          cd src

2. Build the test application by running 'make':

          make

      The Makefile will build the following simple test application:   

          test_clickatell_sms
        
### Running the Test Application:
1. Note that the test_clickatell_sms binary application should be run without parameters.
   Run the simple test application by executing this command:

          ./test_clickatell_sms
     
