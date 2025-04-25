#!/usr/bin/env python3

from math import nan
from statistics import mean
import sys
from typing import Any
import serial
import time
import pprint

import platform
import glob

SERIAL_PORTNO = 115200


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
            print(f'Error: Unsupported OS "{os}", exiting.')
            sys.exit(1)

    if len(port) == 0:
        print('❌ Error: No serial device found. Please connect a microcontroller and try again.')
        sys.exit(1)

    print('Available ports:')
    for i, p in enumerate(port):
        print(f'{i}: {p}')

    while True:
        try:
            sel = int(input('Select port: '))
        except:
            print('Not a number.')
            continue

        if sel >= 0 and sel < len(port):
            return port[sel]

        print(f'⚠️ Invalid port {sel}.')

def run_tests(port: str) -> None:
  tests = sorted(glob.glob('data/*.bin'))
  results = sorted(glob.glob('results/*.bin'))
  failed = 0

  test_results: dict[str, dict[str, Any]] = {}

  with serial.Serial(port, SERIAL_PORTNO, timeout=1) as ser:
    ser.reset_input_buffer()
    ser.reset_output_buffer()
    for test, res in zip(tests, results):
      print(f'🕑 Sending bits from {test}, please wait...')
      with open(test, 'rb') as f:
        for byte in f.read():
          bits = format(byte, '08b')
          _ = ser.write(bits.encode())
          time.sleep(0.0005)

      print('🕑 Data sent to MCU, reading output...')

      extracted_outputs: list[str] = []
      times: list[int] = []

      while True:
        raw_line = ser.readline().decode(errors='ignore').strip()
        
        if not raw_line:
          break

        if not raw_line.startswith('out:'):
          print(f'⚠️ WARN: Block start invalid: {raw_line}')
          continue

        raw_parts = raw_line[4:].split('took:')
        extracted_outputs.append(raw_parts[0])
        if len(raw_parts) > 1 and raw_parts[1].isdigit():
          times.append(int(raw_parts[1]))
        else:
          print(f'⚠️ WARN: Time missing!')

        test_results[test] = { 'max': max(times), 'min': min(times), 'mean':  mean(times)}

      with open(res, 'rb') as f:
        result = ''.join(format(b, '08b') for b in f.read())
        print('r ----')
        print(len(result))
        print('a ----')
        print(len(''.join(extracted_outputs)))
        if result != ''.join(extracted_outputs):
          failed += 1
          print(f'❌ Test {res} failed!')
        else:
          print(f'✅ Test {res} SUCCESS!')

    pprint.pp(test_results)
    print(f'✅ {round(len(tests) - failed / len(tests))}% successful!')

def main():
    port = serial_select()
    try:
        run_tests(port)
    except serial.SerialException as e:
        print(f'Serial error: {e}')
        sys.exit(1)


if __name__ == "__main__":
    main()
