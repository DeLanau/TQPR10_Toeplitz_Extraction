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
#include <string.h>

//should be ok for input and output range up to 4096, whats our goal? should we utilize all avaible SRAM or not?
//if 512 or 256 is ok we can save more memory, should we try to save memory ?  

extern "C" char* sbrk(int incr);
int freeMemory(){
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}

uint8_t seed_bits[SEED_LEN];
uint8_t raw_bits[RAW_BITS_LEN];
uint8_t output_bits[OUTPUT_LEN];
uint16_t raw_index = 0; 

bool harvest_seed() {
  static uint16_t seed_index = 0;
  while(SERIAL_MAIN.available() && seed_index < SEED_LEN) {  
    char c = SERIAL_MAIN.read();
    if (c == '0' || c == '1') {
      seed_bits[seed_index++] = c - '0';
    }
  }
  return seed_index == SEED_LEN;
}

//this code uses 1 serial as both input and output, how good is it? what the limitations? speed? 
//tested on windows (meh, i know) using arduino IDE, needs implemintation for mac/linux
//BTW!!! filename and foldername needs to be same if using arduino IDE
/**
void toeplitz_extraction() {
  static std::unordered_map<uint32_t, uint8_t> cache;  // key: hash of row, value: sum mod 2

  for (uint16_t i = 0; i < OUTPUT_LEN; ++i) {
    uint32_t hash_key = 0;
    uint8_t sum = 0;

    for (uint16_t j = 0; j < RAW_BITS_LEN; ++j) {
      if (raw_bits[j] && seed_bits[i + j]) {
        sum += 1;
        hash_key += ((i + j + 1) * 31 + raw_bits[j] * 7);  // simple rolling hash-like thing
      }
    }

    if (cache.count(hash_key)) {
      output_bits[i] = cache[hash_key];
    } else {
      uint8_t bit_result = sum % 2;
      output_bits[i] = bit_result;
      cache[hash_key] = bit_result;
    }
  }
}*/ 
//watahell ? from 700ish microsec to 2200-3000 microsec

void toeplitz_extraction() {
  for (uint16_t i = 0; i < OUTPUT_LEN; ++i) {
    uint8_t sum = 0;
    for (uint16_t j = 0; j < RAW_BITS_LEN; ++j) {
      if (raw_bits[j] && seed_bits[i + j]) {
        sum += 1;
      }
    }
    output_bits[i] = sum & 1; //is it faster than modulo?
  }
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
      raw_bits[raw_index++] = c - '0';
    }

    if (raw_index == RAW_BITS_LEN) {
      unsigned long start = micros();
      toeplitz_extraction();
      unsigned long time = micros() - start;
      SERIAL_MAIN.print("out:");
      for (uint16_t i = 0; i < OUTPUT_LEN; i++) {
        SERIAL_MAIN.print(output_bits[i]);
      }
      SERIAL_MAIN.println(" (took ");
      SERIAL_MAIN.print(time);
      SERIAL_MAIN.println(" µs)");
      SERIAL_MAIN.println("RAM: ");
      SERIAL_MAIN.print(freeMemory());
      SERIAL_MAIN.println(" bytes");
      raw_index = 0;
    }
  }
}
