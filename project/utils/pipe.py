# import sys
#
# if len(sys.argv) != 3:
#   print("Usage: python3 pipe.py input.bin /dev/tty.usbmodemXXXX")
#   sys.exit(1)
#
# input_file = sys.argv[1]
# serial_port = sys.argv[2]
#
# with open(input_file, "rb") as f, open(serial_port, "wb", buffering=0) as out:
#   for byte in f.read():
#     bits = format(byte, "08b")
#     out.write(bits.encode())

import sys
import serial
import time

if len(sys.argv) != 4:
  print("Usage: python3 pipe.py input.bin /dev/tty.usbmodemXXXX output.bin")
  sys.exit(1)

input_path = sys.argv[1]
serial_path = sys.argv[2]
output_path = sys.argv[3]

with serial.Serial(serial_path, 115200, timeout=1) as ser:
  time.sleep(2)
  ser.reset_input_buffer()

  with open(input_path, "rb") as f:
    for byte in f.read():
      bits = format(byte, "08b")
      ser.write(bits.encode())
      time.sleep(0.0005)

  extracted_bits = ""
  while True:
    if ser.in_waiting:
      line = ser.readline().decode(errors='ignore').strip()
      if line.startswith("out:"):
        extracted_bits += line[4:]
    else:
      break

byte_chunks = [extracted_bits[i:i+8] for i in range(0, len(extracted_bits), 8)]
with open(output_path, "wb") as f:
  for chunk in byte_chunks:
    if len(chunk) < 8:
      chunk = chunk.ljust(8, '0')
    f.write(int(chunk, 2).to_bytes(1, byteorder='big'))

print(f"Wrote {len(byte_chunks)} bytes of extracted output to {output_path}")
