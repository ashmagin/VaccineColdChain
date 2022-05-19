#ifndef TASK_NOTIFICATIONS_H_
#define TASK_NOTIFICATIONS_H_

/* Kernel includes. */
#include "FreeRTOS.h"

#define TASK_NOTIFICATION_PUBLISH_ENV_DATA    (1u << 0u)
#define TASK_NOTIFICATION_PUBLISH_MOTION_DATA (1u << 1u)
#define TASK_NOTIFICATION_PUBLISH_GEO_DATA    (1u << 2u)
#define TASK_NOTIFICATION_UPDATE_SHADOW       (1u << 3u)

void WakeupEnvPublishTask(void);
void WakeupMotionPublishTask(void);
void WakeupGeoPublishTask(void);
void WakeupShadowUpdateTask(BaseType_t fromISR);

#endif /* TASK_NOTIFICATIONS_H_ */
