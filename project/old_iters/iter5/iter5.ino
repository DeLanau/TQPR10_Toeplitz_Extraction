#pragma GCC optimize ("O3")

#define RAW_BITS_LEN   64  // Change this to 128, 256, etc.
#define OUTPUT_LEN     (RAW_BITS_LEN / 2)
#define SERIAL_BAUD    6000000
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
#include <vector>
using std::vector;

constexpr size_t SEED_LEN = RAW_BITS_LEN + OUTPUT_LEN - 1;

vector<uint_fast8_t> seed_bits(SEED_LEN);
vector<uint_fast8_t> raw_bits;

void init_seed() {
  uint32_t lfsr = 0xBEEF1234u;
  for (size_t i = 0; i < SEED_LEN; i++) {
    seed_bits[i] = lfsr & 1u;
    bool feedback = ((lfsr >> 0) ^ (lfsr >> 1) ^ (lfsr >> 21) ^ (lfsr >> 31)) & 1u;
    lfsr = (lfsr >> 1) | (uint32_t(feedback) << 31);
  }
}

uint32_t toeplitz_extraction(const uint_fast8_t* raw, const uint_fast8_t* seed) {
  uint32_t output = 0;

  for (size_t i = 0; i < OUTPUT_LEN; i++) {
    int sum = 0;
    for (size_t j = 0; j < RAW_BITS_LEN; j++) {
      sum += raw[j] * seed[i + j];
    }
    if (sum & 1u) {
      output |= (1UL << (OUTPUT_LEN - 1 - i));
    }
  }

  return output;
}

void setup() {
  SERIAL_MAIN.begin(SERIAL_BAUD);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  while (!SERIAL_MAIN) {}
  raw_bits.reserve(RAW_BITS_LEN);
  init_seed();
}

void loop() {
  while (SERIAL_MAIN.available() && raw_bits.size() < RAW_BITS_LEN) {
    int b = SERIAL_MAIN.read();
    for (int bit = 7; bit >= 0 && raw_bits.size() < RAW_BITS_LEN; --bit) {
      raw_bits.push_back((b >> bit) & 1u);
    }
  }

  if (raw_bits.size() == RAW_BITS_LEN) {
    unsigned long t0 = micros();
    uint32_t result = toeplitz_extraction(raw_bits.data(), seed_bits.data());
    unsigned long dt = micros() - t0;

    if (TIMED) {
      uint8_t dt_bytes[4] = {
        uint8_t((dt >> 24) & 0xFF),
        uint8_t((dt >> 16) & 0xFF),
        uint8_t((dt >>  8) & 0xFF),
        uint8_t((dt >>  0) & 0xFF)
      };
      SERIAL_MAIN.write(dt_bytes, 4);
    } else {
      const size_t BYTES = OUTPUT_LEN / 8;
      uint8_t out_bytes[BYTES] = {0};
      for (size_t i = 0; i < OUTPUT_LEN; i++) {
        if (result & (1UL << (OUTPUT_LEN - 1 - i))) {
          size_t byte_i = i >> 3;
          size_t bit_i = 7 - (i & 7);
          out_bytes[byte_i] |= (1 << bit_i);
        }
      }
      SERIAL_MAIN.write(out_bytes, BYTES);
    }

    raw_bits.clear();
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
}

