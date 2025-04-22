#define RAW_BITS_LEN 512
#define OUTPUT_LEN 256
#define SEED_LEN (RAW_BITS_LEN + OUTPUT_LEN - 1)

#if defined(ARDUINO_TEENSY41)
  #define MCU_NAME "Teensy 4.1"
  #define SERIAL_MAIN Serial
  #define LED_PIN 13
#elif defined(ARDUINO_ESP32_DEV)
  #define MCU_NAME "ESP32"
  #define SERIAL_MAIN Serial
  #define LED_PIN 13
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
  #define MCU_NAME "RP2040"
  #define SERIAL_MAIN Serial
  #define LED_PIN 25
#else
  #error "Unsupported MCU"
#endif

#include <Arduino.h>
#include <vector>
using std::vector;

//use this in future, cuz more optimized version e.g iteration 2 
//int seed_bits[SEED_LEN];
//int seed_index = 0:
vector<int> seed_bits;

bool harvest_seed() {
  while(SERIAL_MAIN.available() && seed_bits.size() < SEED_LEN) {  
    char c = SERIAL_MAIN.read();
    if (c == '0' || c == '1') {
      seed_bits.push_back(c - '0');
    }
  }
  if (seed_bits.size() == SEED_LEN){
    SERIAL_MAIN.println("Done with seed harvesting.");
    return true;
  }
  return false;
}

//this code uses 1 serial as both input and output, how good is it? what the limitations? speed? 
//tested on windows (meh, i know) using arduino IDE, needs implemintation for mac/linux
//BTW!!! filename and foldername needs to be same if using arduino IDE

vector<int> raw_bits;

vector<int> toeplitz_extraction(const vector<int>& raw_bits) {
  vector<int> output(OUTPUT_LEN, 0);
  for (size_t i = 0; i < OUTPUT_LEN; i++) {
    int sum = 0;
    for (size_t j = 0; j < RAW_BITS_LEN; j++) {
      sum += raw_bits[j] * seed_bits[i + j];
    }
    output[i] = sum % 2;
  }
  return output;
}

void setup() {
  SERIAL_MAIN.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  while (!SERIAL_MAIN);
  SERIAL_MAIN.println("[" MCU_NAME "] Streaming Toeplitz extractor ready.");

  SERIAL_MAIN.println("Starting harvesting seed...");
  while(!harvest_seed());
}

void loop() {

  if (SERIAL_MAIN.available()) {
    char c = SERIAL_MAIN.read();
    if (c == '0' || c == '1') {
      raw_bits.push_back(c - '0');
    }

    if (raw_bits.size() == RAW_BITS_LEN) {
      unsigned long start = micros();
      vector<int> result = toeplitz_extraction(raw_bits);
      unsigned long time = micros() - start;
      SERIAL_MAIN.print("out:");
      for (int b : result) {
        SERIAL_MAIN.print(b);
      }
      // SERIAL_MAIN.println(" (took ");
      // SERIAL_MAIN.print(time);
      // SERIAL_MAIN.println(" µs)");
      raw_bits.clear();
    }
  }
}
