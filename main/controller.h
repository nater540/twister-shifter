/*******************************************************************
 * Twister Shifter
 *
 */

#pragma once

////////////////////////////////////////////////////////////////////

#include <string>
#include <tuple>

#include <NimBLEHIDDevice.h>
#include <NimBLECharacteristic.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

////////////////////////////////////////////////////////////////////

/**
 * Buttons that this controller supports.
 */
enum class Button : uint8_t {
  One   = 0x1,
  Two   = 0x2,
  Three = 0x3,
  Four  = 0x4,
  Five  = 0x5,
  Six   = 0x6
};

////////////////////////////////////////////////////////////////////

class Controller : public NimBLEServerCallbacks {
  public:

    Controller(const std::string name = "Twister's Shifter", const std::string manufacturer = "Bounce");
    virtual ~Controller() {};

    /**
     * Trigger a button press.
     * @param [in] btn Button that has been pressed.
     */
    void press(Button btn = Button::One);

    /**
     * Trigger a button release.
     * @param [in] btn Button that has been released.
     */
    void release(Button btn = Button::One);

    /**
     * Resets the internal buffer that maintains button states.
     */
    void resetButtons();

    /**
     * Writes the current button state to the BLE device.
     */
    void sendReport();

  public:

    /**
     * @return The configured device name.
     */
    inline const std::string getDeviceName() const {
      return m_name;
    }

    /**
     * @return The configured device manufacturer.
     */
    inline const std::string getManufacturer() const {
      return m_manufacturer;
    }

    /**
     * @return The configured report id.
     */
    inline uint8_t getReportId() const {
      return m_hidReportId;
    }

    /**
     * @return The current battery level.
     */
    inline uint8_t getBatteryLevel() const {
      return m_batteryLevel;
    }

    /**
     * Sets the current battery level.
     * @param [in] level
     */
    inline void setBatteryLevel(uint8_t level) {
      m_batteryLevel = level;
    }

    /**
     * @return Tuple containing the report descriptor & the descriptor size.
     */
    inline std::tuple<uint8_t*, int> getReportDescriptor() {
      return { m_hidReportDescriptor, m_hidReportDescriptorSize };
    }

  public:

    /**
     * Handler for bluetooth connections.
     */
    inline void onConnect(NimBLEServer* pServer) override {
      m_connected = true;
    }

    /**
     * Handler for bluetooth disconnects.
     */
    inline void onDisconnect(NimBLEServer* pServer) override {
      m_connected = false;
    }

  protected:

    /**
     * Does the grunt work of setting everything up.
     */
    void initialize();

    /**
     * BLE event handler started inside `initialize()`
     */
    static void taskHandler(void* pvParameter);

  protected:

    /**
     * Helper function to pass pointer ownership back to the parent controller class.
     * @param [in] hid HID pointer.
     * @param [in] gamepad BLE characteristic pointer.
     * @param [in] server BLE server pointer.
     */
    inline void setBluetoothHandles(NimBLEHIDDevice* hid, NimBLECharacteristic* gamepad, NimBLEServer* server) {
      m_gamepad = gamepad;
      m_server  = server;
      m_hid     = hid;
    }

  protected:

    const std::string m_name;
    const std::string m_manufacturer;

    bool m_connected = false;

    // 8 bytes -> 8 potential buttons
    uint8_t m_buttons[1];
    uint8_t m_reportSize   = 8;
    uint8_t m_hidReportId  = 3;
    uint8_t m_batteryLevel = 100;

    uint8_t* m_hidReportDescriptor;

    int m_hidReportDescriptorSize = 0;

    xTaskHandle m_handle;

    NimBLEServer* m_server;
    NimBLEHIDDevice* m_hid;
    NimBLECharacteristic* m_gamepad;
};
