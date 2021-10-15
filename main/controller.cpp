/*******************************************************************
 * Twister Shifter
 *
 */

#include "controller.h"

#include <NimBLEDevice.h>
#include <NimBLEUtils.h>
#include <NimBLEServer.h>
#include <esp_log.h>

////////////////////////////////////////////////////////////////////

static const char* LOG_TAG = "Controller";

#define CONTROLLER_TYPE_GAMEPAD 0x05

////////////////////////////////////////////////////////////////////

Controller::Controller(const std::string name, const std::string manufacturer) :
  m_name(name), m_manufacturer(manufacturer), m_connected(false),
  m_hidReportDescriptor(nullptr), m_handle(nullptr), m_server(nullptr),
  m_hid(nullptr), m_gamepad(nullptr) {

  // Enact the Clean Slate Protocol
  resetButtons();
}

void Controller::press(Button btn) {
  uint8_t butt = static_cast<uint8_t>(btn);

  uint8_t index   = (butt - 1) / 8;
  uint8_t bit     = (butt - 1) % 8;
  uint8_t bitmask = (1 << bit);
  uint8_t result  = m_buttons[index] | bitmask;

  if (result != m_buttons[index]) {
    m_buttons[index] = result;
    sendReport();
  }
}

void Controller::release(Button btn) {
  uint8_t butt = static_cast<uint8_t>(btn);

  uint8_t index   = (butt - 1) / 8;
  uint8_t bit     = (butt - 1) % 8;
  uint8_t bitmask = (1 << bit);
  uint64_t result = m_buttons[index] & ~bitmask;

  if (result != m_buttons[index]) {
    m_buttons[index] = result;
    sendReport();
  }
}

void Controller::resetButtons() {
  memset(&m_buttons, 0, sizeof(m_buttons));
}

void Controller::sendReport() {
  if (!m_connected) {
    return;
  }

  uint8_t buffer[m_reportSize];

  // Reset the buffer & write the current button states
  memset(&buffer, 0, sizeof(buffer));
  memcpy(&buffer, &m_buttons, sizeof(m_buttons));

  m_gamepad->setValue(buffer, sizeof(buffer));
  m_gamepad->notify();
}

void Controller::initialize() {
  assert(m_handle == nullptr);

  uint8_t tempHidReportDescriptor[42];

  // USAGE_PAGE (Generic Desktop)
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x05;
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x01;

  // USAGE (Gamepad)
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x05;
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = CONTROLLER_TYPE_GAMEPAD;

  // COLLECTION (Application)
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0xa1;
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x01;

  // REPORT_ID (Default: 3)
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x85;
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = m_hidReportId;

  // USAGE_PAGE (Button)
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x05;
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x09;

  // USAGE_MINIMUM (Button 1)
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x19;
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x01;

  // USAGE_MAXIMUM (6 Buttons)
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x29;
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 6;

  // LOGICAL_MINIMUM (0)
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x15;
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x00;

  // LOGICAL_MAXIMUM (1)
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x25;
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x01;

  // REPORT_SIZE (1)
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x75;
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x01;

  // REPORT_COUNT (6 Buttons)
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x95;
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 6;

  // UNIT_EXPONENT (0)
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x55;
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x00;

  // UNIT (None)
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x65;
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x00;

  // INPUT (Data, Var, Abs)
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x81;
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0x02;

  // END_COLLECTION (Application)
  tempHidReportDescriptor[m_hidReportDescriptorSize++] = 0xc0;

  // Allocate memory to store the finalized report descriptor
  m_hidReportDescriptor = new uint8_t[m_hidReportDescriptorSize];
  memcpy(m_hidReportDescriptor, tempHidReportDescriptor, m_hidReportDescriptorSize);

  // Make it so number one
  xTaskCreate(&taskHandler, "server", 2000, (void*)this, 5, &m_handle);
}

void Controller::taskHandler(void* pvParameter) {
  Controller* instance = (Controller*)pvParameter;

  // Initialize NimBLE
  NimBLEDevice::init(instance->getDeviceName());
  NimBLEServer* server = NimBLEDevice::createServer();
  server->setCallbacks(instance);

  NimBLEHIDDevice* hid = new NimBLEHIDDevice(server);
  hid->manufacturer()->setValue(instance->getManufacturer());

  // Configure Plug & Play
  hid->pnp(0x01, 0x02e5, 0xabbb, 0x0110);
  hid->hidInfo(0x00, 0x01);

  NimBLECharacteristic* gamepad = hid->inputReport(instance->getReportId());

  // Security is our top priority!
  NimBLESecurity* security = new NimBLESecurity();
  security->setAuthenticationMode(ESP_LE_AUTH_BOND);

  // Set the finalized report descriptor & start the HID services
  auto [reportDescriptor, descriptorSize] = instance->getReportDescriptor();
  hid->reportMap(reportDescriptor, descriptorSize);
  hid->startServices();

  // Start advertising our fabulous presence
  NimBLEAdvertising* advertising = server->getAdvertising();
  advertising->setAppearance(HID_GAMEPAD);
  advertising->addServiceUUID(hid->hidService()->getUUID());
  advertising->start();

  // Everyone loves battery levels, so let's report that too
  hid->setBatteryLevel(instance->getBatteryLevel());

  instance->setBluetoothHandles(
    std::move(hid),
    std::move(gamepad),
    std::move(server)
  );

  ESP_LOGD(LOG_TAG, "Advertising started!");
  vTaskDelay(portMAX_DELAY);
}
