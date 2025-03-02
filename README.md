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

Made with ❤️ by [Your Name]
