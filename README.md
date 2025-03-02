<<<<<<< HEAD
# UART Text Transfer Project

## Overview

This project demonstrates a bidirectional UART text transfer system between a PC and an ESP32 microcontroller. The system sends text from the PC to the ESP32 via UART, stores it in memory, and then sends it back to the PC. During transmission, the actual data transfer speed is measured and displayed in bits per second.

## Key Features

- **Bidirectional text transfer over UART at 2400 baud**
- **Real-time measurement of data transmission speed**
- **Text storage and retrieval on ESP32**
- **Data integrity verification**
- **Comprehensive error handling and timeout mechanisms**

## Components

### ESP32 Firmware (C using ESP-IDF)
- Receives text from PC via UART
- Stores text in RAM
- Sends text back to PC via UART

### PC Application (Python)
- Sends text to ESP32
- Receives and displays text from ESP32
- Measures and displays real-time transmission speed
- Verifies data integrity

## Requirements

### Hardware
- ESP32 development board
- USB-to-Serial connection to PC

### Software
- **ESP-IDF (v4.4 or newer)**
- **Python 3.6 or newer**
- **PySerial library**

## Installation

### ESP32 Firmware

Clone this repository:

```sh
git clone https://github.com/yourusername/uart-text-transfer.git
cd uart-text-transfer
```

Build and flash the ESP32 firmware:

```sh
cd esp32_firmware
idf.py build
idf.py -p [PORT] flash monitor
```

Replace `[PORT]` with your ESP32's serial port (e.g., `COM4`, `/dev/ttyUSB0`).

### PC Application

Install required Python packages:

```sh
pip install pyserial
```

Configure the serial port in `uart_transfer.py` if needed:

```python
PORT = 'COM4'  # Change to match your system
```

## Usage

1. Connect the ESP32 to your PC via USB.
2. Flash the firmware to the ESP32.
3. Run the Python script:

```sh
python uart_transfer.py
```

The script will:
- Send the predefined text to the ESP32.
- Display the transmission speed in bits/second.
- Receive the text back from the ESP32.
- Display the received text and verify its integrity.

## How It Works

### Data Flow

1. **PC** sends text data to ESP32 via UART at 2400 baud.
2. **ESP32** receives and stores each byte in RAM.
3. **Once all data is received** (detected by EOT marker), ESP32 sends it back.
4. **PC** receives the returned data and compares it with the original.

### Speed Measurement

The actual transmission speed is calculated by:

```math
(Bytes Transmitted × 10) / Time Elapsed (seconds)
```

(*10 accounts for 8 data bits + start bit + stop bit*)

## Project Structure

```
uart-text-transfer/
├── README.md
├── esp32_firmware/
│   ├── main/
│   │   ├── main.c             # ESP32 firmware
│   │   └── CMakeLists.txt
│   ├── CMakeLists.txt
│   └── sdkconfig
└── pc_software/
    └── uart_transfer.py       # PC application
```

## Technical Details

### UART Configuration
- **Baud rate:** 2400
- **Data bits:** 8
- **Parity:** None
- **Stop bits:** 1
- **Flow control:** None

### ESP32 Memory Usage
- Text data is stored in RAM (up to 2048 bytes).
- End of transmission is detected using a **3-byte marker**: `"\n\n\n"`

### Performance
- **Expected transmission speed:** ~240 bytes/second (2400 bits/second)
- **Actual measured speed** may vary due to processing overhead.

## Troubleshooting

### Common Issues

#### Serial port not found
- Check that the ESP32 is properly connected.
- Verify the correct port is specified in the Python script.
- Ensure no other program is using the serial port.

#### Data transmission errors
- Try reducing the baud rate if experiencing errors.
- Increase the delay between character transmissions.
- Check for buffer overflow conditions.

#### ESP32 not responding
- Reset the ESP32 and try again.
- Check the monitor output for error messages.
- Ensure proper wiring of the serial connection.

---

Made with ❤️ by Manav Singh
=======
| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | -------- | -------- | -------- |

# UART Echo Example

(See the README.md file in the upper level 'examples' directory for more information about examples.)

This example demonstrates how to utilize UART interfaces by echoing back to the sender any data received on
configured UART.

## How to use example

### Hardware Required

The example can be run on any development board, that is based on the Espressif SoC. The board shall be connected to a computer with a single USB cable for flashing and monitoring. The external interface should have 3.3V outputs. You may
use e.g. 3.3V compatible USB-to-Serial dongle.

### Setup the Hardware

Connect the external serial interface to the board as follows.

```
  -----------------------------------------------------------------------------------------
  | Target chip Interface | Kconfig Option     | Default ESP Pin      | External UART Pin |
  | ----------------------|--------------------|----------------------|--------------------
  | Transmit Data (TxD)   | EXAMPLE_UART_TXD   | GPIO4                | RxD               |
  | Receive Data (RxD)    | EXAMPLE_UART_RXD   | GPIO5                | TxD               |
  | Ground                | n/a                | GND                  | GND               |
  -----------------------------------------------------------------------------------------
```
Note: Some GPIOs can not be used with certain chips because they are reserved for internal use. Please refer to UART documentation for selected target.

Optionally, you can set-up and use a serial interface that has RTS and CTS signals in order to verify that the
hardware control flow works. Connect the extra signals according to the following table, configure both extra pins in
the example code `uart_echo_example_main.c` by replacing existing `UART_PIN_NO_CHANGE` macros with the appropriate pin
numbers and configure UART1 driver to use the hardware flow control by setting `.flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS`
and adding `.rx_flow_ctrl_thresh = 122` to the `uart_config` structure.

```
  ---------------------------------------------------------------
  | Target chip Interface | Macro           | External UART Pin |
  | ----------------------|-----------------|--------------------
  | Transmit Data (TxD)   | ECHO_TEST_RTS   | CTS               |
  | Receive Data (RxD)    | ECHO_TEST_CTS   | RTS               |
  | Ground                | n/a             | GND               |
  ---------------------------------------------------------------
```

### Configure the project

Use the command below to configure project using Kconfig menu as showed in the table above.
The default Kconfig values can be changed such as: EXAMPLE_TASK_STACK_SIZE, EXAMPLE_UART_BAUD_RATE, EXAMPLE_UART_PORT_NUM (Refer to Kconfig file).
```
idf.py menuconfig
```

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## Example Output

Type some characters in the terminal connected to the external serial interface. As result you should see echo in the same terminal which you used for typing the characters. You can verify if the echo indeed comes from ESP board by
disconnecting either `TxD` or `RxD` pin: no characters will appear when typing.

## Troubleshooting

You are not supposed to see the echo in the terminal which is used for flashing and monitoring, but in the other UART configured through Kconfig can be used.
>>>>>>> be9a19f (Initial commit - Added UART Text Transfer Project)
