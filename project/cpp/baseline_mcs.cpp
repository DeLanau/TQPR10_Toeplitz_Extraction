#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <chrono>
#include <string>
#include <iomanip>

// parameters
constexpr size_t RAW_BITS_LEN = 1024;
constexpr size_t OUTPUT_LEN   = RAW_BITS_LEN / 2;
constexpr size_t SEED_LEN     = RAW_BITS_LEN + OUTPUT_LEN - 1;

// global seed storage
static std::vector<int> seed_bits;

void initSeed() {
  seed_bits.reserve(SEED_LEN);
  uint32_t lfsr = 0xBEEF1234u;
  for (size_t i = 0; i < SEED_LEN; ++i) {
    seed_bits.push_back(lfsr & 1);
    bool feedback = ((lfsr >>  0) ^
                     (lfsr >>  1) ^
                     (lfsr >> 21) ^
                     (lfsr >> 31)) & 1;
    lfsr = (lfsr >> 1) | (uint32_t(feedback) << 31);
  }
}

std::vector<int> toeplitzExtraction(const std::vector<int>& raw) {
  std::vector<int> output(OUTPUT_LEN, 0);
  for (size_t i = 0; i < OUTPUT_LEN; ++i) {
    int sum = 0;
    for (size_t j = 0; j < RAW_BITS_LEN; ++j) {
      sum += raw[j] * seed_bits[i + j];
    }
    output[i] = sum & 1;
  }
  return output;
}

void processFile(const std::string& filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "Error: cannot open " << filename << "\n";
    return;
  }

  const size_t chunkBytes = RAW_BITS_LEN / 8;
  std::vector<char> buffer(chunkBytes);
  std::vector<int> raw_bits;
  raw_bits.reserve(RAW_BITS_LEN);

  uint64_t totalChunks     = 0;
  double   totalTimeS      = 0.0;  // seconds

  while (file.read(buffer.data(), chunkBytes)) {
    raw_bits.clear();
    for (size_t b = 0; b < chunkBytes; ++b) {
      uint8_t byte = static_cast<uint8_t>(buffer[b]);
      for (int bit = 7; bit >= 0; --bit) {
        raw_bits.push_back((byte >> bit) & 1);
      }
    }

    auto t0 = std::chrono::high_resolution_clock::now();
    auto result = toeplitzExtraction(raw_bits);
    auto t1 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> dt = t1 - t0;
    totalTimeS += dt.count();
    ++totalChunks;

    // (Optional) do something with 'result' here
  }

  if (totalChunks == 0) {
    std::cerr << "No full " << chunkBytes << "-byte chunks in file\n";
    return;
  }

  double avgTimeS       = totalTimeS / totalChunks;
  double avgTimeUs      = avgTimeS * 1e6;
  double inThroughput   = (RAW_BITS_LEN  / avgTimeS) / 1e6;  // Mbit/s
  double outThroughput  = (OUTPUT_LEN    / avgTimeS) / 1e6;  // Mbit/s

  std::cout << "Chunks processed       : " << totalChunks    << "\n"
            << "Avg extraction time    : " << std::fixed
            << std::setprecision(8) << avgTimeUs << " µs\n"
            << "Input throughput       : " << inThroughput  << " Mbit/s\n"
            << "Output throughput      : " << outThroughput << " Mbit/s\n";
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <input_file.bin>\n";
    return 1;
  }
  initSeed();
  processFile(argv[1]);
  return 0;
}
