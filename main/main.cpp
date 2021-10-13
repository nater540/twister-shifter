
#include <stdio.h>
#include <memory>
#include <ostream>
#include <iostream>
#include "sdkconfig.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_spi_flash.h>
#include <driver/gpio.h>
#include <esp_event.h>

#include "NimBLEDevice.h"
#include "NimBLEHIDDevice.h"

#include "button.h"
#include "singleton.h"

extern "C" {
  void app_main(void);
}

class BleConnectionStatus : public NimBLEServerCallbacks
{
public:
  BleConnectionStatus(void);
  bool connected = false;
  void onConnect(NimBLEServer* pServer);
  void onDisconnect(NimBLEServer* pServer);
  NimBLECharacteristic* inputGamepad;
};

BleConnectionStatus::BleConnectionStatus(void) {}

void BleConnectionStatus::onConnect(NimBLEServer* pServer) {
  this->connected = true;
}

void BleConnectionStatus::onDisconnect(NimBLEServer* pServer) {
  this->connected = false;
}

class Application : public Singleton<Application> {
public:
  Application()  {
    m_connection_status = new BleConnectionStatus();
  }

  ~Application() { }

  BleConnectionStatus* connection_status() {
    return m_connection_status;
  }

  protected:

  BleConnectionStatus* m_connection_status;
};

static void button_task(void* pParams) {
  button_event_t ev;
  QueueHandle_t button_events = button_init(PIN_BIT(GPIO_NUM_23));
  // gpio_set_pull_mode(GPIO_NUM_23, GPIO_PULLUP_ONLY);

  while (true) {
    if (xQueueReceive(button_events, &ev, 1000 / portTICK_PERIOD_MS)) {
      if ((ev.pin == GPIO_NUM_23) && (ev.event == BUTTON_DOWN)) {
        std::cout << "######### BUTTON PRESSED #########" << std::endl;
      }
    }
  }
}

void app_main(void) {
  esp_event_loop_create_default();

  NimBLEDevice::init("Twister's Shifter");

  NimBLEServer *pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(Application::instance().connection_status());

  auto hid = new NimBLEHIDDevice(pServer);
  hid->manufacturer()->setValue("Bounce Puppy");
  hid->pnp(0x01, 0x02e5, 0xabbb, 0x0110);
  hid->hidInfo(0x00, 0x01);

  NimBLESecurity *pSecurity = new NimBLESecurity();

  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);

  hid->startServices();

  NimBLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->setAppearance(HID_GAMEPAD);
  pAdvertising->addServiceUUID(hid->hidService()->getUUID());
  pAdvertising->start();

  xTaskCreate(button_task, "button_task", 4096, NULL, 0, NULL);
}
