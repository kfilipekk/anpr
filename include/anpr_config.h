#pragma once

/* Timing & Intervals */
#define CAPTURE_COOLDOWN_S    5
#define FALLBACK_INTERVAL_US  10000000LL  //10 secs
#define LIDAR_CHECK_MS        100
#define UPLOAD_POLL_MS        5000

/* Task Configuration */
#define TASK_ANPR_STACK       4096
#define TASK_UPLOAD_STACK     8192
#define TASK_PRIORITY_HIGH    5
#define TASK_PRIORITY_LOW     4

/* Queue Settings */
#define UPLOAD_QUEUE_LEN      5
#define UPLOAD_SEND_TIMEOUT   0
