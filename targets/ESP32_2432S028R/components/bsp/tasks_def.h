//Task definition for ESP32 2332S028R

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define NETWORK_STACK_DEPTH 4096 *2
#define STREAM_STACK_DEPTH 4096 *2
#define UI_STACK_DEPTH 4096 *2

#define STREAM_CHUNK_SIZE 1024

#define ESP3D_SOCKET_RX_BUFFER_SIZE 1024
#define ESP3D_SOCKET_TASK_SIZE 4096
#define ESP3D_SOCKET_TASK_PRIORITY 5
#define ESP3D_SOCKET_TASK_CORE 0

#ifdef __cplusplus
} /* extern "C" */
#endif