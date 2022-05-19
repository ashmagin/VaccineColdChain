/*
 * Lab-Project-coreMQTT-Agent 201206
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 */

#ifndef DEMO_CONFIG_H
#define DEMO_CONFIG_H

#include "logging_levels.h"

#ifndef LOG_LEVEL
    #define LOG_LEVEL                               LOG_DEBUG
#endif

#include "logging.h"

/************ End of logging configuration ****************/



/* Constants that select which demos to build into the project:
 * Set the following to 1 to include the demo in the build, or 0 to exclude the
 * demo. */
#define democonfigCREATE_LARGE_MESSAGE_SUB_PUB_TASK        0
#define democonfigLARGE_MESSAGE_SUB_PUB_TASK_STACK_SIZE    ( configMINIMAL_STACK_SIZE )

#define democonfigNUM_SIMPLE_SUB_PUB_TASKS_TO_CREATE       0
#define democonfigSIMPLE_SUB_PUB_TASK_STACK_SIZE           ( 2048 )

#define democonfigCREATE_CODE_SIGNING_OTA_DEMO             0
#define democonfigCODE_SIGNING_OTA_TASK_STACK_SIZE         ( configMINIMAL_STACK_SIZE )


#define democonfigCREATE_DEFENDER_DEMO                     0
#define democonfigDEFENDER_TASK_STACK_SIZE                 ( configMINIMAL_STACK_SIZE )

#define democonfigCREATE_SHADOW_DEMO                       0
#define democonfigSHADOW_TASK_STACK_SIZE                   ( configMINIMAL_STACK_SIZE )

/**
 * @brief Server's root CA certificate.
 *
 * For AWS IoT MQTT broker, this certificate is used to identify the AWS IoT
 * server and is publicly available. Refer to the AWS documentation available
 * in the link below.
 * https://docs.aws.amazon.com/iot/latest/developerguide/server-authentication.html#server-authentication-certs
 *
 * @note This certificate should be PEM-encoded.
 *
 * @note If you would like to setup an MQTT broker for running this demo,
 * please see `mqtt_broker_setup.txt`.
 *
 * Must include the PEM header and footer:
 * "-----BEGIN CERTIFICATE-----\n"\
 * "...base64 data...\n"\
 * "-----END CERTIFICATE-----\n"
 *
 * #define democonfigROOT_CA_PEM    "...insert here..."
 */
#define democonfigROOT_CA_PEM                              "-----BEGIN CERTIFICATE-----\n\
MIIBtjCCAVugAwIBAgITBmyf1XSXNmY/Owua2eiedgPySjAKBggqhkjOPQQDAjA5\n\
MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6b24g\n\
Um9vdCBDQSAzMB4XDTE1MDUyNjAwMDAwMFoXDTQwMDUyNjAwMDAwMFowOTELMAkG\n\
A1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJvb3Qg\n\
Q0EgMzBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABCmXp8ZBf8ANm+gBG1bG8lKl\n\
ui2yEujSLtf6ycXYqm0fc4E7O5hrOXwzpcVOho6AF2hiRVd9RFgdszflZwjrZt6j\n\
QjBAMA8GA1UdEwEB/wQFMAMBAf8wDgYDVR0PAQH/BAQDAgGGMB0GA1UdDgQWBBSr\n\
ttvXBp43rDCGB5Fwx5zEGbF4wDAKBggqhkjOPQQDAgNJADBGAiEA4IWSoxe3jfkr\n\
BqWTrBqYaGFy+uGh0PsceGCmQ5nFuMQCIQCcAu/xlJyzlvnrxir4tiz+OpAUFteM\n\
YyRIHN8wfdVoOw==\n\
-----END CERTIFICATE-----"

/**
 * @brief Client certificate.
 *
 * For AWS IoT MQTT broker, refer to the AWS documentation below for details
 * regarding client authentication.
 * https://docs.aws.amazon.com/iot/latest/developerguide/client-authentication.html
 *
 * @note This certificate should be PEM-encoded.
 *
 * Must include the PEM header and footer:
 * "-----BEGIN CERTIFICATE-----\n"\
 * "...base64 data...\n"\
 * "-----END CERTIFICATE-----\n"
 *
 * #define democonfigCLIENT_CERTIFICATE_PEM    "...insert here..."
 */
#define democonfigCLIENT_CERTIFICATE_PEM \
"-----BEGIN CERTIFICATE-----\n\
MIIDWTCCAkGgAwIBAgIUQqg2Qihf76A9Va14rFokqkmkVjwwDQYJKoZIhvcNAQEL\n\
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n\
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTIxMTAwNDA5MDI0\n\
MVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n\
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAPE3KFnC07yzowRIA3aT\n\
H7OBIn/xN4kCjQIbByPp+7ygBI0njG71yaiUEWNxDM//g5cy/Q5oCqRZRTQsPfV7\n\
z3IplqmpzMECBOFmXAoFjjrgZqcDo2X4Ij93jSkviUk0z5nf8YIxZ/JgtSa9KOGl\n\
WiCprEihgL/zivbN56qOcPdWMuXRrPTyOv7z6z7L6ATPTErkejvfWtYrGqVRKf7R\n\
JvkpuBzIaxql4Sztq5+MH8CVw6sS01dQeeO5T6POR/4gMsQO8Da7C90FyHN2go6k\n\
U7Q6Q8orKke797R1i0jRlCFWioYCxUoZ8MwTWuBkbSizJXCeplwdFbvOXM1tiK+O\n\
9p8CAwEAAaNgMF4wHwYDVR0jBBgwFoAUdvowZ2l6/BwUbQKKMbNqXfBfP34wHQYD\n\
VR0OBBYEFDO7C9pbu/HEmEA4Jym2n/itX2G5MAwGA1UdEwEB/wQCMAAwDgYDVR0P\n\
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBGQLC+C8fHoAyHwLyu1WixIlEV\n\
3LJrDxURb+xj4XsLBoIrtc2hfOVAdd0knUhQKwFYePCRqUtwjTNpIuOv25qczcSY\n\
W2sWGp9eIFqSqsUEF5c6AJrX6HYF3+P+8B1tMbSo/DKrbGYPSc6ssGLM9rUunyfB\n\
Jo4bM9Hy/tpqHfcanH4v3IhRgu7xierVxwSmRBd4114+5KbwMO8j1iBtm6kfUKwb\n\
Cqcd0/iHw/WP4vyLaKrns0Ao1SMJXBRih/UwTUd1LJSeWhDAGpPYytTGKItmdTcl\n\
irN/8q8NtRYFN4LuKYMjEsobWM8jgje04AL/ObPnPd8VuyfBW/mgGCfWIJ/e\n\
-----END CERTIFICATE-----\n"



/**
 * @brief Client's private key.
 *
 *!!! Please note pasting a key into the header file in this manner is for
 *!!! convenience of demonstration only and should not be done in production.
 *!!! Never paste a production private key here!.  Production devices should
 *!!! store keys securely, such as within a secure element.  Additionally,
 *!!! we provide the corePKCS library that further enhances security by
 *!!! enabling securely stored keys to be used without exposing them to
 *!!! software.
 *
 * For AWS IoT MQTT broker, refer to the AWS documentation below for details
 * regarding clientauthentication.
 * https://docs.aws.amazon.com/iot/latest/developerguide/client-authentication.html
 *
 * @note This private key should be PEM-encoded.
 *
 * Must include the PEM header and footer:
 * "-----BEGIN RSA PRIVATE KEY-----\n"\
 * "...base64 data...\n"\
 * "-----END RSA PRIVATE KEY-----\n"
 *
 * #define democonfigCLIENT_PRIVATE_KEY_PEM    "...insert here..."
 */
#define democonfigCLIENT_PRIVATE_KEY_PEM \
"-----BEGIN RSA PRIVATE KEY-----\n\
MIIEowIBAAKCAQEA8TcoWcLTvLOjBEgDdpMfs4Eif/E3iQKNAhsHI+n7vKAEjSeM\n\
bvXJqJQRY3EMz/+DlzL9DmgKpFlFNCw99XvPcimWqanMwQIE4WZcCgWOOuBmpwOj\n\
ZfgiP3eNKS+JSTTPmd/xgjFn8mC1Jr0o4aVaIKmsSKGAv/OK9s3nqo5w91Yy5dGs\n\
9PI6/vPrPsvoBM9MSuR6O99a1isapVEp/tEm+Sm4HMhrGqXhLO2rn4wfwJXDqxLT\n\
V1B547lPo85H/iAyxA7wNrsL3QXIc3aCjqRTtDpDyisqR7v3tHWLSNGUIVaKhgLF\n\
ShnwzBNa4GRtKLMlcJ6mXB0Vu85czW2Ir472nwIDAQABAoIBADdjrA9YjphxQ4iz\n\
JKBwzkHA8HQiptb+T/Tg0fXsP9Mjh4gGQdgk9oaQYaHNDt0gVk489iD9wRvWJ+8A\n\
DcY2BP+5DxCsngjf9tDvWUkv12Yrq0bErhu3K94ZDMAdW1AFIXq7EtELmUvt6wsX\n\
OrD98EE51QbklxT8zrc3tu/SP5x7q5QdFz3R3aCsZ5UAMNzwA2/3P7cOCIx0h/rb\n\
ja7hHyOQLmM/zB91/SyDZm9hpgtGvQ56306TTAEJqOrtIR34+mzEpNYI+VpPOC3z\n\
zVgpdtrYiUw9F4ILRMAP8QsPhCpNyksHIOZ30OoZen7sanyoZJ/PksSmkoqMp3pC\n\
NTwVkRECgYEA/bSBVXJeeyTAopIVF37vVrP++o/uHyzqHYDcL+EEKbxvFjyMNduY\n\
dKuN4rGK+J1KGsmyAvarB+LxsA12iJV8hkuxMy3hq8vAeBUQJcbntM0TDjbV1loh\n\
2KpLWYNvWKYCMfoQ8goBb8msGuZQ/AU9vdAyNwdK+zzy/lkXCknw6rcCgYEA82W7\n\
C+FYGdT12jtSbzNFY99rjTUSs1wkJcrVWkovnoZY/U2hYjgghDowoYkk796+mRci\n\
a7/QJnwvGkdB4z/YWa511TZ54qgmQqemYxvk2gc6vE6nI3YRoLFSM+W6tAaD2QR+\n\
vxCoFo4tRBYyYM59f015XtgBJyZT7RgO2gOEi1kCgYAM6wj+2dD3wjpQzNFoHd1/\n\
87BS/8snKTlOgUY05ec5XM/nOPQGtgTARA/4cAPcft3MGWMIBZlV0Hdv+G+Fqt7p\n\
U6dVLMvl3H0TYDcvRT59FUgYHDCtNm+swZcOmsOrpuBLvSjpuhqyn3k08W9ENdwb\n\
8yLrbGKkAkl5K/4hVvx+ZwKBgC/Uyxg/lX+mERZs1/tQWQ+SZWFfj6g9sEDZBmpy\n\
7764ah6FNfTGUOWLRelIuUCGRnBxUmtYS4B0oREJ9dHBxzOAjFl1FijLoJEyvwj0\n\
/1k1WH6O+luGpVot3piiSbl9GkX4kyvbCx07nXum3QmXKqRsxjA0RK2743xRYQHI\n\
9TApAoGBANZIPlOFPcCmgWZgXcbTRmf5ULX8svOAfSWwkBGMktVwN0p7wcYhaxui\n\
+8B+P1BZG0zXF/AwNz+Xq+YsFdXVwH/Y11KOy+xhP3/cwJaKU9YnLN7o2ukaRHff\n\
/ymbBLdvV+3sC2GByiOVL8bO+Su5kIP1l03m5ekP3PK5+cplm7qi\n\
-----END RSA PRIVATE KEY-----\n"

/**
 * @brief An option to disable Server Name Indication.
 *
 * @note When using a local Mosquitto server setup, SNI needs to be disabled
 * for an MQTT broker that only has an IP address but no hostname. However,
 * SNI should be enabled whenever possible.
 */
#define democonfigDISABLE_SNI                ( pdFALSE )

/**
 * @brief Configuration that indicates if the demo connection is made to the AWS IoT Core MQTT broker.
 *
 * If username/password based authentication is used, the demo will use appropriate TLS ALPN and
 * SNI configurations as required for the Custom Authentication feature of AWS IoT.
 * For more information, refer to the following documentation:
 * https://docs.aws.amazon.com/iot/latest/developerguide/custom-auth.html#custom-auth-mqtt
 *
 * #define democonfigUSE_AWS_IOT_CORE_BROKER    ( 1 )
 */
#define democonfigUSE_AWS_IOT_CORE_BROKER    ( 1 )

/**
 * @brief The password value for authenticating client to the MQTT broker when
 * username/password based client authentication is used.
 *
 * For AWS IoT MQTT broker, refer to the AWS IoT documentation below for
 * details regarding client authentication with a username and password.
 * https://docs.aws.amazon.com/iot/latest/developerguide/custom-authentication.html
 * An authorizer setup needs to be done, as mentioned in the above link, to use
 * username/password based client authentication.
 *
 * #define democonfigCLIENT_PASSWORD    "...insert here..."
 */

/**
 * @brief The name of the operating system that the application is running on.
 * The current value is given as an example. Please update for your specific
 * operating system.
 */
#define democonfigOS_NAME                   "FreeRTOS"

/**
 * @brief The version of the operating system that the application is running
 * on. The current value is given as an example. Please update for your specific
 * operating system version.
 */
#define democonfigOS_VERSION                tskKERNEL_VERSION_NUMBER

/**
 * @brief The name of the hardware platform the application is running on. The
 * current value is given as an example. Please update for your specific
 * hardware platform.
 */
#define democonfigHARDWARE_PLATFORM_NAME    "STM32U5 IoT Discovery"

/**
 * @brief The name of the MQTT library used and its version, following an "@"
 * symbol.
 */
#define democonfigMQTT_LIB                  "core-mqtt@1.0.0"

/**
 * @brief Whether to use mutual authentication. If this macro is not set to 1
 * or not defined, then plaintext TCP will be used instead of TLS over TCP.
 */
#define democonfigUSE_TLS                   1

/**
 * @brief Set the stack size of the main demo task.
 *
 * In the Windows port, this stack only holds a structure. The actual
 * stack is created by an operating system thread.
 */
#define democonfigDEMO_STACKSIZE            configMINIMAL_STACK_SIZE

/**********************************************************************************
* Error checks and derived values only below here - do not edit below here. -----*
**********************************************************************************/


#ifndef democonfigROOT_CA_PEM
    #error "Please define Root CA certificate of the MQTT broker(democonfigROOT_CA_PEM) in demo_config.h."
#endif

/*
 *!!! Please note democonfigCLIENT_PRIVATE_KEY_PEM in used for
 *!!! convenience of demonstration only.  Production devices should
 *!!! store keys securely, such as within a secure element.
 */

#ifndef democonfigCLIENT_CERTIFICATE_PEM
    #error "Please define client certificate(democonfigCLIENT_CERTIFICATE_PEM) in demo_config.h."
#endif
#ifndef democonfigCLIENT_PRIVATE_KEY_PEM
    #error "Please define client private key(democonfigCLIENT_PRIVATE_KEY_PEM) in demo_config.h."
#endif

#ifndef democonfigMQTT_BROKER_PORT
    #define democonfigMQTT_BROKER_PORT    ( 8883 )
#endif

/**
 * @brief ALPN (Application-Layer Protocol Negotiation) protocol name for AWS IoT MQTT.
 *
 * This will be used if democonfigMQTT_BROKER_PORT is configured as 443 for the AWS IoT MQTT broker.
 * Please see more details about the ALPN protocol for AWS IoT MQTT endpoint
 * in the link below.
 * https://aws.amazon.com/blogs/iot/mqtt-with-tls-client-authentication-on-port-443-why-it-is-useful-and-how-it-works/
 */
#define AWS_IOT_MQTT_ALPN           "\x0ex-amzn-mqtt-ca"

/**
 * @brief This is the ALPN (Application-Layer Protocol Negotiation) string
 * required by AWS IoT for password-based authentication using TCP port 443.
 */
#define AWS_IOT_CUSTOM_AUTH_ALPN    "\x04mqtt"

/**
 * Provide default values for undefined configuration settings.
 */
#ifndef democonfigOS_NAME
    #define democonfigOS_NAME    "FreeRTOS"
#endif

#ifndef democonfigOS_VERSION
    #define democonfigOS_VERSION    tskKERNEL_VERSION_NUMBER
#endif

#ifndef democonfigHARDWARE_PLATFORM_NAME
    #define democonfigHARDWARE_PLATFORM_NAME    "STM32U5 IoT Discovery"
#endif

#ifndef democonfigMQTT_LIB
    #define democonfigMQTT_LIB    "core-mqtt@1.0.0"
#endif

/**
 * @brief The MQTT metrics string expected by AWS IoT.
 */
#define AWS_IOT_METRICS_STRING                                 \
    "?SDK=" democonfigOS_NAME "&Version=" democonfigOS_VERSION \
    "&Platform=" democonfigHARDWARE_PLATFORM_NAME "&MQTTLib=" democonfigMQTT_LIB

/**
 * @brief The length of the MQTT metrics string expected by AWS IoT.
 */
#define AWS_IOT_METRICS_STRING_LENGTH    ( ( uint16_t ) ( sizeof( AWS_IOT_METRICS_STRING ) - 1 ) )

/**
 * @brief Length of client identifier.
 */
#define democonfigCLIENT_IDENTIFIER_LENGTH    ( ( uint16_t ) ( sizeof( democonfigCLIENT_IDENTIFIER ) - 1 ) )

/**
 * @brief Length of MQTT server host name.
 */
#define democonfigBROKER_ENDPOINT_LENGTH      ( ( uint16_t ) ( sizeof( democonfigMQTT_BROKER_ENDPOINT ) - 1 ) )


#endif /* DEMO_CONFIG_H */
