/*
 * Lab-Project-coreMQTT-Agent 201215
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
 */

#include "logging_levels.h"
#define LOG_LEVEL LOG_DEBUG
#include "logging.h"

/* Standard includes. */
#include <string.h>
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "event_groups.h"

#include <kvstore.h>
/* Demo Specific configs. */
#include "demo_config.h"

/* MQTT library includes. */
#include "core_mqtt.h"

/* MQTT agent include. */
#include "core_mqtt_agent.h"

/* MQTT Agent ports. */
#include "freertos_agent_message.h"
#include "freertos_command_pool.h"

/* Exponential backoff retry include. */
#include "backoff_algorithm.h"

/* Subscription manager header include. */
#include "subscription_manager.h"

#include "mbedtls_transport.h"

/*-----------------------------------------------------------*/
extern TransportInterfaceExtended_t xLwipTransportInterface;

/**
 * @brief Timeout for receiving CONNACK after sending an MQTT CONNECT packet.
 * Defined in milliseconds.
 */
#define CONNACK_RECV_TIMEOUT_MS                      ( 2000U )

/**
 * @brief The maximum number of retries for network operation with server.
 */
#define RETRY_MAX_ATTEMPTS                           ( 5U )

/**
 * @brief The maximum back-off delay (in milliseconds) for retrying failed operation
 *  with server.
 */
#define RETRY_MAX_BACKOFF_DELAY_MS                   ( 5000U )

/**
 * @brief The base back-off delay (in milliseconds) to use for network operation retry
 * attempts.
 */
#define RETRY_BACKOFF_BASE_MS                        ( 500U )

/**
 * @brief The maximum time interval in seconds which is allowed to elapse
 *  between two Control Packets.
 *
 *  It is the responsibility of the Client to ensure that the interval between
 *  Control Packets being sent does not exceed the this Keep Alive value. In the
 *  absence of sending any other Control Packets, the Client MUST send a
 *  PINGREQ Packet.
 */
#define mqttexampleKEEP_ALIVE_INTERVAL_SECONDS       ( 1200U )

/**
 * @brief Socket send and receive timeouts to use.
 */
#define mqttexampleTRANSPORT_SEND_RECV_TIMEOUT_MS    ( 2000 )

#define EVENT_BIT_AGENT_READY 0

typedef struct
{
	char * pcMqttEndpointAddress;
	uint32_t ulMqttEndpointLen;
	char * pcMqttClientId;
	uint32_t ulMqttClientIdLen;
	uint32_t ulMqttPort;

} MqttConnectCtx_t;

static MqttConnectCtx_t xConnectCtx = { 0 };

/*-----------------------------------------------------------*/

/**
 * @brief Initializes an MQTT context, including transport interface and
 * network buffer.
 *
 * @return `MQTTSuccess` if the initialization succeeds, else `MQTTBadParameter`.
 */
static MQTTStatus_t prvMQTTInit( void );

/**
 * @brief Sends an MQTT Connect packet over the already connected TCP socket.
 *
 * @param[in] pxMQTTContext MQTT context pointer.
 * @param[in] xCleanSession If a clean session should be established.
 *
 * @return `MQTTSuccess` if connection succeeds, else appropriate error code
 * from MQTT_Connect.
 */
static MQTTStatus_t prvMQTTConnect( bool xCleanSession );

/**
 * @brief Connect a TCP socket to the MQTT broker.
 *
 * @param[in] pxNetworkContext Network context.
 *
 * @return `pdPASS` if connection succeeds, else `pdFAIL`.
 */
static BaseType_t prvSocketConnect( void );

/**
 * @brief Fan out the incoming publishes to the callbacks registered by different
 * tasks. If there are no callbacks registered for the incoming publish, it will be
 * passed to the unsolicited publish handler.
 *
 * @param[in] pMqttAgentContext Agent context.
 * @param[in] packetId Packet ID of publish.
 * @param[in] pxPublishInfo Info of incoming publish.
 */
static void prvIncomingPublishCallback( MQTTAgentContext_t * pMqttAgentContext,
                                        uint16_t packetId,
                                        MQTTPublishInfo_t * pxPublishInfo );

/**
 * @brief Function to attempt to resubscribe to the topics already present in the
 * subscription list.
 *
 * This function will be invoked when this demo requests the broker to
 * reestablish the session and the broker cannot do so. This function will
 * enqueue commands to the MQTT Agent queue and will be processed once the
 * command loop starts.
 *
 * @return `MQTTSuccess` if adding subscribes to the command queue succeeds, else
 * appropriate error code from MQTTAgent_Subscribe.
 * */
static MQTTStatus_t prvHandleResubscribe( void );

/**
 * @brief Passed into MQTTAgent_Subscribe() as the callback to execute when the
 * broker ACKs the SUBSCRIBE message. This callback implementation is used for
 * handling the completion of resubscribes. Any topic filter failed to resubscribe
 * will be removed from the subscription list.
 *
 * See https://freertos.org/mqtt/mqtt-agent-demo.html#example_mqtt_api_call
 *
 * @param[in] pxCommandContext Context of the initial command.
 * @param[in] pxReturnInfo The result of the command.
 */
static void prvSubscriptionCommandCallback( MQTTAgentCommandContext_t * pxCommandContext,
                                            MQTTAgentReturnInfo_t * pxReturnInfo );

/**
 * @brief Task used to run the MQTT agent.  In this example the first task that
 * is created is responsible for creating all the other demo tasks.  Then,
 * rather than create prvMQTTAgentTask() as a separate task, it simply calls
 * prvMQTTAgentTask() to become the agent task itself.
 *
 * This task calls MQTTAgent_CommandLoop() in a loop, until MQTTAgent_Terminate()
 * is called. If an error occurs in the command loop, then it will reconnect the
 * TCP and MQTT connections.
 *
 * @param[in] pvParameters Parameters as passed at the time of task creation. Not
 * used in this example.
 */
static void prvMQTTAgentTask( void * pvParameters );

/**
 * @brief The timer query function provided to the MQTT context.
 *
 * @return Time in milliseconds.
 */
static uint32_t prvGetTimeMs( void );


/*
 * Functions that start the tasks demonstrated by this project.
 */

extern void vStartOTACodeSigningDemo( configSTACK_DEPTH_TYPE uxStackSize,
                                      UBaseType_t uxPriority );
extern void vSuspendOTACodeSigningDemo( void );
extern void vResumeOTACodeSigningDemo( void );
extern bool vOTAProcessMessage( void * pvIncomingPublishCallbackContext,
                                MQTTPublishInfo_t * pxPublishInfo );

extern void vStartDefenderDemo( configSTACK_DEPTH_TYPE uxStackSize,
                                UBaseType_t uxPriority );

extern void vStartShadowDemo( configSTACK_DEPTH_TYPE uxStackSize,
                              UBaseType_t uxPriority );
/*-----------------------------------------------------------*/

/**
 * @brief The network context used by the MQTT library transport interface.
 * See https://www.freertos.org/network-interface.html
 */
static NetworkContext_t * pxNetworkContext = NULL;

/**
 * @brief Global entry time into the application to use as a reference timestamp
 * in the #prvGetTimeMs function. #prvGetTimeMs will always return the difference
 * between the current time and the global entry time. This will reduce the chances
 * of overflow for the 32 bit unsigned integer used for holding the timestamp.
 */
static uint32_t ulGlobalEntryTimeMs;

MQTTAgentContext_t xGlobalMqttAgentContext;

static uint8_t xNetworkBuffer[ MQTT_AGENT_NETWORK_BUFFER_SIZE ];

static MQTTAgentMessageContext_t xCommandQueue;

/**
 * @brief The global array of subscription elements.
 *
 * @note No thread safety is required to this array, since the updates the array
 * elements are done only from one task at a time. The subscription manager
 * implementation expects that the array of the subscription elements used for
 * storing subscriptions to be initialized to 0. As this is a global array, it
 * will be initialized to 0 by default.
 */
SubscriptionElement_t xGlobalSubscriptionList[ SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS ];

static EventGroupHandle_t xAgentEvents = NULL;

/*-----------------------------------------------------------*/

/*
 * @brief Create the task that demonstrates the MQTT Connection sharing demo.
 */
void vStartMQTTAgentDemo( void )
{
    /* prvConnectAndCreateDemoTasks() connects to the MQTT broker, creates the
     * tasks that will interact with the broker via the MQTT agent, then turns
     * itself into the MQTT agent task. */
    xTaskCreate( prvMQTTAgentTask, /* Function that implements the task. */
                 "MQTTAgent",      /* Text name for the task - only used for debugging. */
                 4096,             /* Size of stack (in words, not bytes) to allocate for the task. */
                 NULL,                         /* Optional - task parameter - not used in this case. */
                 tskIDLE_PRIORITY + 1,         /* Task priority, must be between 0 and configMAX_PRIORITIES - 1. */
                 NULL );                       /* Optional - used to pass out a handle to the created task. */
}


/*-----------------------------------------------------------*/
void vSleepUntilMQTTAgentReady( void )
{
	while( 1 )
	{
		if( xAgentEvents == NULL )
		{
			/* Agent has not even initialized its event groups yet. Sleep the task while agent initializes */
			vTaskDelay( pdMS_TO_TICKS( 3*1000 ) );
		}
		else
		{
			EventBits_t uxEvents = xEventGroupWaitBits( xAgentEvents,
													    1u << EVENT_BIT_AGENT_READY,
													    pdFALSE,
													    pdTRUE,
													    portMAX_DELAY );

			if( uxEvents & ( 1u << EVENT_BIT_AGENT_READY ) )
			{
				return;
			}
		}
	}
}

static BaseType_t xInitializeMqttConnectCtx( MqttConnectCtx_t * pxCtx )
{
    pxCtx->ulMqttEndpointLen = KVStore_getSize( CS_CORE_MQTT_ENDPOINT );

    configASSERT( pxCtx->ulMqttEndpointLen > 0 );

    pxCtx->pcMqttEndpointAddress = ( char * ) pvPortMalloc( pxCtx->ulMqttEndpointLen );

    if( pxCtx->pcMqttEndpointAddress == NULL )
    {
        pxCtx->ulMqttEndpointLen = 0;
    }
    else
    {
        ( void ) KVStore_getString( CS_CORE_MQTT_ENDPOINT,
                                 pxCtx->pcMqttEndpointAddress,
                                 pxCtx->ulMqttEndpointLen );
    }

    pxCtx->ulMqttClientIdLen = KVStore_getSize( CS_CORE_THING_NAME );

    configASSERT( pxCtx->ulMqttClientIdLen > 0 );

    pxCtx->pcMqttClientId = ( char * ) pvPortMalloc( pxCtx->ulMqttClientIdLen );

    if( pxCtx->pcMqttClientId == NULL )
    {
        pxCtx->ulMqttClientIdLen = 0;
    }
    else
    {
        ( void ) KVStore_getString( CS_CORE_THING_NAME,
                                    pxCtx->pcMqttClientId,
                                    pxCtx->ulMqttClientIdLen );
    }

    pxCtx->ulMqttPort = KVStore_getUInt32( CS_CORE_MQTT_PORT, NULL );

    return( pxCtx->pcMqttEndpointAddress != NULL );
}

/*-----------------------------------------------------------*/

static MQTTStatus_t prvMQTTInit( void )
{
    TransportInterface_t xTransport;
    MQTTStatus_t xReturn;
    MQTTFixedBuffer_t xFixedBuffer = { .pBuffer = xNetworkBuffer, .size = MQTT_AGENT_NETWORK_BUFFER_SIZE };
    static uint8_t staticQueueStorageArea[ MQTT_AGENT_COMMAND_QUEUE_LENGTH * sizeof( MQTTAgentCommand_t * ) ];
    static StaticQueue_t staticQueueStructure;

    MQTTAgentMessageInterface_t messageInterface =
    {
        .pMsgCtx        = NULL,
        .send           = Agent_MessageSend,
        .recv           = Agent_MessageReceive,
        .getCommand     = Agent_GetCommand,
        .releaseCommand = Agent_ReleaseCommand
    };

    LogDebug( ( "Creating command queue." ) );
    xCommandQueue.queue = xQueueCreateStatic( MQTT_AGENT_COMMAND_QUEUE_LENGTH,
                                              sizeof( MQTTAgentCommand_t * ),
                                              staticQueueStorageArea,
                                              &staticQueueStructure );
    configASSERT( xCommandQueue.queue );
    messageInterface.pMsgCtx = &xCommandQueue;

    /* Initialize the task pool. */
    Agent_InitializePool();

    /* Fill in Transport Interface send and receive function pointers. */
    xTransport.pNetworkContext = pxNetworkContext;
    xTransport.send = mbedtls_transport_send;
    xTransport.recv = mbedtls_transport_recv;

    /* Initialize MQTT library. */
    xReturn = MQTTAgent_Init( &xGlobalMqttAgentContext,
                              &messageInterface,
                              &xFixedBuffer,
                              &xTransport,
                              prvGetTimeMs,
                              prvIncomingPublishCallback,
                              /* Context to pass into the callback. Passing the pointer to subscription array. */
                              xGlobalSubscriptionList );

    return xReturn;
}

/*-----------------------------------------------------------*/

static MQTTStatus_t prvMQTTConnect( bool xCleanSession )
{
    MQTTStatus_t xResult;
    MQTTConnectInfo_t xConnectInfo;
    bool xSessionPresent = false;

    /* Many fields are not used in this demo so start with everything at 0. */
    memset( &xConnectInfo, 0x00, sizeof( xConnectInfo ) );

    /* Start with a clean session i.e. direct the MQTT broker to discard any
     * previous session data. Also, establishing a connection with clean session
     * will ensure that the broker does not store any data when this client
     * gets disconnected. */
    xConnectInfo.cleanSession = xCleanSession;

    /* The client identifier is used to uniquely identify this MQTT client to
     * the MQTT broker. In a production device the identifier can be something
     * unique, such as a device serial number. */
    xConnectInfo.pClientIdentifier = xConnectCtx.pcMqttClientId;
    xConnectInfo.clientIdentifierLength = xConnectCtx.ulMqttClientIdLen;

    /* Set MQTT keep-alive period. It is the responsibility of the application
     * to ensure that the interval between Control Packets being sent does not
     * exceed the Keep Alive value. In the absence of sending any other Control
     * Packets, the Client MUST send a PINGREQ Packet.  This responsibility will
     * be moved inside the agent. */
    xConnectInfo.keepAliveSeconds = mqttexampleKEEP_ALIVE_INTERVAL_SECONDS;

    /* Append metrics when connecting to the AWS IoT Core broker. */
    xConnectInfo.pUserName = AWS_IOT_METRICS_STRING;
    xConnectInfo.userNameLength = AWS_IOT_METRICS_STRING_LENGTH;
    /* Password for authentication is not used. */
    xConnectInfo.pPassword = NULL;
    xConnectInfo.passwordLength = 0U;

    /* Send MQTT CONNECT packet to broker. MQTT's Last Will and Testament feature
     * is not used in this demo, so it is passed as NULL. */
    xResult = MQTT_Connect( &( xGlobalMqttAgentContext.mqttContext ),
                            &xConnectInfo,
                            NULL,
                            CONNACK_RECV_TIMEOUT_MS,
                            &xSessionPresent );

    LogInfo( ( "Session present: %d\n", xSessionPresent ) );

    /* Resume a session if desired. */
    if( ( xResult == MQTTSuccess ) && ( xCleanSession == false ) )
    {
        xResult = MQTTAgent_ResumeSession( &xGlobalMqttAgentContext, xSessionPresent );

        /* Resubscribe to all the subscribed topics. */
        if( ( xResult == MQTTSuccess ) && ( xSessionPresent == false ) )
        {
            xResult = prvHandleResubscribe();
        }
    }

    return xResult;
}

/*-----------------------------------------------------------*/

static MQTTStatus_t prvHandleResubscribe( void )
{
    MQTTStatus_t xResult = MQTTBadParameter;
    uint32_t ulIndex = 0U;
    uint16_t usNumSubscriptions = 0U;

    /* These variables need to stay in scope until command completes. */
    static MQTTAgentSubscribeArgs_t xSubArgs = { 0 };
    static MQTTSubscribeInfo_t xSubInfo[ SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS ] = { 0 };
    static MQTTAgentCommandInfo_t xCommandParams = { 0 };

    /* Loop through each subscription in the subscription list and add a subscribe
     * command to the command queue. */
    for( ulIndex = 0U; ulIndex < SUBSCRIPTION_MANAGER_MAX_SUBSCRIPTIONS; ulIndex++ )
    {
        /* Check if there is a subscription in the subscription list. This demo
         * doesn't check for duplicate subscriptions. */
        if( xGlobalSubscriptionList[ ulIndex ].usFilterStringLength != 0 )
        {
            xSubInfo[ usNumSubscriptions ].pTopicFilter = xGlobalSubscriptionList[ ulIndex ].pcSubscriptionFilterString;
            xSubInfo[ usNumSubscriptions ].topicFilterLength = xGlobalSubscriptionList[ ulIndex ].usFilterStringLength;

            /* QoS1 is used for all the subscriptions in this demo. */
            xSubInfo[ usNumSubscriptions ].qos = MQTTQoS1;

            LogInfo( ( "Resubscribe to the topic %.*s will be attempted.",
                       xSubInfo[ usNumSubscriptions ].topicFilterLength,
                       xSubInfo[ usNumSubscriptions ].pTopicFilter ) );

            usNumSubscriptions++;
        }
    }

    if( usNumSubscriptions > 0U )
    {
        xSubArgs.pSubscribeInfo = xSubInfo;
        xSubArgs.numSubscriptions = usNumSubscriptions;

        /* The block time can be 0 as the command loop is not running at this point. */
        xCommandParams.blockTimeMs = 0U;
        xCommandParams.cmdCompleteCallback = prvSubscriptionCommandCallback;
        xCommandParams.pCmdCompleteCallbackContext = ( void * ) &xSubArgs;

        /* Enqueue subscribe to the command queue. These commands will be processed only
         * when command loop starts. */
        xResult = MQTTAgent_Subscribe( &xGlobalMqttAgentContext, &xSubArgs, &xCommandParams );
    }
    else
    {
        /* Mark the resubscribe as success if there is nothing to be subscribed. */
        xResult = MQTTSuccess;
    }

    if( xResult != MQTTSuccess )
    {
        LogError( ( "Failed to enqueue the MQTT subscribe command. xResult=%s.",
                    MQTT_Status_strerror( xResult ) ) );
    }

    return xResult;
}

/*-----------------------------------------------------------*/

static void prvSubscriptionCommandCallback( MQTTAgentCommandContext_t * pxCommandContext,
                                            MQTTAgentReturnInfo_t * pxReturnInfo )
{
    size_t lIndex = 0;
    MQTTAgentSubscribeArgs_t * pxSubscribeArgs = ( MQTTAgentSubscribeArgs_t * ) pxCommandContext;

    /* If the return code is success, no further action is required as all the topic filters
     * are already part of the subscription list. */
    if( pxReturnInfo->returnCode != MQTTSuccess )
    {
        /* Check through each of the suback codes and determine if there are any failures. */
        for( lIndex = 0; lIndex < pxSubscribeArgs->numSubscriptions; lIndex++ )
        {
            /* This demo doesn't attempt to resubscribe in the event that a SUBACK failed. */
            if( pxReturnInfo->pSubackCodes[ lIndex ] == MQTTSubAckFailure )
            {
                LogError( ( "Failed to resubscribe to topic %.*s.",
                            pxSubscribeArgs->pSubscribeInfo[ lIndex ].topicFilterLength,
                            pxSubscribeArgs->pSubscribeInfo[ lIndex ].pTopicFilter ) );
                /* Remove subscription callback for unsubscribe. */
                removeSubscription( xGlobalSubscriptionList,
                                    pxSubscribeArgs->pSubscribeInfo[ lIndex ].pTopicFilter,
                                    pxSubscribeArgs->pSubscribeInfo[ lIndex ].topicFilterLength );
            }
        }

        /* Hit an assert as some of the tasks won't be able to proceed correctly without
         * the subscriptions. This logic will be updated with exponential backoff and retry.  */
        configASSERT( pdTRUE );
    }
}

/*-----------------------------------------------------------*/

static BaseType_t prvSocketConnect( void )
{
    BaseType_t xConnected = pdFAIL;
    BackoffAlgorithmStatus_t xBackoffAlgStatus = BackoffAlgorithmSuccess;
    BackoffAlgorithmContext_t xReconnectParams = { 0 };
    uint16_t usNextRetryBackOff = 0U;

    TlsTransportStatus_t xNetworkStatus = TLS_TRANSPORT_CONNECT_FAILURE;
    NetworkCredentials_t xNetworkCredentials = { 0 };

    /* ALPN protocols must be a NULL-terminated list of strings. Therefore,
    * the first entry will contain the actual ALPN protocol string while the
    * second entry must remain NULL. */
    const char * pcAlpnProtocols[] = { NULL, NULL };

    pcAlpnProtocols[ 0 ] = AWS_IOT_MQTT_ALPN;
    xNetworkCredentials.pAlpnProtos = pcAlpnProtocols;

    /* Set the credentials for establishing a TLS connection. */
    xNetworkCredentials.xRootCaCertForm = OBJ_FORM_PEM;
    xNetworkCredentials.pvRootCaCert = democonfigROOT_CA_PEM;
    xNetworkCredentials.rootCaCertSize = sizeof( democonfigROOT_CA_PEM );

    xNetworkCredentials.xClientCertForm = OBJ_FORM_PEM;
    xNetworkCredentials.pvClientCert = democonfigCLIENT_CERTIFICATE_PEM;
    xNetworkCredentials.clientCertSize = sizeof( democonfigCLIENT_CERTIFICATE_PEM );

    xNetworkCredentials.xPrivateKeyForm = OBJ_FORM_PEM;
    xNetworkCredentials.pvPrivateKey = democonfigCLIENT_PRIVATE_KEY_PEM;
    xNetworkCredentials.privateKeySize = sizeof( democonfigCLIENT_PRIVATE_KEY_PEM );

    xNetworkCredentials.disableSni = democonfigDISABLE_SNI;

    /* We will use a retry mechanism with an exponential backoff mechanism and
     * jitter.  That is done to prevent a fleet of IoT devices all trying to
     * reconnect at exactly the same time should they become disconnected at
     * the same time. We initialize reconnect attempts and interval here. */
    BackoffAlgorithm_InitializeParams( &xReconnectParams,
                                       RETRY_BACKOFF_BASE_MS,
                                       RETRY_MAX_BACKOFF_DELAY_MS,
                                       RETRY_MAX_ATTEMPTS );

    /* Attempt to connect to MQTT broker. If connection fails, retry after a
     * timeout. Timeout value will exponentially increase until the maximum
     * number of attempts are reached.
     */
    do
    {
        /* Establish a TCP connection with the MQTT broker. This example connects to
        * the MQTT broker as specified in democonfigMQTT_BROKER_ENDPOINT and
        * democonfigMQTT_BROKER_PORT at the top of this file. */



        LogInfo( ( "Creating a TLS connection to %s:%d.",
        		xConnectCtx.pcMqttEndpointAddress,
				xConnectCtx.ulMqttPort ) );

        xNetworkStatus = mbedtls_transport_connect( pxNetworkContext,
        										    xConnectCtx.pcMqttEndpointAddress,
                                                    (uint16_t) xConnectCtx.ulMqttPort,
                                                    &xNetworkCredentials,
                                                    mqttexampleTRANSPORT_SEND_RECV_TIMEOUT_MS,
                                                    mqttexampleTRANSPORT_SEND_RECV_TIMEOUT_MS );

        xConnected = ( xNetworkStatus == TLS_TRANSPORT_SUCCESS ) ? pdPASS : pdFAIL;

        if( xConnected == pdPASS )
        {
            /* Set event group to wake tasks waiting for */
            (void) xEventGroupSetBits( xAgentEvents, ( 1u << EVENT_BIT_AGENT_READY ) );
        }
        else
        {
        	(void) xEventGroupClearBits( xAgentEvents, ( 1u << EVENT_BIT_AGENT_READY ) );
            /* Get back-off value (in milliseconds) for the next connection retry. */
            xBackoffAlgStatus = BackoffAlgorithm_GetNextBackoff( &xReconnectParams, uxRand(), &usNextRetryBackOff );

            if( xBackoffAlgStatus == BackoffAlgorithmSuccess )
            {
                LogWarn( ( "Connection to the broker failed. "
                           "Retrying connection in %hu ms.",
                           usNextRetryBackOff ) );
                vTaskDelay( pdMS_TO_TICKS( usNextRetryBackOff ) );
            }
        }

        if( xBackoffAlgStatus == BackoffAlgorithmRetriesExhausted )
        {
            LogError( ( "Connection to the broker failed, all attempts exhausted." ) );
        }
    } while( ( xConnected != pdPASS ) && ( xBackoffAlgStatus == BackoffAlgorithmSuccess ) );

    return xConnected;
}

/*-----------------------------------------------------------*/

//static void prvMQTTClientSocketWakeupCallback( Socket_t pxSocket )
//{
//    MQTTAgentCommandInfo_t xCommandParams = { 0 };
//
//    /* Just to avoid compiler warnings.  The socket is not used but the function
//     * prototype cannot be changed because this is a callback function. */
//    ( void ) pxSocket;
//
//    /* A socket used by the MQTT task may need attention.  Send an event
//     * to the MQTT task to make sure the task is not blocked on xCommandQueue. */
//    if( ( uxQueueMessagesWaiting( xCommandQueue.queue ) == 0U ) && ( FreeRTOS_recvcount( pxSocket ) > 0 ) )
//    {
//        /* Don't block as this is called from the context of the IP task. */
//        xCommandParams.blockTimeMs = 0U;
//        MQTTAgent_ProcessLoop( &xGlobalMqttAgentContext, &xCommandParams );
//    }
//}

/*-----------------------------------------------------------*/

static void prvIncomingPublishCallback( MQTTAgentContext_t * pMqttAgentContext,
                                        uint16_t packetId,
                                        MQTTPublishInfo_t * pxPublishInfo )
{
    bool xPublishHandled = false;
    char cOriginalChar, * pcLocation;

    ( void ) packetId;

    /* Fan out the incoming publishes to the callbacks registered using
     * subscription manager. */
    xPublishHandled = handleIncomingPublishes( ( SubscriptionElement_t * ) pMqttAgentContext->pIncomingCallbackContext,
                                               pxPublishInfo );


    #if ( democonfigCREATE_CODE_SIGNING_OTA_DEMO == 1 )

        /*
         * Check if the incoming publish is for OTA agent.
         */
        if( xPublishHandled != true )
        {
            xPublishHandled = vOTAProcessMessage( pMqttAgentContext->pIncomingCallbackContext, pxPublishInfo );
        }
    #endif

    /* If there are no callbacks to handle the incoming publishes,
     * handle it as an unsolicited publish. */
    if( xPublishHandled != true )
    {
        /* Ensure the topic string is terminated for printing.  This will over-
         * write the message ID, which is restored afterwards. */
        pcLocation = ( char * ) &( pxPublishInfo->pTopicName[ pxPublishInfo->topicNameLength ] );
        cOriginalChar = *pcLocation;
        *pcLocation = 0x00;
        LogWarn( ( "WARN:  Received an unsolicited publish from topic %s", pxPublishInfo->pTopicName ) );
        *pcLocation = cOriginalChar;
    }
}

/*-----------------------------------------------------------*/

static void prvMQTTAgentTask( void * pvParameters )
{
	xAgentEvents = xEventGroupCreate();
	configASSERT( xAgentEvents != NULL );

    vTaskDelay(10*1000); //HACK wait for interface
    // TODO: Replace with event group

    /* Miscellaneous initialization. */
    ulGlobalEntryTimeMs = prvGetTimeMs();

    xInitializeMqttConnectCtx( &xConnectCtx );

    /* Create the TCP connection to the broker, then the MQTT connection to the
     * same. */

    BaseType_t xNetworkStatus = pdFAIL;
    MQTTStatus_t xMQTTStatus;

    if( pxNetworkContext == NULL )
    {
        pxNetworkContext = mbedtls_transport_allocate( &xLwipTransportInterface );
    }

    configASSERT( pxNetworkContext != NULL );

    /* Connect a socket to the broker. */
    xNetworkStatus = prvSocketConnect();

    /* Initialize the MQTT context with the buffer and transport interface. */
    xMQTTStatus = prvMQTTInit();
    configASSERT( xMQTTStatus == MQTTSuccess );

    /* Form an MQTT connection without a persistent session. */
    xMQTTStatus = prvMQTTConnect( true );
    configASSERT( xMQTTStatus == MQTTSuccess );

    /* Selectively create demo tasks as per the compile time constant settings. */
    #if ( democonfigCREATE_LARGE_MESSAGE_SUB_PUB_TASK == 1 )
        {
            vStartLargeMessageSubscribePublishTask( democonfigLARGE_MESSAGE_SUB_PUB_TASK_STACK_SIZE,
                                                    tskIDLE_PRIORITY );
        }
    #endif

    #if ( democonfigNUM_SIMPLE_SUB_PUB_TASKS_TO_CREATE > 0 )
        {
            vStartSimpleSubscribePublishTask( democonfigNUM_SIMPLE_SUB_PUB_TASKS_TO_CREATE,
                                              democonfigSIMPLE_SUB_PUB_TASK_STACK_SIZE,
                                              tskIDLE_PRIORITY + 2 );
        }
    #endif

    #if ( democonfigCREATE_CODE_SIGNING_OTA_DEMO == 1 )
        {
            vStartOTACodeSigningDemo( democonfigCODE_SIGNING_OTA_TASK_STACK_SIZE,
                                      tskIDLE_PRIORITY + 1 );
        }
    #endif

    #if ( democonfigCREATE_DEFENDER_DEMO == 1 )
        {
            vStartDefenderDemo( democonfigDEFENDER_TASK_STACK_SIZE,
                                tskIDLE_PRIORITY );
        }
    #endif

    #if ( democonfigCREATE_SHADOW_DEMO == 1 )
        {
            vStartShadowDemo( democonfigSHADOW_TASK_STACK_SIZE,
                              tskIDLE_PRIORITY );
        }
    #endif

    {
        BaseType_t xNetworkResult = pdFAIL;
        MQTTStatus_t xMQTTStatus = MQTTSuccess, xConnectStatus = MQTTSuccess;
        MQTTContext_t * pMqttContext = &( xGlobalMqttAgentContext.mqttContext );

        ( void ) pvParameters;

        do
        {
            /* MQTTAgent_CommandLoop() is effectively the agent implementation.  It
             * will manage the MQTT protocol until such time that an error occurs,
             * which could be a disconnect.  If an error occurs the MQTT context on
             * which the error happened is returned so there can be an attempt to
             * clean up and reconnect however the application writer prefers. */
            xMQTTStatus = MQTTAgent_CommandLoop( &xGlobalMqttAgentContext );


            mbedtls_transport_disconnect( pxNetworkContext );

            /* Success is returned for disconnect or termination. The socket should
             * be disconnected. */
            if( xMQTTStatus == MQTTSuccess )
            {
                /* MQTT Disconnect. Disconnect the socket. */

            }
            /* Error. */
            else
            {
                #if ( democonfigCREATE_CODE_SIGNING_OTA_DEMO == 1 )
                    {
                        vSuspendOTACodeSigningDemo();
                    }
                #endif

                /* Reconnect TCP. */
                xNetworkResult = prvSocketConnect();
                // TODO: handle this in a more reasonable way.
                configASSERT( xNetworkResult == pdPASS );
                pMqttContext->connectStatus = MQTTNotConnected;
                /* MQTT Connect with a persistent session. */
                xConnectStatus = prvMQTTConnect( false );
                configASSERT( xConnectStatus == MQTTSuccess );

                #if ( democonfigCREATE_CODE_SIGNING_OTA_DEMO == 1 )
                    {
                        if( xMQTTStatus == MQTTSuccess )
                        {
                            vResumeOTACodeSigningDemo();
                        }
                    }
                #endif
            }
        } while( xMQTTStatus != MQTTSuccess );
    }
}

/*-----------------------------------------------------------*/

static uint32_t prvGetTimeMs( void )
{
    uint32_t ulTimeMs = 0UL;

    /* Determine the elapsed time in the application */
    ulTimeMs = ( uint32_t ) ( xTaskGetTickCount() * portTICK_PERIOD_MS ) - ulGlobalEntryTimeMs;

    return ulTimeMs;
}

/*-----------------------------------------------------------*/
