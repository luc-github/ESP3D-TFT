/**
 * @file lv_test_conf.h
 *
 */

#ifndef LV_TEST_CONF_H
#define LV_TEST_CONF_H

#define LV_CONF_SUPPRESS_DEFINE_CHECK 1

#ifdef __cplusplus
extern "C" {
#endif

/***********************
 * PLATFORM CONFIGS
 ***********************/

#ifdef LVGL_CI_USING_SYS_HEAP
#define LV_USE_STDLIB_MALLOC        LV_STDLIB_CLIB
#define LV_USE_STDLIB_STRING        LV_STDLIB_CLIB
#define LV_USE_STDLIB_SPRINTF       LV_STDLIB_CLIB
#define LV_USE_OS                   LV_OS_PTHREAD
#define LV_OBJ_STYLE_CACHE          0
#endif

#ifdef LVGL_CI_USING_DEF_HEAP
#define LV_USE_STDLIB_MALLOC    LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_STRING    LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_SPRINTF   LV_STDLIB_BUILTIN
#define LV_OBJ_STYLE_CACHE      1
#endif

#ifdef MICROPYTHON
#define LV_USE_BUILTIN_MALLOC   0
#define LV_USE_BUILTIN_MEMCPY   1
#define LV_USE_BUILTIN_SNPRINTF 1
#define LV_STDLIB_INCLUDE "include/lv_mp_mem_custom_include.h"
#define LV_MALLOC       m_malloc
#define LV_REALLOC      m_realloc
#define LV_FREE         m_free
#define LV_MEMSET       lv_memset_builtin
#define LV_MEMCPY       lv_memcpy_builtin
#define LV_SNPRINTF     lv_snprintf_builtin
#define LV_VSNPRINTF    lv_vsnprintf_builtin
#define LV_STRLEN       lv_strlen_builtin
#define LV_STRNCPY      lv_strncpy_builtin

#define LV_ENABLE_GC 1
#define LV_GC_INCLUDE "py/mpstate.h"
#define LV_GC_ROOT(x) MP_STATE_PORT(x)
#endif

#ifndef __ASSEMBLY__
void lv_test_assert_fail(void);
#define LV_ASSERT_HANDLER lv_test_assert_fail();

typedef void * lv_user_data_t;
#endif
/***********************
 * TEST CONFIGS
 ***********************/

#if !(defined(LV_TEST_OPTION)) || LV_TEST_OPTION == 5
#define  LV_COLOR_DEPTH     32
#define  LV_DPI_DEF         160
#include "lv_test_conf_full.h"
#elif LV_TEST_OPTION == 6
#define  LV_COLOR_DEPTH     32
#define  LV_DPI_DEF         160

#define LV_USE_THORVG_INTERNAL      1
/* Use VG-Lite GPU. */
#define LV_USE_DRAW_VG_LITE         1

/* Enable VG-Lite custom external 'gpu_init()' function */
#define LV_VG_LITE_USE_GPU_INIT     1

/* Enable VG-Lite assert. */
#define LV_VG_LITE_USE_ASSERT       1

/* Simulate VG-Lite hardware using ThorVG */
#define LV_USE_VG_LITE_THORVG       1

/* VG-Lite GPU buffer alignment. */
#define LV_DRAW_BUF_ALIGN           64

#include "lv_test_conf_full.h"
#elif LV_TEST_OPTION == 4
#define  LV_COLOR_DEPTH     24
#define  LV_DPI_DEF         120
#elif LV_TEST_OPTION == 3
#define  LV_COLOR_DEPTH     16
#define  LV_DPI_DEF         90
#include "lv_test_conf_minimal.h"
#elif LV_TEST_OPTION == 2
#define  LV_COLOR_DEPTH     8
#define  LV_DPI_DEF         60
#include "lv_test_conf_minimal.h"
#elif LV_TEST_OPTION == 1
#define  LV_COLOR_DEPTH     1
#define  LV_DPI_DEF         30
#include "lv_test_conf_minimal.h"
#endif

#if defined(LVGL_CI_USING_SYS_HEAP) || defined(LVGL_CI_USING_DEF_HEAP)
#undef LV_LOG_PRINTF

/*Use a large value be sure any issues will cause crash*/
#define LV_DRAW_BUF_STRIDE_ALIGN                64

#if !defined(LV_DRAW_BUF_ALIGN)
/*Use non power of 2 to avoid the case when `malloc` returns aligned pointer by default, and use a large value be sure any issues will cause crash*/
#define LV_DRAW_BUF_ALIGN                       852
#endif

/*For screenshots*/
#undef LV_USE_PERF_MONITOR
#undef LV_USE_MEM_MONITOR
#undef LV_DPI_DEF
#define  LV_DPI_DEF         130
#endif

#if defined(LVGL_CI_USING_SYS_HEAP)
#undef LV_USE_FLOAT
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_TEST_CONF_H*/
