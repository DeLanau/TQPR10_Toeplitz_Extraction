MCU ?= teensy41
PORT         ?= $(shell arduino-cli board list | awk '/Teensy.*Teensy Ports/ {print $$1; exit}')
FQBN_TEENSY  := teensy:avr:$(MCU)
FQBN_ESP32 ?= esp32:esp32:esp32
FQBN_PICO ?= rp2040:rp2040:rpipico2

SKETCH_PATH ?= ./src/main.ino

upload_teensy:
	arduino-cli compile --fqbn $(FQBN_TEENSY) --output-dir build .
	arduino-cli upload --fqbn $(FQBN_TEENSY) -p $(PORT) .

upload_esp32:
	arduino-cli compile --fqbn $(FQBN_ESP32) --output-dir build .	
	arduino-cli upload -p /dev/ttyUSB0 --fqbn $(FQBN_ESP32) --input-dir build

upload_pico: build/$(notdir $(CURDIR)).ino.uf2
	@echo "✅ Sketch compiled."
	@echo "👉 Please hold BOOTSEL, plug Pico, and copy:"
	@echo "   build/$(notdir $(CURDIR)).ino.uf2 → RPI-RP2 drive."

build/$(notdir $(CURDIR)).ino.uf2:
	arduino-cli compile --fqbn $(FQBN_PICO) --output-dir build .

clean:
	rm -rf build
