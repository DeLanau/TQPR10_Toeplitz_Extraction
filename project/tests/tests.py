#!/usr/bin/env python3

import sys
import serial
import platform
import glob
import os
import re

SERIAL_BAUD = 6000000
BIT_CHUNK_SIZE = 64
OUTPUT_LEN = 32

BIT_CHUNK_BYTES = 8
OUT_BYTES = 4

def serial_select() -> str:
    os = platform.system()
    port = None

    match os:
        case 'Linux':
            port = glob.glob('/dev/ttyACM*') + glob.glob('/dev/ttyUSB*')
        case 'Darwin':
            port = glob.glob('/dev/tty.usbmodem*') + \
                glob.glob('/dev/tty.usbserial*') + \
                glob.glob('/dev/cu.usbmodem*')
        case 'Windows':
            port = ['COM3']
        case _:
            print(f'❌ Error: Unsupported OS "{os}", exiting.')
            sys.exit(1)

    if len(port) == 0:
        print('❌ Error: No serial device found. Please connect a microcontroller and try again.')
        sys.exit(1)

    print('❓ Available ports:')
    for i, p in enumerate(port):
        print(f'{i}: {p}')

    while True:
        try:
            sel = int(input('⌨️ Select port: '))
        except:
            print('❌ Not a number.')
            continue

        if sel >= 0 and sel < len(port):
            return port[sel]

        print(f'⚠️ Invalid port {sel}.')

def run_tests(port: str) -> None:
    tests   = sorted(glob.glob('data/*.bin'))
    results = sorted(glob.glob('results/*.bin'))
    failed  = 0

    with serial.Serial(port, SERIAL_BAUD, timeout=1) as ser:
        ser.reset_input_buffer()
        ser.reset_output_buffer()
        for test_path, result_path in zip(tests, results):
            data = open(test_path, 'rb').read()
            chunks_out = []
            for offset in range(0, len(data), BIT_CHUNK_BYTES):
                chunk = data[offset : offset + BIT_CHUNK_BYTES]
                if len(chunk) < BIT_CHUNK_BYTES:
                    chunk = chunk.ljust(BIT_CHUNK_BYTES, b'\x00')

                written = ser.write(chunk)
                if written != len(chunk):
                    raise RuntimeError(
                        f'❌ Wrote {written}/{len(chunk)} bytes; expected {len(chunk)}'
                    )

                resp = ser.read(OUT_BYTES)
                if len(resp) != OUT_BYTES:
                    raise RuntimeError(
                        f'❌ Incomplete response from MCU: got {len(resp)} bytes, expected {OUT_BYTES}'
                    )

                bits = ''.join(f'{b:08b}' for b in resp)
                chunks_out.append(bits)

            full_out = ''.join(chunks_out)

            expected_bytes = open(result_path, 'rb').read()
            expected = ''.join(format(b, '08b') for b in expected_bytes)

            name = os.path.basename(result_path)
            if full_out != expected:
                failed += 1
                print(f'❌ Test {name} ➔ FAILED')
            else:
                print(f'✅ Test {name} ➔ OK')

    total = len(tests)
    passed = total - failed
    print(f'📈 {passed}/{total} tests passed ({100 * passed/total:.1f}%)')

def generate_results(port: str) -> None:
    tests = sorted(glob.glob('data/*.bin'))
    os.makedirs('results', exist_ok=True)

    with serial.Serial(port, SERIAL_BAUD, timeout=1) as ser:
        ser.reset_input_buffer()
        ser.reset_output_buffer()

        for test_path in tests:
            print(f'🕑 Running {test_path}…')
            data = open(test_path, 'rb').read()

            chunks = []
            for offset in range(0, len(data), BIT_CHUNK_BYTES):
              chunk = data[offset : offset + BIT_CHUNK_BYTES]
              if len(chunk) < BIT_CHUNK_BYTES:
                chunk = chunk.ljust(BIT_CHUNK_BYTES, b'\x00')

              written = ser.write(chunk)
              if written != len(chunk):
                  raise RuntimeError(
                      f'⚠️ Wrote {written}/{len(chunk)} bytes; expected {len(chunk)}'
                  )

              resp = ser.read(OUT_BYTES)
              if len(resp) != OUT_BYTES:
                  raise RuntimeError(
                    f'❌Incomplete response from MCU: got {len(resp)} bytes, expected {OUT_BYTES}'
                  )
              chunks.append(resp)

            base = os.path.basename(test_path)
            result_path = os.path.join('results', base)
            with open(result_path, 'wb') as f:
                f.write(b''.join(chunks))

            print(f'✅ Regenerated {result_path}')

def main():
  port = serial_select()
  try:
    if len(sys.argv) > 1 and sys.argv[1] == '-g':
      generate_results(port)
    else:
      run_tests(port)
  except serial.SerialException as e:
    print(f'Serial error: {e}')
    sys.exit(1)

if __name__ == "__main__":
  main()
