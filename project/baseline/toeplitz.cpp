#include <chrono>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace std;

enum LOG_LEVEL {
  LOG_HIGH,
  LOG_MED,
  LOG_LOW
};

constexpr int NO_ITERATIONS{1000};
constexpr LOG_LEVEL DEBUG{LOG_MED};

// Needs at least m + n - 1 bits in seed where:
//    m is the output
//    n is the size of raw bits
vector<bool> toeplitz_extraction(const vector<bool> &raw_bits,
                                 const vector<bool> &seed_bits,
                                 size_t output_len) {
  size_t n{raw_bits.size()};
  size_t m{output_len};

  if (seed_bits.size() < m + n - 1) {
    throw runtime_error{"Seed is smaller than m + n - 1"};
  }

  vector<bool> output(m, false);

  for (size_t i{0}; i < m; i++) {
    bool sum = false;
    for (size_t j{0}; j < n; j++) {
      if (raw_bits[j] && seed_bits[i + j]) {
        sum = !sum;
      }
    }
    output[i] = sum;
  }

  return output;
}

int main() {
  vector<bool> raw_bits{true, false, true, true, false, true, false, false};

  // Simulating seed bits rather than fetching from real source
  // This should be fetched from sampled data later
  // Constant or variable seed?
  vector<bool> seed_bits{true, false, true,  false, true, false,
                         true, true,  false, true,  false};

  chrono::nanoseconds total_duration{0};

  for (int i{0}; i < NO_ITERATIONS; i++) {
    auto start{chrono::high_resolution_clock::now()};

    vector<bool> extracted_bits{toeplitz_extraction(raw_bits, seed_bits, 4)};

    auto end{chrono::high_resolution_clock::now()};
    auto duration{chrono::duration_cast<chrono::nanoseconds>(end - start)};
    if (DEBUG == LOG_HIGH) {
      cout << "Iteration " << i << ": " << duration.count() << " ns." << endl;
    } else if (DEBUG == LOG_MED) {
      if (duration.count() > 300) {
        cout << "Iteration " << i << ": " << duration.count() << " ns." << endl;
      }
    }
    total_duration += duration;
  }

  cout << "Average execution time: " << total_duration.count() / NO_ITERATIONS
       << endl;
  return 0;
}
