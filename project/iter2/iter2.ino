#pragma GCC optimize ("O3")

#define RAW_BITS_LEN   128
#define OUTPUT_LEN     (RAW_BITS_LEN / 2)
#define SEED_LEN       (RAW_BITS_LEN + OUTPUT_LEN - 1)
#define OUTPUT_BYTES   (OUTPUT_LEN / 8)
#define SERIAL_BAUD    6000000
#define TIMED          0

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
    seed_bits[i] = lfsr & 1u;
    bool feedback = ((lfsr >> 0) ^ (lfsr >> 1) ^ (lfsr >> 21) ^ (lfsr >> 31)) & 1u;
    lfsr = (lfsr >> 1) | (feedback << 31);
  }
}

void toeplitz_extraction_optimized(uint64_t raw0, uint64_t raw1, uint8_t* out_bytes) {
  memset(out_bytes, 0, OUTPUT_BYTES);

  for (size_t i = 0; i < OUTPUT_LEN; ++i) {
    uint64_t seed0 = 0;
    uint64_t seed1 = 0;

    for (size_t k = 0; k < 64; ++k) {
      seed0 |= static_cast<uint64_t>(seed_bits[i + k])      << (63 - k);
      seed1 |= static_cast<uint64_t>(seed_bits[i + 64 + k]) << (63 - k);
    }

    int sum = __builtin_popcountll(raw0 & seed0) + __builtin_popcountll(raw1 & seed1);

    if (sum & 1u) {
      size_t byte_i = i >> 3;
      size_t bit_i  = 7 - (i & 7);
      out_bytes[byte_i] |= (1 << bit_i);
    }
  }
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
      raw_bits[bit_index++] = (b >> bit) & 1u;
    }
  }

  if (bit_index == RAW_BITS_LEN) {
    uint64_t raw0 = raw_bits.to_ullong();
    uint64_t raw1 = (raw_bits >> 64).to_ullong();

    uint8_t out_bytes[OUTPUT_BYTES];

    unsigned long t0 = micros();
    toeplitz_extraction_optimized(raw0, raw1, out_bytes);
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
      SERIAL_MAIN.write(out_bytes, OUTPUT_BYTES);  // ✅ exactly 8 bytes
    }

    bit_index = 0;
    raw_bits.reset();
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
}
