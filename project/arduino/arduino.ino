#if defined(ARDUINO_TEENSY41)
  #define MCU_NAME "Teensy 4.1"
  #define SERIAL_MAIN Serial
  #define SERIAL_ALT SerialUSB
  #define LED_PIN 13
#elif defined(ARDUINO_ESP32_DEV)
  #define MCU_NAME "ESP32"
  #define SERIAL_MAIN Serial
  #define SERIAL_ALT Serial1 
  #define LED_PIN 13
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
  #define MCU_NAME "Raspberry Pi Pico"
  #define SERIAL_MAIN Serial
  #define SERIAL_ALT Serial1 
  #define LED_PIN 25
#else
  #error "Unsupported MCU"
#endif

void setup() {
    SERIAL_MAIN.begin(115200);
    SERIAL_ALT.begin(115200);
    delay(1000);
    SERIAL_MAIN.println("Starting " MCU_NAME " dual-serial test");
    SERIAL_ALT.println("[ALT] Starting dual-serial test");
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
}

void loop() {
    static unsigned long lastMillis = 0;
    if (millis() - lastMillis >= 1000) {
        lastMillis = millis();
        SERIAL_MAIN.print("Main Serial: ");
        SERIAL_MAIN.println(millis());
        SERIAL_ALT.print("Alt Serial: ");
        SERIAL_ALT.println(millis());
    }
}
