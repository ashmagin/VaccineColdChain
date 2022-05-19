/*
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Derived from simple_sub_pub_demo.c
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
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */


#include "logging_levels.h"
/* define LOG_LEVEL here if you want to modify the logging level from the default */

#define LOG_LEVEL LOG_ERROR

#include "logging.h"


/* Standard includes. */
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* For I2c mutex */
#include "main.h"

/* MQTT library includes. */
#include "core_mqtt.h"

/* MQTT agent include. */
#include "core_mqtt_agent.h"
#include "mqtt_agent_task.h"

#include "kvstore.h"

/* Subscription manager header include. */
#include "subscription_manager.h"

/* Sensor includes */
#include "b_u585i_iot02a_env_sensors.h"
#include "b_u585i_iot02a_motion_sensors.h"
#include "sensors/sensor_readout.h"

/* Task Notifications */
#include "task_notifications.h"

/**
 * @brief Size of statically allocated buffers for holding topic names and
 * payloads.
 */
#define MQTT_PUBLISH_MAX_LEN              ( 256 )
#define MQTT_PUBLISH_TOPIC_MOTION         "motion_sensor_data"
#define MQTT_PUBLISH_TOPIC_ENV            "env_sensor_data"
#define MQTT_PUBLISH_TOPIC_GEO            "geo_data"
#define MQTT_PUBLICH_TOPIC_STR_LEN        ( 256 )
#define MQTT_PUBLISH_BLOCK_TIME_MS        ( 10000 )
#define MQTT_NOTIFY_IDX                   ( 1 )
#define MQTT_PUBLISH_QOS                  ( 0 )

/*-----------------------------------------------------------*/

/**
 * @brief Defines the structure to use as the command callback context in this
 * demo.
 */
struct MQTTAgentCommandContext
{
    MQTTStatus_t xReturnStatus;
    TaskHandle_t xTaskToNotify;
};

/*-----------------------------------------------------------*/

static char payloadBuf[ MQTT_PUBLISH_MAX_LEN ];

/*-----------------------------------------------------------*/

extern MQTTAgentContext_t xGlobalMqttAgentContext;

/*-----------------------------------------------------------*/

static void prvPublishCommandCallback( MQTTAgentCommandContext_t * pxCommandContext,
                                       MQTTAgentReturnInfo_t * pxReturnInfo )
{
    configASSERT( pxCommandContext != NULL );
    configASSERT( pxReturnInfo != NULL );

    pxCommandContext->xReturnStatus = pxReturnInfo->returnCode;

    if( pxCommandContext->xTaskToNotify != NULL )
    {
        /* Send the context's ulNotificationValue as the notification value so
         * the receiving task can check the value it set in the context matches
         * the value it receives in the notification. */
        ( void ) xTaskNotifyGiveIndexed( pxCommandContext->xTaskToNotify,
                                         MQTT_NOTIFY_IDX );
    }
}

/*-----------------------------------------------------------*/

static BaseType_t prvPublishAndWaitForAck( const char * pcTopic,
                                           const void * pvPublishData,
                                           size_t xPublishDataLen )
{
    BaseType_t xResult = pdFALSE;
    MQTTStatus_t xStatus;

    configASSERT( pcTopic != NULL );
    configASSERT( pvPublishData != NULL );
    configASSERT( xPublishDataLen > 0 );

    MQTTPublishInfo_t xPublishInfo =
    {
        .qos = MQTT_PUBLISH_QOS,
        .retain = 0,
        .dup = 0,
        .pTopicName = pcTopic,
        .topicNameLength = strlen( pcTopic ),
        .pPayload = pvPublishData,
        .payloadLength = xPublishDataLen
    };

    MQTTAgentCommandContext_t xCommandContext =
    {
        .xTaskToNotify = xTaskGetCurrentTaskHandle(),
        .xReturnStatus = MQTTIllegalState,
    };

    MQTTAgentCommandInfo_t xCommandParams =
    {
        .blockTimeMs = MQTT_PUBLISH_BLOCK_TIME_MS,
        .cmdCompleteCallback = prvPublishCommandCallback,
        .pCmdCompleteCallbackContext =  &xCommandContext
    };

    /* Clear the notification index */
    xStatus = MQTTAgent_Publish( &xGlobalMqttAgentContext,
                                 &xPublishInfo,
                                 &xCommandParams );

    if( xStatus == MQTTSuccess )
    {
        xResult = ulTaskNotifyTakeIndexed( MQTT_NOTIFY_IDX,
                                           pdTRUE,
                                           pdMS_TO_TICKS( MQTT_PUBLISH_BLOCK_TIME_MS ) );

        if( xResult == 0 )
        {
            LogError( "Timed out while waiting for publish ACK or Sent event. xTimeout = %d",
                      pdMS_TO_TICKS( MQTT_PUBLISH_BLOCK_TIME_MS ) );
            xResult = pdFALSE;
        }
        else if( xCommandContext.xReturnStatus != MQTTSuccess )
        {
            LogError( "MQTT Agent returned error code: %d during publish operation.",
                      xCommandContext.xReturnStatus );
            xResult = pdFALSE;
        }
    }
    else
    {
        LogError( "MQTTAgent_Publish returned error code: %d.",
                  xStatus );
    }

    return xResult;
}

/*-----------------------------------------------------------*/

static void xPublishEnvSensorData(const char * const pcTopicString)
{
	BaseType_t xResult = pdFALSE;

	EnvironmentalSensorData_t xEnvData;
	xResult = SensorReadout_GetEnvSensorData( &xEnvData );

	if( xResult == pdTRUE )
	{
		int bytesWritten = 0;

		/* Write to */
		bytesWritten = snprintf( payloadBuf,
				MQTT_PUBLISH_MAX_LEN,
				"{ \"temp_0_c\": %f, \"rh_pct\": %f, \"temp_1_c\": %f, \"baro_mbar\": %f }",
				xEnvData.fTemperature0,
				xEnvData.fHumidity,
				xEnvData.fTemperature1,
				xEnvData.fBarometricPressure );

		if( bytesWritten < MQTT_PUBLISH_MAX_LEN )
		{
			xResult = prvPublishAndWaitForAck( pcTopicString,
					payloadBuf,
					bytesWritten );
		}
		else
		{
			LogError("Not enough buffer space.");
		}

		if( xResult == pdTRUE )
		{
			LogDebug( payloadBuf );
		}
	}
	else
	{
		LogError("Failed to update sensor data.");
	}
}

/*-----------------------------------------------------------*/

static void xPublishMotionSensorData(const char * const pcTopicString)
{
	BaseType_t xResult = pdFALSE;

	MotionSensorData_t xMotionData;
	xResult = SensorReadout_GetMotionSensorData( &xMotionData );

	if( xResult == pdTRUE )
	{
		int bytesWritten = snprintf( payloadBuf,
				MQTT_PUBLISH_MAX_LEN,
				"{"
			"\"acceleration_mG\":"
			"{"
				"\"x\": %ld,"
				"\"y\": %ld,"
				"\"z\": %ld"
				"},"
				"\"gyro_mDPS\":"
				"{"
				"\"x\": %ld,"
				"\"y\": %ld,"
				"\"z\": %ld"
				"},"
				"\"magnetism_mGauss\":"
				"{"
				"\"x\": %ld,"
				"\"y\": %ld,"
				"\"z\": %ld"
				"}"
				"}",
				xMotionData.xAcceleroAxes.x, xMotionData.xAcceleroAxes.y, xMotionData.xAcceleroAxes.z,
				xMotionData.xGyroAxes.x, xMotionData.xGyroAxes.y, xMotionData.xGyroAxes.z,
				xMotionData.xMagnetoAxes.x, xMotionData.xMagnetoAxes.y, xMotionData.xMagnetoAxes.z );

		if( bytesWritten < MQTT_PUBLISH_MAX_LEN )
		{
			xResult = prvPublishAndWaitForAck( pcTopicString,
					payloadBuf,
					bytesWritten );
			if( xResult != pdPASS )
			{
				LogError( "Failed to publish motion sensor data" );
			}
		}
	}
}

/*-----------------------------------------------------------*/

static void xPublishGeoSensorData(const char * const pcTopicString)
{
	BaseType_t xResult = pdFALSE;

	GeoPositionData_t xPositionData;
	xResult = SensorReadout_GetGeoPositionData( &xPositionData );

	if( xResult == pdTRUE )
	{
		int bytesWritten = snprintf( payloadBuf,
				MQTT_PUBLISH_MAX_LEN,
				"{"
				"\"geoJSON\":"
				"{"
				"\"type\": \"Point\","
				"\"coordinates\": ["
				"%f,"
				"%f"
				"]"
				"}"
				"}",
				xPositionData.fLongitude, xPositionData.fLatitude );

		if( bytesWritten < MQTT_PUBLISH_MAX_LEN )
		{
			xResult = prvPublishAndWaitForAck( pcTopicString,
					payloadBuf,
					bytesWritten );
			if( xResult != pdPASS )
			{
				LogError( "Failed to publish position data" );
			}
		}
	}
}

/*-----------------------------------------------------------*/

void vSensorDataPublishTask( void * pvParameters )
{
    (void) pvParameters;
    BaseType_t xExitFlag = pdFALSE;

    /* Build the topic strings */
    char envTopicString[ MQTT_PUBLICH_TOPIC_STR_LEN ] = { 0 };
    char motionTopicString[ MQTT_PUBLICH_TOPIC_STR_LEN ] = { 0 };
    char geoTopicString[ MQTT_PUBLICH_TOPIC_STR_LEN ] = { 0 };
    size_t xTopicLen = 0;

    /** Environmental sensors topic */
    xTopicLen = strlcat( envTopicString, "/", MQTT_PUBLICH_TOPIC_STR_LEN );
    if( xTopicLen + 1 < MQTT_PUBLICH_TOPIC_STR_LEN )
    {
        ( void ) KVStore_getString( CS_CORE_THING_NAME, &( envTopicString[ xTopicLen ] ), MQTT_PUBLICH_TOPIC_STR_LEN - xTopicLen );

        xTopicLen = strlcat( envTopicString, "/"MQTT_PUBLISH_TOPIC_ENV, MQTT_PUBLICH_TOPIC_STR_LEN );
    }

    /** Motion sensors topic */
    xTopicLen = strlcat( motionTopicString, "/", MQTT_PUBLICH_TOPIC_STR_LEN );
    if( xTopicLen + 1 < MQTT_PUBLICH_TOPIC_STR_LEN )
    {
        ( void ) KVStore_getString( CS_CORE_THING_NAME, &( motionTopicString[ xTopicLen ] ), MQTT_PUBLICH_TOPIC_STR_LEN - xTopicLen );

        xTopicLen = strlcat( motionTopicString, "/"MQTT_PUBLISH_TOPIC_MOTION, MQTT_PUBLICH_TOPIC_STR_LEN );
    }

    /** Environmental sensors topic */
    xTopicLen = strlcat( geoTopicString, "/", MQTT_PUBLICH_TOPIC_STR_LEN );
    if( xTopicLen + 1 < MQTT_PUBLICH_TOPIC_STR_LEN )
    {
        ( void ) KVStore_getString( CS_CORE_THING_NAME, &( geoTopicString[ xTopicLen ] ), MQTT_PUBLICH_TOPIC_STR_LEN - xTopicLen );

        xTopicLen = strlcat( geoTopicString, "/"MQTT_PUBLISH_TOPIC_GEO, MQTT_PUBLICH_TOPIC_STR_LEN );
    }

    LogInfo( "Waiting until MQTT Agent is ready" );
    vSleepUntilMQTTAgentReady();
    LogInfo( "MQTT Agent is ready. Resuming..." );

    while( xExitFlag == pdFALSE )
    {
    	uint32_t ulNotifiedValue;

        xTaskNotifyWait(0x00, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY );

        if ((ulNotifiedValue & TASK_NOTIFICATION_PUBLISH_ENV_DATA) != 0u)
        {
        	xPublishEnvSensorData(envTopicString);
        }

        if ((ulNotifiedValue & TASK_NOTIFICATION_PUBLISH_MOTION_DATA) != 0u)
        {
        	xPublishMotionSensorData(motionTopicString);
        }

        if ((ulNotifiedValue & TASK_NOTIFICATION_PUBLISH_GEO_DATA) != 0u)
        {
        	xPublishGeoSensorData(geoTopicString);
        }
    }

    vTaskDelete(NULL);
}
