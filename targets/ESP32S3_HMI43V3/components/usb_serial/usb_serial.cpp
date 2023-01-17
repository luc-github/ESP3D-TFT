/**
 * @file usb_serial.cpp
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "esp3d_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "usb_serial.h"
#include "usb/cdc_acm_host.h"
#include "usb/vcp_ch34x.hpp"
#include "usb/vcp_cp210x.hpp"
#include "usb/vcp_ftdi.hpp"
#include "usb/vcp.hpp"
#include "usb/usb_host.h"
#include "usb_serial_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief USB Host library handling task
 *
 * @param arg Unused
 */
void usb_lib_task(void *arg)
{
    while (1) {
        // Start handling system events
        uint32_t event_flags;
        //exception must be enabled - so I guess there is a reason for it
        //this generate random crash when disconnected
        try {
            usb_host_lib_handle_events(portMAX_DELAY, &event_flags);
        } catch (...) {
            esp3d_log("Error handling usb event");
        }
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
            if (ESP_OK!=usb_host_device_free_all()) {
                esp3d_log_e("Failed to free all devices");
            }
        }
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE) {
            esp3d_log("USB: All devices freed");
            // Continue handling USB events to allow device reconnection
        }
    }
}

esp_err_t usb_serial_init()
{
    const usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    //Install USB Host driver. Should only be called once in entire application
    esp3d_log("Installing USB Host");
    if (usb_host_install(&host_config)!=ESP_OK) {
        esp3d_log_e("Failed to install USB Host");
        return ESP_FAIL;
    };
    // Create a task that will handle USB library events
    TaskHandle_t xHandle = NULL;
    BaseType_t  res =  xTaskCreatePinnedToCore(usb_lib_task, "usb_lib", ESP3D_USB_SERIAL_TASK_SIZE, NULL, ESP3D_USB_SERIAL_TASK_PRIORITY, &xHandle, ESP3D_USB_SERIAL_TASK_CORE);
    if (res==pdPASS && xHandle) {
        esp3d_log("Installing CDC-ACM driver");
        if (cdc_acm_host_install(NULL)==ESP_OK) {
            // Register VCP drivers to VCP service.
            esp3d_log("Registering FT23x driver");
            esp_usb::VCP::register_driver<esp_usb::FT23x>();
            esp3d_log("Registering CP210x driver");
            esp_usb::VCP::register_driver<esp_usb::CP210x>();
            esp3d_log("Registering CH34x driver");
            esp_usb::VCP::register_driver<esp_usb::CH34x>();
            return ESP_OK;
        } else {
            esp3d_log_e("Failed to install CDC-ACM driver");
        }
    } else {
        esp3d_log_e("Failed to create task USB Host");
    }

    return ESP_FAIL;
}

#ifdef __cplusplus
}
#endif