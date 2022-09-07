/* ADC1 Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "bat_adc.h"

static const char *TAG = "bat_adc";

//ADC Channels
#define ADC_EXAMPLE_CHAN0    ADC2_CHANNEL_7   // GPIO18

//ADC Attenuation
#define ADC_EXAMPLE_ATTEN    ADC_ATTEN_DB_11

//ADC Calibration
#if CONFIG_IDF_TARGET_ESP32
#define ADC_EXAMPLE_CALI_SCHEME ESP_ADC_CAL_VAL_EFUSE_VREF
#elif CONFIG_IDF_TARGET_ESP32S2
#define ADC_EXAMPLE_CALI_SCHEME ESP_ADC_CAL_VAL_EFUSE_TP
#elif CONFIG_IDF_TARGET_ESP32C3
#define ADC_EXAMPLE_CALI_SCHEME ESP_ADC_CAL_VAL_EFUSE_TP
#elif CONFIG_IDF_TARGET_ESP32S3
#define ADC_EXAMPLE_CALI_SCHEME ESP_ADC_CAL_VAL_EFUSE_TP_FIT
#endif

static esp_adc_cal_characteristics_t adc_chars;

static bool adc_calibration_init(void)
{
    esp_err_t ret;
    bool cali_enable = false;

    ret = esp_adc_cal_check_efuse(ADC_EXAMPLE_CALI_SCHEME);
    if (ret == ESP_ERR_NOT_SUPPORTED)
    {
        ESP_LOGW(TAG, "Calibration scheme not supported, skip software calibration");
    }
    else if (ret == ESP_ERR_INVALID_VERSION)
    {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    }
    else if (ret == ESP_OK)
    {
        cali_enable = true;
        esp_adc_cal_characterize(ADC_UNIT_2, ADC_EXAMPLE_ATTEN, ADC_WIDTH_BIT_DEFAULT, 0, &adc_chars);
    }
    else
    {
        ESP_LOGE(TAG, "Invalid arg");
    }

    return cali_enable;
}

esp_err_t adc_init(void)
{
    bool cali_enable = adc_calibration_init();
    if (true != cali_enable)
    {
        ESP_LOGI(TAG, "adc init fail!!!");
        return ESP_FAIL;
    }
    
    //ADC2 config
    ESP_ERROR_CHECK(adc2_config_channel_atten(ADC_EXAMPLE_CHAN0, ADC_EXAMPLE_ATTEN));

    ESP_LOGI(TAG, "adc init ok");
    return ESP_OK;
}

uint16_t adc_get_voltage(void)
{
    uint16_t adc_raw;
    uint16_t voltage = 0;

    adc2_get_raw(ADC_EXAMPLE_CHAN0, ADC_WIDTH_BIT_DEFAULT, &adc_raw);
    voltage = esp_adc_cal_raw_to_voltage(adc_raw, &adc_chars);

    // ESP_LOGI(TAG, "adc_raw= %d cali_data= %dmV", adc_raw, voltage);

    return voltage;
}


