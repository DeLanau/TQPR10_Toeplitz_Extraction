import sys

if len(sys.argv) != 3:
  print("Usage: python3 pipe.py input.bin /dev/tty.usbmodemXXXX")
  sys.exit(1)

input_file = sys.argv[1]
serial_port = sys.argv[2]

with open(input_file, "rb") as f, open(serial_port, "wb", buffering=0) as out:
  for byte in f.read():
    bits = format(byte, "08b")
    out.write(bits.encode())
