# UART Text Transfer Project

## Overview

This project demonstrates a bidirectional UART text transfer system between a PC and an ESP32 microcontroller. The system sends text from the PC to the ESP32 via UART, stores it in memory, and then sends it back to the PC. During transmission, the actual data transfer speed is measured and displayed in bits per second.

![UART Text Transfer Diagram](uart_text_transfer_diagram.png)

## Key Features

- Bidirectional text transfer over UART at 2400 baud  
- Real-time measurement of data transmission speed  
- Text storage and retrieval on ESP32  
- Data integrity verification  
- Comprehensive error handling and timeout mechanisms  

## Components

### ESP32 Firmware (Written in C using ESP-IDF)
- Receives text from PC via UART  
- Stores text in RAM  
- Sends text back to PC via UART  

### PC Application (Written in Python)
- Sends text to ESP32  
- Receives and displays text from ESP32  
- Measures and displays real-time transmission speed  
- Verifies data integrity  

## Requirements

### Hardware
- ESP32 development board  
- USB-to-Serial connection to PC  

### Software
- ESP-IDF (v4.4 or newer)  
- Python 3.6 or newer  
- PySerial library  

## Installation

### ESP32 Firmware

Clone this repository:

```sh
git clone https://github.com/yourusername/uart-text-transfer.git
cd uart-text-transfer
