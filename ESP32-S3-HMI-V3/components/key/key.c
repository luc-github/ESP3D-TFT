
#include "key.h"

static const char *TAG = "key";


static xQueueHandle gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_interrupt_task(void* arg)
{
    uint32_t io_num;

    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            // printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
            if ((GPIO_KEY == io_num) && (0 == gpio_get_level(GPIO_KEY)))
            {
                ESP_LOGI(TAG, "wake key pressed!");
            }
        }
    }
}

esp_err_t key_init_isr(void)
{
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_INPUT,
        //.intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = 1ULL << GPIO_KEY,
        .pull_down_en = 0,
        .pull_up_en = 1,
    };
    gpio_config(&io_conf);

    gpio_set_intr_type(GPIO_KEY, GPIO_INTR_NEGEDGE);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    xTaskCreate(gpio_interrupt_task, "gpio_interrupt_task", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_KEY, gpio_isr_handler, (void*) GPIO_KEY);

    return ESP_OK;
}


