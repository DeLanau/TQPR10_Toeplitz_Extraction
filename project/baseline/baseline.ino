#define RAW_BITS_LEN 64
#define OUTPUT_LEN 32
#define SERIAL_BAUD 6000000
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
//vector<int> seed_bits;

const int seed_bits[SEED_LEN] = {
  1,0,1,1,0,1,0,0, 1,1,1,0,1,0,0,1,
  0,1,1,0,1,1,0,0, 1,0,1,0,0,1,1,1,
  0,1,0,1,1,1,1,0, 0,0,1,0,1,1,0,1,
  1,0,0,1,1,1,0,0, 1,0,1,0,1,1,0,0,
  1,1,1,0,0,0,1,1, 0,1,0,0,1,0,0,1,
  1,0,0,1,1,1,1,1, 0,0,1,0,1,0,1
};

/*bool harvest_seed() {
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
}*/

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
    output[i] = sum & 1;
  }
  return output;
}

void setup() {
  SERIAL_MAIN.begin(SERIAL_BAUD);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
}

void loop() {
  while (SERIAL_MAIN.available() && raw_bits.size() < RAW_BITS_LEN) {
    int b = SERIAL_MAIN.read(); // reads raw byte 0 - 255
    for (int i = 7; i >= 0; i--) {
      raw_bits.push_back((b >> i) & 1);
    }
  }

  if (raw_bits.size() == RAW_BITS_LEN) {
    auto result = toeplitz_extraction(raw_bits);

    // pack 32 bits into 4 bytes
    uint8_t out_bytes[OUTPUT_LEN / 8] = {0};
    for (int i = 0; i < OUTPUT_LEN; i++) {
      int byte_i = i >> 3;
      int bit_i  = 7 - (i & 7);
      out_bytes[byte_i] |= result[i] << bit_i;
    }

    SERIAL_MAIN.write(out_bytes, sizeof(out_bytes));
    raw_bits.clear();
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
}

