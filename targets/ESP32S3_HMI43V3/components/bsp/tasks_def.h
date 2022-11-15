//Task definition for ESP32S3_HMI43V3

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define NETWORK_STACK_DEPTH 4096 *2
#define STREAM_STACK_DEPTH 4096 *2
#define UI_STACK_DEPTH 4096 *2

#define STREAM_CHUNK_SIZE 1024

#ifdef __cplusplus
} /* extern "C" */
#endif