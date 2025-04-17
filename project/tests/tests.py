#!/usr/bin/env python3

import sys
import serial
import time

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
      port = glob.glob('/dev/tty.usbmodem*') + glob.glob('/dev/tty.usbserial*') + glob.glob('/dev/cu.usbmodem*')
    case 'Windows':
      print('Error: Ew. Exiting due to cringe.')
      sys.exit(1)
    case _:
      print(f'Error: Unsupported OS "{os}", exiting.')
      sys.exit(1)

  if len(port) == 0:
    print('Error: No serial device found. Please connect a microcontroller and try again.')
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

    print(f'Invalid port {sel}.')

def run_tests(port: str) -> None:
  no_tests = len(glob.glob('data/*.txt'))
  failed = 0

  with serial.Serial(port, SERIAL_PORTNO, timeout=1) as ser:
    for i in range(no_tests):
      data = f'data/{i}.txt'
      result = f'results/{i}.txt'

      with open(data, 'r') as f:
        data_bits = f.read()

      with open(result, 'r') as f:
        expected_outputs = [line.strip() for line in f if line.strip()]

      print(f'Running test {i}:')
      received_outputs: list[str] = []
      ser.reset_input_buffer()

      for bit in data_bits:
        _ = ser.write(bit.encode())
        time.sleep(0.0005)

        if ser.in_waiting:
          line = ser.readline().decode(errors='ignore').strip()
          if line.startswith('out:'):
            received_outputs.append(line[4:])

      if len(received_outputs) != len(expected_outputs):
        print(f'FAIL: Test {i} - Mismatched output count ({len(received_outputs)} vs {len(expected_outputs)})')
        failed += 1
        continue

      mismatch = False
      for i, (expected, actual) in enumerate(zip(expected_outputs, received_outputs)):
        if expected != actual:
          print(f'FAILED: Test {i} - Mismatch at index {i}')
          print(f'\txpected: {expected}')
          print(f'\tReceived: {actual}')
          mismatch = True
          failed += 1
          break

      if not mismatch:
        print(f'PASS: Test {i}')

  print('TESTS COMPLETE')
  print(f'Passed tests: {round((no_tests - failed / no_tests) * 100, 2)}%')

def main():
  port = serial_select()
  try:
    run_tests(port)
  except serial.SerialException as e:
    print(f'Serial error: {e}')
    sys.exit(1)

if __name__ == "__main__":
  main()
