#!/bin/bash

echo "Detecting platform..."

UNAME_OUT="$(uname -s)"
case "${UNAME_OUT}" in
  Linux*)   PORT_GLOB="/dev/ttyACM* /dev/ttyUSB*";;
  Darwin*)  PORT_GLOB="/dev/tty.usbmodem* /dev/tty.usbserial* /dev/cu.usbmodem*";;
  *)        echo "Unsupported OS: ${UNAME_OUT}"; exit 1;;
esac

echo "Searching for available serial ports..."
PORTS=$(ls $PORT_GLOB 2>/dev/null)

if [ -z "$PORTS" ]; then
  echo "No serial devices found. Please connect a microcontroller and try again."
  exit 1
fi

echo "Available ports:"
echo "$PORTS"

echo "Select a port to connect to:"
select PORT in $PORTS; do
  if [ -n "$PORT" ]; then
    echo "Connecting to $PORT..."
    minicom -D "$PORT" -b 115200
    break
  else
    echo "Invalid selection, please try again."
  fi
done
