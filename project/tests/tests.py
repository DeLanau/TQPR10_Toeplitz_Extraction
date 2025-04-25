#!/usr/bin/env python3

from statistics import mean
import sys
from typing import Any
import serial
import pprint

import platform
import glob
import os

SERIAL_PORTNO = 115200
BIT_CHUNK_SIZE = 64
OUTPUT_LEN = 32

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

  test_results: dict[str, dict[str, Any]] = {}

  with serial.Serial(port, SERIAL_PORTNO, timeout=1) as ser:
    ser.reset_input_buffer()
    ser.reset_output_buffer()

    for test_path, result_path in zip(tests, results):
      with open(test_path, 'rb') as f:
        data_bytes = f.read()
      all_bits = ''.join(format(b, '08b') for b in data_bytes)

      extracted_outputs: list[str] = []
      times: list[int] = []

      for offset in range(0, len(all_bits), BIT_CHUNK_SIZE):
        chunk = all_bits[offset : offset + BIT_CHUNK_SIZE]
        _ = ser.write(chunk.encode()) # send entire chunk
        ser.flush()

        while True:
          line = ser.readline().decode(errors='ignore').strip()
          if not line:
            continue
          if not line.startswith('out:'):
            print(f'⚠️ WARN: unexpected line from MCU: {line}')
            continue

          payload, _, took = line[4:].partition('took:')
          extracted_outputs.append(payload)

          if took.isdigit():
            times.append(int(took))
          else:
            print('⚠️ WARN: missing or invalid timing info')
          break

      full_output = ''.join(extracted_outputs)
      with open(result_path, 'rb') as f:
        expected = ''.join(format(b, '08b') for b in f.read())

      if full_output != expected:
        failed += 1
        print(f'❌ Test {result_path} failed.')
      else:
        print(f'✅ Test {result_path} succeeded.')

      test_results[test_path] = {
        'min' : min(times),
        'max' : max(times),
        'mean': mean(times),
      }

    total = len(tests)
    success_rate = 100 * (total - failed) / total
    print(f'🕑 {success_rate:.1f}% of tests passed ({total - failed}/{total})')
    pprint.pp(test_results)

# def run_tests(port: str) -> None:
#   tests = sorted(glob.glob('data/*.bin'))
#   results = sorted(glob.glob('results/*.bin'))
#   failed = 0
#
#   test_results: dict[str, dict[str, Any]] = {}
#
#   with serial.Serial(port, SERIAL_PORTNO, timeout=1) as ser:
#     ser.reset_input_buffer()
#     ser.reset_output_buffer()
#     for test, res in zip(tests, results):
#       print(f'🕑 Sending bits from {test}, please wait...')
#       with open(test, 'rb') as f:
#         for byte in f.read():
#           bits = format(byte, '08b')
#           _ = ser.write(bits.encode())
#           # time.sleep(0.0005)
#
#       print('🕑 Data sent to MCU, reading output...')
#
#       extracted_outputs: list[str] = []
#       times: list[int] = []
#
#       while True:
#         raw_line = ser.readline().decode(errors='ignore').strip()
#
#         if not raw_line:
#           break
#
#         if not raw_line.startswith('out:'):
#           print(f'⚠️ WARN: Block start invalid: {raw_line}')
#           continue
#
#         raw_parts = raw_line[4:].split('took:')
#         extracted_outputs.append(raw_parts[0])
#         if len(raw_parts) > 1 and raw_parts[1].isdigit():
#           times.append(int(raw_parts[1]))
#         else:
#           print(f'⚠️ WARN: Time missing!')
#
#         test_results[test] = { 'max': max(times), 'min': min(times), 'mean':  mean(times)}
#
#       with open(res, 'rb') as f:
#         result = ''.join(format(b, '08b') for b in f.read())
#         print('r ----')
#         print(len(result))
#         print('a ----')
#         print(len(''.join(extracted_outputs)))
#         if result != ''.join(extracted_outputs):
#           failed += 1
#           print(f'❌ Test {res} failed!')
#         else:
#           print(f'✅ Test {res} SUCCESS!')
#
#     pprint.pp(test_results)
#     print(f'✅ {round(len(tests) - failed / len(tests))}% successful!')

def generate_results(port: str) -> None:
  tests   = sorted(glob.glob('data/*.bin'))
  results = sorted(glob.glob('results/*.bin'))

  with serial.Serial(port, SERIAL_PORTNO, timeout=1) as ser:
    ser.reset_input_buffer()
    ser.reset_output_buffer()

    for test_path, result_path in zip(tests, results):
      with open(test_path, 'rb') as f:
        data = f.read()
      all_bits = ''.join(format(b, '08b') for b in data)

      chunks_out: list[str] = []

      for i in range(0, len(all_bits), BIT_CHUNK_SIZE):
        chunk = all_bits[i:i + BIT_CHUNK_SIZE]
        ser.write(chunk.encode())
        ser.flush()

        # read exactly one response, break on timeout or error
        line = ser.readline()  # timeout=1s
        if not line:
          raise RuntimeError("Timeout waiting for MCU response")
        line = line.decode(errors='ignore').strip()
        if not line.startswith('out:'):
          raise RuntimeError(f"Bad MCU response: {line}")

        payload, _, _ = line[4:].partition('took:')
        chunks_out.append(payload)
        print(payload)
        break

      full_out = ''.join(chunks_out)
      byte_vals = [
        int(full_out[j:j+8], 2)
        for j in range(0, len(full_out), 8)
      ]
      with open(result_path, 'wb') as f:
        _ = f.write(bytes(byte_vals))
      print(f'Regenerated {os.path.basename(result_path)}')

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
