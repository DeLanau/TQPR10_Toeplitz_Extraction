#![no_std]
#![no_main]

use bsp::board;
use bsp::hal::usbd::{BusAdapter, EndpointMemory, EndpointState};
use teensy4_bsp as bsp;
use teensy4_panic as _;
use usb_device::bus::UsbBusAllocator;
use usb_device::prelude::*;
use usbd_serial::{SerialPort, USB_CLASS_CDC};

#[bsp::rt::entry]
fn main() -> ! {
    let board::Resources {
        usb,
        mut gpio2,
        pins,
        ..
    } = bsp::board::t41(bsp::board::instances());

    usb.enable();

    // ✅ Set up LED for debugging
    let mut led = bsp::board::led(&mut gpio2, pins.p13);

    // ✅ Allocate USB memory & state
    static mut EP_MEMORY: EndpointMemory<1024> = EndpointMemory::new();
    static mut EP_STATE: EndpointState = EndpointState::max_endpoints();

    // ✅ Set up the USB Bus Adapter
    let usb_adapter = unsafe { BusAdapter::new(usb, &mut EP_MEMORY, &mut EP_STATE) };

    // ✅ Allocate USB Bus
    let usb_alloc = UsbBusAllocator::new(usb_adapter);

    // ✅ Create USB Serial device
    let mut serial = SerialPort::new(&usb_alloc);

    // ✅ Set up the USB Device
    let mut usb_dev = UsbDeviceBuilder::new(&usb_alloc, UsbVidPid(0x16c0, 0x0483))
        .product("Teensy USB Serial")
        .device_class(USB_CLASS_CDC)
        .build();

    led.toggle();

    loop {
        // ✅ Poll USB to keep it alive
        if usb_dev.poll(&mut [&mut serial]) {
            let mut buf = [0u8; 64];

            // ✅ Read user input from `/dev/ttyACM1`
            if let Ok(count) = serial.read(&mut buf) {
                if count > 0 {
                    led.toggle(); // ✅ Blink LED on input
                    serial.write(&buf[..count]).ok(); // ✅ Echo input back
                }
            }
        }
    }
}
