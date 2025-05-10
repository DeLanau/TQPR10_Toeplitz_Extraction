#!/usr/bin/env python3

import sys
import serial
import platform
import glob
import os
from statistics import mean
import argparse
import subprocess
from pathlib import Path

SERIAL_BAUD = 6000000
BIT_CHUNK_SIZE = 128
OUTPUT_LEN = BIT_CHUNK_SIZE // 2

BIT_CHUNK_BYTES = BIT_CHUNK_SIZE // 8
OUT_BYTES = OUTPUT_LEN // 8

current_iter = 0


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
            port = ['COM7']
        case _:
            print(f'❌ Error: Unsupported OS "{os}", exiting.')
            sys.exit(1)

    if len(port) == 0:
        print('❌ Error: No serial device found. Please connect a microcontroller and try again.')
        sys.exit(1)

    print('❓ Available ports:')
    for i, p in enumerate(port):
        print(f'{i}: {p}')

    if len(port) > 1:
        while True:
            try:
                sel = int(input('⌨️ Select port: '))
            except:
                print('❌ Not a number.')
            continue

        if sel >= 0 and sel < len(port):
            return port[sel]

            print(f'⚠️ Invalid port {sel}.')
    else:
        print(f'✅ Auto-selecting only port: {port[0]}')
        return port[0]


def process_chunks(data, ser, ms=False, raw=False):
    if ms:
        output_chunk_bytes = 4
    else:
        output_chunk_bytes = OUT_BYTES

    chunks_out = []
    for offset in range(0, len(data), BIT_CHUNK_BYTES):
        chunk = data[offset: offset + BIT_CHUNK_BYTES]
        if len(chunk) < BIT_CHUNK_BYTES:
            chunk = chunk.ljust(BIT_CHUNK_BYTES, b'\x00')

        written = ser.write(chunk)
        if written != len(chunk):
            raise RuntimeError(
                f'❌ Wrote {written}/{len(chunk)} bytes; expected {len(chunk)}'
            )

        resp = ser.read(output_chunk_bytes)
        if len(resp) != output_chunk_bytes:
            raise RuntimeError(
                f'❌ Incomplete response from MCU: got {len(resp)} bytes, expected {output_chunk_bytes}'
            )
        if ms:
            chunks_out.append(int.from_bytes(resp, 'big'))
        elif raw:
            chunks_out.append(resp)
        else:
            bits = ''.join(f'{b:08b}' for b in resp)
            chunks_out.append(bits)

    return chunks_out


def run_tests(port: str) -> None:
    tests = sorted(glob.glob('data/*.bin'))
    results = sorted(glob.glob(f'results/{OUTPUT_LEN}/*.bin'))
    failed = 0

    if len(tests) != len(results):
        print('❌ Mismatch in test and results. Generate new ones first!')
        sys.exit(1)

    with serial.Serial(port, SERIAL_BAUD, timeout=1) as ser:
        ser.reset_input_buffer()
        ser.reset_output_buffer()
        for test_path, result_path in zip(tests, results):
            data = open(test_path, 'rb').read()
            chunks_out = process_chunks(data, ser)

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
    os.makedirs(f'results/{OUTPUT_LEN}', exist_ok=True)

    ent_string = ''

    with serial.Serial(port, SERIAL_BAUD, timeout=1) as ser:
        ser.reset_input_buffer()
        ser.reset_output_buffer()

        for test_path in tests:
            print(f'🕑 Running {test_path}…')
            ent_string += f'{test_path}\n\n'
            data = open(test_path, 'rb').read()
            chunks_out = process_chunks(data, ser, raw=True)

            base = os.path.basename(test_path)
            result_path = os.path.join(f'results/{OUTPUT_LEN}', base)
            with open(result_path, 'wb') as f:
                f.write(b''.join(chunks_out))

            ent_string += subprocess.check_output(
                ['ent', result_path],
                stderr=subprocess.DEVNULL,
                text=True
            ) + '\n'

            print(f'✅ Regenerated {result_path}')

    with open(f'results/{OUTPUT_LEN}/ent.txt', 'w') as f:
        f.write(ent_string)


def log_times(port: str) -> None:
    while True:
        current_iter = input('⌨️ Select iteration (1 - 6): ')
        if current_iter in ['1', '2', '3', '4', '5', '6']:
            break
        print('❌ Not a valid iteration.')

    while True:
        u_repl = input('⚠️ WARN: Is the right firmware flashed? (y / n): ')
        if u_repl.lower() == 'y':
            break
        elif u_repl.lower() == 'n':
            print('❌ Flash the right firmware!')
        else:
            print('❓ Reply with y / n.')

    log_dir = f'logs/iter{current_iter}/{OUTPUT_LEN}'

    tests = sorted(glob.glob(f'data/*.bin'))
    os.makedirs(f'{log_dir}/', exist_ok=True)
    for old_log in glob.glob(f'{log_dir}/*.log'):
        os.remove(old_log)

    with serial.Serial(port, SERIAL_BAUD, timeout=1) as ser:
        ser.reset_input_buffer()
        ser.reset_output_buffer()

        for test_path in tests:
            basename = os.path.basename(test_path)
            name = os.path.splitext(basename)[0]
            log_path = os.path.join(f'{log_dir}', f'{name}.log')

            data = open(test_path, 'rb').read()
            times = process_chunks(data, ser, ms=True)

            t_min = min(times)
            t_max = max(times)
            avg = mean(times)

            with open(log_path, 'w') as f:
                f.write(f'min:  {t_min} µs\n')
                f.write(f'max:  {t_max} µs\n')
                f.write(f'mean: {avg:.1f} µs\n')

            print(f'✅ Logged timings for {basename} → {log_path}')
            print(f'🕑 min: {t_min} max: {t_max} avg: {avg}')


def main():
    parser = argparse.ArgumentParser(
        description='🧠 Run MCU test, generation, or timing suites!'
    )
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        '--test',
        action='store_true',
        help='🧪 Run integrity test suite'
    )
    group.add_argument(
        '--time',
        action='store_true',
        help='🕑 Run timing suite (firmware must have TIMED enabled)'
    )
    group.add_argument(
        '--gen',
        action='store_true',
        help='💣 Generate new result binaries (destructive)'
    )

    args = parser.parse_args()

    port = serial_select()

    if args.test:
        run_tests(port)
    elif args.time:
        log_times(port)
    elif args.gen:
        generate_results(port)


if __name__ == "__main__":
    main()
