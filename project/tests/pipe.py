#!/usr/bin/env python3

import sys
import serial
import time
import re
import datetime

if len(sys.argv) != 3:
    print('Usage: python3 pipe.py <input.bin> <serial_port>')
    sys.exit(1)

input_path = f'data/{sys.argv[1]}'
serial_path = sys.argv[2]
output_path = f'results/{sys.argv[1]}'


def filter_debug(buffer: str):
    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    log_path = f'logs/debug_{timestamp}.log'

    debug_buff = []
    data_buff = []
    for line in buffer.splitlines():
        line = line.strip()
        if line.startswith('['):
            debug_buff.append(line)
        else:
            data_buff.append(line)

    with open(log_path, 'w') as log_file:
        log_file.write('\n'.join(debug_buff))

    print(f'Saved debug output to {log_path}')
    return '\n'.join(data_buff)


try:
    with serial.Serial(serial_path, 115200, timeout=1) as ser:
        # time.sleep(0.00005)
        ser.reset_input_buffer()

        print(
            f'Sending bits from {input_path} to {serial_path}, please wait...')
        with open(input_path, 'rb') as f:
            for byte in f.read():
                bits = format(byte, '08b')
                _ = ser.write(bits.encode())
                time.sleep(0.0005)

        print('Data sent to MCU, reading output...')

        buffer = ''
        extracted_outputs: list[str] = []

        # read until timeout
        while True:
            chunk = ser.read(128).decode(errors='ignore')
            if not chunk:
                break
            buffer += chunk

        buffer = filter_debug(buffer)
        buffer = buffer.split('out:')
        for i, bits in enumerate(buffer):
            bits = bits.strip()
            bits = ''.join(b for b in bits if b in '01')

            if not bits:
                print(f'{i}:\tWARN: Block contained invalid characters: {bits}')
                continue

            print(f'{i}:\t{bits[:8]}... ({len(bits)} bits)')
            extracted_outputs.append(bits)

        all_bits = ''.join(extracted_outputs)

        print(f'Total blocks received: {len(extracted_outputs)}')
        print(f'Total bits collected: {len(all_bits)}')
        print(f'Writing binary output to {output_path}...')

        byte_chunks = [all_bits[i:i+8] for i in range(0, len(all_bits), 8)]

        with open(output_path, 'wb') as f:
            for i, chunk in enumerate(byte_chunks):
                if len(chunk) < 8:
                    chunk = chunk.ljust(8, '0')  # pad final byte
                _ = f.write(int(chunk, 2).to_bytes(1, byteorder='big'))

        print(f'Wrote {len(byte_chunks)} bytes to "{output_path}"')

except serial.SerialException as e:
    print(f'Serial error: {e}')
    sys.exit(1)

except FileNotFoundError as e:
    print(f'File error: {e}')
    sys.exit(1)
