#!/usr/bin/env python3

import sys
import serial
import time

def main():
  if len(sys.argv) != 3:
    print("Usage: python3 send_to_serial.py <filename> <serial_port>")
    sys.exit(1)

  filename = sys.argv[1]
  serial_port = sys.argv[2]

  try:
    with open(filename, 'r') as file:
      lines = file.readlines()
  except FileNotFoundError:
    print(f"Error: File '{filename}' not found.")
    sys.exit(1)

  try:
    with serial.Serial(serial_port, 115200, timeout=1) as ser:
      for line in lines:
        line = line.strip()
        if line:
          ser.write((line + '\n').encode())
          time.sleep(0.01)
          print(f"Successfully sent contents of {filename} to {serial_port}.")
  except serial.SerialException as e:
    print(f"Serial error: {e}")
    sys.exit(1)

if __name__ == "__main__":
  main()
