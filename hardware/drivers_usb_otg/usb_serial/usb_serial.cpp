/**
 * @file usb_serial.cpp
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "usb_serial.h"

#include "esp3d_log.h"
#include "esp_private/usb_phy.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/usb_phy_types.h"
#include "usb/cdc_acm_host.h"
#include "usb/usb_host.h"
#include "usb/vcp.hpp"
#include "usb/vcp_ch34x.hpp"
#include "usb/vcp_cp210x.hpp"
#include "usb/vcp_ftdi.hpp"
#include "usb_serial_def.h"

#ifdef __cplusplus
extern "C" {
#endif
static usb_phy_handle_t phy_hdl = NULL;
static TaskHandle_t usb_serial_xHandle = NULL;

/**
 * @brief USB Host library handling task
 *
 * @param arg Unused
 */
void usb_lib_task(void *arg) {
  while (1) {
    // Start handling system events
    uint32_t event_flags;
    // exception must be enabled - so I guess there is a reason for it
    // this generate random crash when disconnected
    try {
      usb_host_lib_handle_events(portMAX_DELAY, &event_flags);
    } catch (...) {
      esp3d_log("Error handling usb event");
    }
    if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
      if (ESP_OK != usb_host_device_free_all()) {
        esp3d_log_e("Failed to free all devices");
      }
    }
    if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE) {
      esp3d_log("USB: All devices freed");
      // Continue handling USB events to allow device reconnection
    }
  }
}

esp_err_t usb_serial_deinit() {
  esp_err_t ret;
  if (usb_serial_xHandle) {
    ret = usb_serial_delete_task();
    if (ret != ESP_OK) {
      esp3d_log_e("Failed to delete task for usb-host %s",
                  esp_err_to_name(ret));
    }
    vTaskDelay(10);
  }
  ret = usb_host_device_free_all();
  if (ret != ESP_OK) {
    esp3d_log_e("Failed to free all usb device %s", esp_err_to_name(ret));
  }
  ret = usb_host_uninstall();
  // this one failed with ESP_ERR_INVALID_STATE if USB is connected
  if (ret != ESP_OK) {
    esp3d_log_e("Failed to unsinstall usb host %s",
                esp_err_to_name(usb_host_uninstall()));
  }
  // Deinitialize the internal USB PHY
  ret = usb_del_phy(phy_hdl);
  if (ret != ESP_OK) {
    esp3d_log_e("Failed to delete PHY %s", esp_err_to_name(ret));
  }
  phy_hdl = NULL;
  return ESP_OK;
}

esp_err_t usb_serial_init() {
  usb_phy_config_t phy_config = {
      .controller = USB_PHY_CTRL_OTG,
      .target = USB_PHY_TARGET_INT,
      .otg_mode = USB_OTG_MODE_HOST,
      .otg_speed =
          USB_PHY_SPEED_UNDEFINED,  // In Host mode, the speed is determined by
                                    // the connected device
      .ext_io_conf = NULL,
      .otg_io_conf = NULL,
  };
  if (ESP_OK != usb_new_phy(&phy_config, &phy_hdl)) {
    esp3d_log_e("Failed to init USB PHY");
  }

  const usb_host_config_t host_config = {
      .skip_phy_setup = true,
      .intr_flags = ESP_INTR_FLAG_LEVEL1,
      //.enum_filter_cb = NULL,
  };
  // Install USB Host driver. Should only be called once in entire application
  esp3d_log("Installing USB Host");
  esp_err_t err = usb_host_install(&host_config);
  if (err != ESP_OK) {
    esp3d_log_e("Failed to install USB Host %s", esp_err_to_name(err));
    return ESP_FAIL;
  };
  return ESP_OK;
}

esp_err_t usb_serial_delete_task() {
  esp_err_t err = cdc_acm_host_uninstall();
  if (err != ESP_OK) {
    vTaskDelete(usb_serial_xHandle);
    usb_serial_xHandle = NULL;
    return ESP_OK;
  }
  return err;
}

esp_err_t usb_serial_create_task() {
  // Create a task that will handle USB library events

  BaseType_t res =
      xTaskCreatePinnedToCore(usb_lib_task, "usb_lib", ESP3D_USB_LIB_TASK_SIZE,
                              NULL, ESP3D_USB_LIB_TASK_PRIORITY,
                              &usb_serial_xHandle, ESP3D_USB_LIB_TASK_CORE);
  if (res == pdPASS && usb_serial_xHandle) {
    esp3d_log("Installing CDC-ACM driver");
    if (cdc_acm_host_install(NULL) == ESP_OK) {
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