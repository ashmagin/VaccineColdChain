#include "logging_levels.h"
/* define LOG_LEVEL here if you want to modify the logging level from the default */

#define LOG_LEVEL LOG_ERROR

#include "logging.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Standard includes. */
#include <string.h>
#include <stdio.h>
#include <assert.h>

/* Sensor includes */
#include "b_u585i_iot02a_env_sensors.h"
#include "b_u585i_iot02a_motion_sensors.h"
#include "ism330dlc/ism330dlc.h"

#include "sensor_readout.h"
#include "task_notifications.h"
#include "main.h"

/*-----------------------------------------------------------*/
//#define SENSOR_DATA_REFRESH_PERIOD_MS (2000)
#define SENSOR_DATA_REFRESH_PERIOD_MS (1000)

#define FREEFALL_DETECTION_INT_PIN (ISM330DLC_INT1_PIN)
#define FREEFALL_DETECTION_THRESHOLD (0x00u)  /* 3 bits: 0x0=156mg -> 0x7=500mg */
#define FREEFALL_DETECTION_DURATION  (0x0Fu)  /* 6 bits: 1 lsb = 1/ODR time */

/*-----------------------------------------------------------*/

static EnvironmentalSensorData_t envData;
static MotionSensorData_t        motionData;
static GeoPositionData_t         geoData;

static BaseType_t envDataValid = pdFALSE;
static BaseType_t motionDataValid = pdFALSE;
static BaseType_t geoDataValid = pdFALSE;
static volatile BaseType_t freeFallDetectedFlag = pdFALSE;

static SemaphoreHandle_t dataAccessLock = NULL;

/** Stub variables for geo data */
static uint32_t geoStubIndex;
static BaseType_t geoStubDirectionUp;

/*-----------------------------------------------------------*/

/**
  * @brief  Handle a detected free fall event
  * @retval None
  */
static void FreeFallDetection_Handler()
{
	ISM330DLC_Event_Status_t EventStatus = {0};
	int32_t xReturn = 0;

	/* Get event which generated the interrupt */
	xReturn = BSP_MOTION_SENSOR_GetEventStatus(0, MOTION_ACCELERO, &EventStatus);
	if (ISM330DLC_OK == xReturn)
	{
		if (EventStatus.FreeFallStatus != 0)
		{
			freeFallDetectedFlag = pdTRUE;
			WakeupShadowUpdateTask(pdTRUE);
		}
	}
}

/*-----------------------------------------------------------*/

static BaseType_t xInitEnvSensors( void )
{
    int32_t lBspError = BSP_ERROR_NONE;

    lBspError = BSP_ENV_SENSOR_Init( 0, ENV_TEMPERATURE );

    lBspError |= BSP_ENV_SENSOR_Init( 0, ENV_HUMIDITY );

    lBspError |= BSP_ENV_SENSOR_Init( 1, ENV_TEMPERATURE );

    lBspError |= BSP_ENV_SENSOR_Init( 1, ENV_PRESSURE );

    lBspError |= BSP_ENV_SENSOR_Enable( 0, ENV_TEMPERATURE );

    lBspError |= BSP_ENV_SENSOR_Enable( 0, ENV_HUMIDITY );

    lBspError |= BSP_ENV_SENSOR_Enable( 1, ENV_TEMPERATURE );

    lBspError |= BSP_ENV_SENSOR_Enable( 1, ENV_PRESSURE );

    lBspError |= BSP_ENV_SENSOR_SetOutputDataRate( 0, ENV_TEMPERATURE, 1.0f );

    lBspError |= BSP_ENV_SENSOR_SetOutputDataRate( 0, ENV_HUMIDITY, 1.0f );

    lBspError |= BSP_ENV_SENSOR_SetOutputDataRate( 1, ENV_TEMPERATURE, 1.0f );

    lBspError |= BSP_ENV_SENSOR_SetOutputDataRate( 1, ENV_PRESSURE, 1.0f );

    return ( lBspError == BSP_ERROR_NONE ? pdTRUE : pdFALSE );
}

/*-----------------------------------------------------------*/

static BaseType_t xInitMotionSensors( void )
{
    int32_t lBspError = BSP_ERROR_NONE;

    /* Gyro + Accelerometer*/
    lBspError = BSP_MOTION_SENSOR_Init( 0, MOTION_GYRO | MOTION_ACCELERO );
    lBspError |= BSP_MOTION_SENSOR_Enable( 0, MOTION_GYRO );
    lBspError |= BSP_MOTION_SENSOR_SetOutputDataRate( 0, MOTION_GYRO, 1.0f );
    lBspError |= BSP_MOTION_SENSOR_SetOutputDataRate( 0, MOTION_ACCELERO, 1.0f );

    /** Free Fall Detection */
    //GPIO_PIN_12
    GPIO_EXTI_Register_Callback(ISM330DHCX_INT1_Pin, FreeFallDetection_Handler, NULL);
    motionData.xFallDetected = pdFALSE;
    lBspError |= BSP_MOTION_SENSOR_EnableFreeFallDetection(0, MOTION_ACCELERO, FREEFALL_DETECTION_INT_PIN);
    lBspError |= BSP_MOTION_SENSOR_SetFreeFallDetectionThreshold(0, MOTION_ACCELERO, FREEFALL_DETECTION_THRESHOLD);
    lBspError |= BSP_MOTION_SENSOR_SetFreeFallDetectionDuration(0, MOTION_ACCELERO, FREEFALL_DETECTION_DURATION);

    lBspError |= BSP_MOTION_SENSOR_Enable( 0, MOTION_ACCELERO );

    /* Magnetometer */
    lBspError |= BSP_MOTION_SENSOR_Init( 1, MOTION_MAGNETO );
    lBspError |= BSP_MOTION_SENSOR_Enable( 1, MOTION_MAGNETO );
    lBspError |= BSP_MOTION_SENSOR_SetOutputDataRate( 1, MOTION_MAGNETO, 1.0f );

    return ( lBspError == BSP_ERROR_NONE ? pdTRUE : pdFALSE );
}

/*-----------------------------------------------------------*/

static BaseType_t xInitGeoSensors( void )
{
	geoStubIndex = 0u;
	geoStubDirectionUp = pdTRUE;
	return pdTRUE;
}

/*-----------------------------------------------------------*/

static BaseType_t xUpdateEnvSensorData()
{
    int32_t lBspError = BSP_ERROR_NONE;

	lBspError = BSP_ENV_SENSOR_GetValue( 0, ENV_TEMPERATURE, &envData.fTemperature0 );
	lBspError |= BSP_ENV_SENSOR_GetValue( 0, ENV_HUMIDITY, &envData.fHumidity );
	lBspError |= BSP_ENV_SENSOR_GetValue( 1, ENV_TEMPERATURE, &envData.fTemperature1 );
	lBspError |= BSP_ENV_SENSOR_GetValue( 1, ENV_PRESSURE, &envData.fBarometricPressure );

    return ( lBspError == BSP_ERROR_NONE ? pdTRUE : pdFALSE );
}

/*-----------------------------------------------------------*/

static BaseType_t xUpdateMotionSensorData()
{
    int32_t lBspError = BSP_ERROR_NONE;

    lBspError = BSP_MOTION_SENSOR_GetAxes( 0, MOTION_GYRO, &motionData.xGyroAxes );
    lBspError |= BSP_MOTION_SENSOR_GetAxes( 0, MOTION_ACCELERO, &motionData.xAcceleroAxes );
    lBspError |= BSP_MOTION_SENSOR_GetAxes( 1, MOTION_MAGNETO, &motionData.xMagnetoAxes );

    if (freeFallDetectedFlag != pdFALSE)
    {
    	freeFallDetectedFlag = pdFALSE;
    	motionData.xFallDetected = pdTRUE;
    }

    return ( lBspError == BSP_ERROR_NONE ? pdTRUE : pdFALSE );
}

/*-----------------------------------------------------------*/

extern const GeoPositionData_t geoDataStub[];
extern const uint32_t geoDataStubLength;
static BaseType_t xUpdateGeoSensorData()
{
	geoData = geoDataStub[geoStubIndex];
	geoStubIndex++;

	if (geoStubIndex >= geoDataStubLength)
	{
		/** The route is circle */
		geoStubIndex = 0u;
	}


	return pdTRUE;
}


/*-----------------------------------------------------------*/

static BaseType_t xInitSensors(void)
{
	BaseType_t res;

	res = xInitEnvSensors();

	if (res != pdFALSE)
	{
		res = xInitMotionSensors();
	}

	if (res != pdFALSE)
	{
		dataAccessLock = xSemaphoreCreateBinary();
		res = (dataAccessLock != NULL ? pdTRUE : pdFALSE);
	}

	if (res != pdFALSE)
	{
		res = xInitGeoSensors();
	}

	if (res != pdFALSE)
	{
		/** Binary semaphore is created locked, so we don't need to acquire it */
		envDataValid = xUpdateEnvSensorData();
		motionDataValid = xUpdateMotionSensorData();
		geoDataValid = xUpdateGeoSensorData();
		res = ((envDataValid && motionDataValid && geoDataValid) ? pdTRUE : pdFALSE);
		xSemaphoreGive(dataAccessLock);
	}

	return res;
}

/*-----------------------------------------------------------*/

void vSensorReadoutTask(void * pvParameters)
{
    BaseType_t xResult = pdFALSE;

    (void)pvParameters;

    xResult = xInitSensors();

    if (pdFALSE == xResult)
    {
        LogError("Error while initializing sensors.");
        vTaskDelete(NULL);
    }

    while(1)
    {
    	vTaskDelay(pdMS_TO_TICKS(SENSOR_DATA_REFRESH_PERIOD_MS));
    	xSemaphoreTake(dataAccessLock, portMAX_DELAY);
    	envDataValid = xUpdateEnvSensorData();
		motionDataValid = xUpdateMotionSensorData();
		geoDataValid = xUpdateGeoSensorData();
    	xSemaphoreGive(dataAccessLock);

    	WakeupEnvPublishTask();
    	WakeupMotionPublishTask();
    	WakeupGeoPublishTask();
    	WakeupShadowUpdateTask(pdFALSE);
    }
}

/*-----------------------------------------------------------*/

BaseType_t SensorReadout_GetEnvSensorData(EnvironmentalSensorData_t * const dataOut)
{
	BaseType_t ret;

	xSemaphoreTake(dataAccessLock, portMAX_DELAY);
	if (envDataValid != pdFALSE)
	{
		memcpy(dataOut, &envData, sizeof(EnvironmentalSensorData_t));
	}
	ret = envDataValid;
	xSemaphoreGive(dataAccessLock);

	return ret;
}

/*-----------------------------------------------------------*/

BaseType_t SensorReadout_GetMotionSensorData(MotionSensorData_t * const dataOut)
{
	BaseType_t ret;
	xSemaphoreTake(dataAccessLock, portMAX_DELAY);
	if (motionDataValid != pdFALSE)
	{
		memcpy(dataOut, &motionData, sizeof(MotionSensorData_t));
	}
	ret = motionDataValid;
	xSemaphoreGive(dataAccessLock);

	return ret;
}


/*-----------------------------------------------------------*/

BaseType_t SensorReadout_GetGeoPositionData(GeoPositionData_t * const dataOut)
{
	BaseType_t ret;
	xSemaphoreTake(dataAccessLock, portMAX_DELAY);
	if (geoDataValid != pdFALSE)
	{
		memcpy(dataOut, &geoData, sizeof(GeoPositionData_t));
	}
	ret = geoDataValid;
	xSemaphoreGive(dataAccessLock);

	return ret;
}

/*-----------------------------------------------------------*/

void SensorReadout_ClearFreeFallAlarm(void)
{
	xSemaphoreTake(dataAccessLock, portMAX_DELAY);
	motionData.xFallDetected = pdFALSE;
	xSemaphoreGive(dataAccessLock);
}
