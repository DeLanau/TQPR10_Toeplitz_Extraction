MCU ?= teensy41
PORT ?= /dev/ttyACM0
FQBN_TEENSY ?= teensy:avr:teensy41
FQBN_ESP32 ?= esp32:esp32:esp32
FQBN_PICO ?= arduino:mbed_rp2040:pico

SKETCH_PATH ?= ./src/main.ino

upload_teensy:
	arduino-cli compile --fqbn $(FQBN_TEENSY) --output-dir build .
	arduino-cli upload -p $(PORT) --fqbn $(FQBN_TEENSY) --input-dir build

upload_esp32:
	arduino-cli compile --fqbn $(FQBN_ESP32) --output-dir build .	
	arduino-cli upload -p /dev/ttyUSB0 --fqbn $(FQBN_ESP32) --input-dir build

upload_pico:
	arduino-cli compile --fqbn $(FQBN_PICO) --output-dir build .
	arduino-cli upload -p $(PORT) --fqbn $(FQBN_PICO) --input-dir build

clean:
	rm -rf build
