TARGET = firmware_singlecore
BUILD_DIR = target/release
HEX_FILE = $(BUILD_DIR)/$(TARGET).hex
MCU = TEENSY41

TEENSY_LOADER = teensy_loader_cli

all: build flash

build:
	cargo build --release

hex: build
	cargo objcopy --release -- -O ihex $(BUILD_DIR)/$(TARGET).hex

flash: hex
	$(TEENSY_LOADER) -v -w -mmcu=$(MCU) $(HEX_FILE)

clean:
	cargo clean

debug:
	cargo build

.PHONY: all build hex flash clean debug

