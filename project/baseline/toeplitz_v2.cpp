#include <iostream>
#include <stdexcept>
#include <vector>

using namespace std;

//raw bits - input from entropy source 
//seed - a part of entropy, is it good?
vector<int> toeplitz_extraction(const vector<int> &raw_bits,
                                const vector<int> &seed_bits,
                                size_t output_len) {
  size_t n = raw_bits.size();
  size_t m = output_len;

  if (seed_bits.size() < m + n - 1) {
    throw runtime_error("Seed is smaller than m + n - 1");
  }

  vector<int> output(m, 0);

  for (size_t i = 0; i < m; i++) {
    int sum = 0;
    for (size_t j = 0; j < n; j++) {
      sum += raw_bits[j] * seed_bits[i + j];
    }
    output[i] = sum % 2;
  }

  return output;
}

//now, how to make it real time? 
int main() {
  vector<int> raw_bits{1, 0, 1, 1, 0, 1, 0, 0};

  vector<int> seed_bits{1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0};

  vector<int> extracted_bits = toeplitz_extraction(raw_bits, seed_bits, 4);

  cout << "Extracted bits: ";
  for (int bit : extracted_bits) {
    cout << bit;
  }
  cout << endl;

  return 0;
}

