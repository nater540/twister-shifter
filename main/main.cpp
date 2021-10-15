
#include <stdio.h>
#include <memory>
#include <ostream>
#include <iostream>

#include "sdkconfig.h"
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
#include <esp_event.h>

#include "singleton.h"
#include "controller.h"

extern "C" {
  void app_main(void);
}

class Application : public Singleton<Application> {
public:
  Application() {}
  ~Application() {}
};

// static void button_task(void* pParams) {
//   button_event_t ev;
//   QueueHandle_t button_events = button_init(PIN_BIT(BUTTON_1_PIN) | PIN_BIT(BUTTON_2_PIN) | PIN_BIT(BUTTON_3_PIN) | PIN_BIT(BUTTON_4_PIN) | PIN_BIT(BUTTON_5_PIN) | PIN_BIT(BUTTON_6_PIN));

//   while (true) {
//     if (xQueueReceive(button_events, &ev, 1000 / portTICK_PERIOD_MS)) {
//       if ((ev.pin == BUTTON_1_PIN) && (ev.event == BUTTON_DOWN)) {
//         std::cout << "######### BUTTON 1 PRESSED #########" << std::endl;
//       }

//       if ((ev.pin == BUTTON_2_PIN) && (ev.event == BUTTON_DOWN)) {
//         std::cout << "######### BUTTON 2 PRESSED #########" << std::endl;
//       }

//       if ((ev.pin == BUTTON_3_PIN) && (ev.event == BUTTON_DOWN)) {
//         std::cout << "######### BUTTON 3 PRESSED #########" << std::endl;
//       }

//       if ((ev.pin == BUTTON_4_PIN) && (ev.event == BUTTON_DOWN)) {
//         std::cout << "######### BUTTON 4 PRESSED #########" << std::endl;
//       }

//       if ((ev.pin == BUTTON_5_PIN) && (ev.event == BUTTON_DOWN)) {
//         std::cout << "######### BUTTON 5 PRESSED #########" << std::endl;
//       }

//       if ((ev.pin == BUTTON_6_PIN) && (ev.event == BUTTON_DOWN)) {
//         std::cout << "######### BUTTON 6 PRESSED #########" << std::endl;
//       }
//     }
//   }
// }

void app_main(void) {
  esp_event_loop_create_default();
}
