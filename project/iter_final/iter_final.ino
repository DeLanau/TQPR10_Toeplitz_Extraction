#pragma GCC optimize ("O3")

#define RAW_BITS_LEN   64
#define OUTPUT_LEN     (RAW_BITS_LEN / 2)
#define SERIAL_BAUD    6000000
#define SEED_LEN       (RAW_BITS_LEN + OUTPUT_LEN - 1)
#define TIMED          1

#if defined(ARDUINO_TEENSY41)
  #define SERIAL_MAIN  Serial
  #define LED_PIN      13
#elif defined(ARDUINO_ESP32_DEV)
  #define SERIAL_MAIN  Serial
  #define LED_PIN      13
#elif defined(ARDUINO_RASPBERRY_PI_PICO_2)
  #define SERIAL_MAIN  Serial
  #define LED_PIN      25
#else
  #error Unsupported MCU
#endif

#include <Arduino.h>
#include <bitset>

std::bitset<SEED_LEN> seed_bits;
std::bitset<RAW_BITS_LEN> raw_bits;

void init_seed() {
  uint32_t lfsr = 0xBEEF1234u;
  for (size_t i = 0; i < SEED_LEN; i++) {
    seed_bits[i] = lfsr & 1;
    bool feedback = ((lfsr >> 0) ^ (lfsr >> 1) ^ (lfsr >> 21) ^ (lfsr >> 31)) & 1;
    lfsr = (lfsr >> 1) | (uint32_t(feedback) << 31);
  }
}

std::bitset<OUTPUT_LEN> toeplitz_extraction(const std::bitset<RAW_BITS_LEN>& raw) {
  std::bitset<OUTPUT_LEN> output;
  for (size_t i = 0; i < OUTPUT_LEN; i++) {
    int sum = 0;
    for (size_t j = 0; j < RAW_BITS_LEN; j++) {
      sum += raw[j] * seed_bits[i + j];
    }
    output[i] = sum % 2;
  }
  return output;
}

void setup() {
  SERIAL_MAIN.begin(SERIAL_BAUD);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  while (!SERIAL_MAIN) {}

  init_seed();
}

void loop() {
  static size_t bit_index = 0;

  while (SERIAL_MAIN.available() && bit_index < RAW_BITS_LEN) {
    int b = SERIAL_MAIN.read();
    for (int bit = 7; bit >= 0 && bit_index < RAW_BITS_LEN; --bit) {
      raw_bits[bit_index++] = (b >> bit) & 1;
    }
  }

  if (bit_index == RAW_BITS_LEN) {
    unsigned long t0 = micros();
    auto result = toeplitz_extraction(raw_bits);
    unsigned long dt = micros() - t0;

    if (TIMED) {
      uint8_t dt_bytes[4] = {
        uint8_t((dt >> 24) & 0xFF),
        uint8_t((dt >> 16) & 0xFF),
        uint8_t((dt >>  8) & 0xFF),
        uint8_t((dt >>  0) & 0xFF)
      };
      SERIAL_MAIN.write(dt_bytes, sizeof(dt_bytes));
    } else {
      const int BYTES = OUTPUT_LEN / 8;
      uint8_t out_bytes[BYTES] = {0};
      for (int i = 0; i < OUTPUT_LEN; ++i) {
        if (result[i]) {
          int byte_i = i >> 3;
          int bit_i  = 7 - (i & 7);
          out_bytes[byte_i] |= (1 << bit_i);
        }
      }
      SERIAL_MAIN.write(out_bytes, BYTES);
    }

    raw_bits.reset();
    bit_index = 0;
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
}
