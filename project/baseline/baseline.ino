#define RAW_BITS_LEN   64
#define OUTPUT_LEN     32
#define SERIAL_BAUD    6000000
#define SEED_LEN       (RAW_BITS_LEN + OUTPUT_LEN - 1)
#define TIMED          0

#if defined(ARDUINO_TEENSY41)
  #define SERIAL_MAIN  Serial
  #define LED_PIN      13
#elif defined(ARDUINO_ESP32_DEV)
  #define SERIAL_MAIN  Serial
  #define LED_PIN      13
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
  #define SERIAL_MAIN  Serial
  #define LED_PIN      25
#else
  #error Unsupported MCU
#endif

#include <Arduino.h>
#include <vector>
using std::vector;

const int seed_bits[SEED_LEN] = {
  1,0,1,1,0,1,0,0, 1,1,1,0,1,0,0,1,
  0,1,1,0,1,1,0,0, 1,0,1,0,0,1,1,1,
  0,1,0,1,1,1,1,0, 0,0,1,0,1,1,0,1,
  1,0,0,1,1,1,0,0, 1,0,1,0,1,1,0,0,
  1,1,1,0,0,0,1,1, 0,1,0,0,1,0,0,1,
  1,0,0,1,1,1,1,1, 0,0,1,0,1,0,1
};

vector<int> raw_bits;

vector<int> toeplitz_extraction(const vector<int>& raw) {
  vector<int> output(OUTPUT_LEN, 0);
  for (size_t i = 0; i < OUTPUT_LEN; i++) {
    int sum = 0;
    for (size_t j = 0; j < RAW_BITS_LEN; j++) {
      sum += raw[j] * seed_bits[i + j];
    }
    output[i] = sum & 1;
  }
  return output;
}

void setup() {
  SERIAL_MAIN.begin(SERIAL_BAUD);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  while (!SERIAL_MAIN) {}
}

void loop() {
  while (SERIAL_MAIN.available() && raw_bits.size() < RAW_BITS_LEN) {
    int b = SERIAL_MAIN.read();
    for (int bit = 7; bit >= 0; bit--) {
      raw_bits.push_back((b >> bit) & 1);
    }
  }

  if (raw_bits.size() == RAW_BITS_LEN) {
    // measure or compute
    if (TIMED) {
      unsigned long t0 = micros();
      toeplitz_extraction(raw_bits);
      unsigned long dt = micros() - t0;

      uint8_t dt_bytes[4] = {
        uint8_t((dt >> 24) & 0xFF),
        uint8_t((dt >> 16) & 0xFF),
        uint8_t((dt >>  8) & 0xFF),
        uint8_t((dt >>  0) & 0xFF)
      };
      SERIAL_MAIN.write(dt_bytes, sizeof(dt_bytes));

    } else {
      auto result = toeplitz_extraction(raw_bits);
      uint8_t out_bytes[OUTPUT_LEN / 8] = {0};
      for (int i = 0; i < OUTPUT_LEN; i++) {
        int byte_i = i >> 3;
        int bit_i  = 7 - (i & 7);
        out_bytes[byte_i] |= result[i] << bit_i;
      }
      SERIAL_MAIN.write(out_bytes, sizeof(out_bytes));
    }

    raw_bits.clear();
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
}