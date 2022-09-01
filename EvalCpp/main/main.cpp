#include <iostream>
#include "main.h"

void MyApp::run(int i)
{
    std::cout << "Hello world from " << i << std::endl;
    vTaskDelay(pdMS_TO_TICKS(1000));
}

extern "C" void app_main(void)
{
    MyApp app;
    int i = 0;
    while (true) {
        app.run(i);
        i++;
    }
}
