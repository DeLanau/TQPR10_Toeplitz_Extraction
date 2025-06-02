# User manual

## Prerequisites

Before using the script, make sure that:

- Python 3 is installed
- Required Python modules (pyserial, argparse, glob, subprocess) is installed
- The MCU is connected to your system via USB
- You know the serial port (/dev/ttyACM0, COM7, etc)

## Microcontroller Environment

You can use either:

- Arduino CLI (recommended for linux)
- Arduino IDE (recommended for Windows/MacOS)

If you choose Arduino CLI, you will need following:

- Install Arduino CLI
- Add board URLs (Teensy and Raspberry Pi Pico2)

```bash
arduino-cli config init
arduino-cli config add board_manager.additional_urls \
  https://www.pjrc.com/teensy/package_teensy_index.json, \
  https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
```

- Update Index and Install Cores

```bash
arduino-cli core update-index
arduino-cli core install teensy:avr
arduino-cli core install rp2040:rp2040
```

- Flash Firmware using Makefile (make upload_teensy or make upload_pico)

## Script Usage

```bash
python tests.py --gen
python tests.py --test
python tests.py --time
```

## File Structure

- data/ - Input binary test files (pre sampled/generated entory data)
- results/{output_size}/ - Output binaries and entropy reports based on output
  size of bits
- logs/iter{iteration}/{output_size}/ - Timing logs for each test iteration and
  output size of bits

## Detailed Description of the Test Scipt Usage

The developed test script is designed as a comprehensive tool to communicate
with MCUs. Its primary purpose is to automate testing, timing measurements and
result generation with minimal manual intervention. Thus, ensuring
reproducibility and accuracy, which are crucial for both evaluation and
correctness verification of the implementation. The script achives is by:

- Automating input data transmission via USB serial
- Capturing and verifying processed data output from MCUs
- Logging detailed timing information as well as displaying average timings
- Generating entropy reports via ent, to further evaluate correctness of the
  extraction

This approach minimizes human error, allows for consistent benchmarkjing across
different iterations and different output bit sizes.

### Step-by-Step Execution

1. Connect the Hardware

- Connect the microcontroller to the PC via USB
- Verify that it appears as a serial port (/dev/ttyACM0 on linux or COM7 on
  Windows)

2. Prepare the Microcontroller

- Flash appropriate firmware onto the Teensy 4.1 or Raspberry Pi Pico2 using
  Arduino CLI or Arduino IDE (as detailöed erlier)
- Ensure the firmware matches the intended test mode (DEBUG= 0/1, depending on
  this mode MCUs either produce bits or timings)

3. Run the Script

- Open the terminal and navigate to the directory containing the script
- Run python tests.py --gen using naive iteration to generate baseline for
  output bits size
- Do step 1 and 2 to flash new iteration
- Run python tests.py --test to validate correctnes of the Toeplitz extraction
- Change mode to gather timings
- Run python tests.py --time to gather timing data

If all step is done correctly depending on DEBUG mode, console will output
validation data or average timings for iteration and different datasets.
