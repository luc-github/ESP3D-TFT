// Task definition for ESP32_CUSTOM

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define NETWORK_TASK_CORE 0
#define NETWORK_TASK_PRIORITY 0
#define NETWORK_STACK_DEPTH 4096

#define STREAM_TASK_CORE 1
#define STREAM_TASK_PRIORITY 0
#define STREAM_STACK_DEPTH 4096

#define STREAM_CHUNK_SIZE 1024

#define ESP3D_SOCKET_RX_BUFFER_SIZE 512
#define ESP3D_SOCKET_TASK_SIZE 4096
#define ESP3D_SOCKET_TASK_PRIORITY 5
#define ESP3D_SOCKET_TASK_CORE 0

#define ESP3D_WS_RX_BUFFER_SIZE 512
#define ESP3D_WS_TASK_SIZE 4096
#define ESP3D_WS_TASK_PRIORITY 5
#define ESP3D_WS_TASK_CORE 0

#ifdef __cplusplus
} /* extern "C" */
#endif