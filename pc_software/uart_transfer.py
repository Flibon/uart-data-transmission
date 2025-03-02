import serial
import serial.tools.list_ports
import time
import sys
import os

# Configuration
PORT = 'COM4'  
BAUD_RATE = 2400
EOT_MARKER = '\n\n\n'  
TIMEOUT = 60  
MAX_RECEIVE_BYTES = 2048  # Maximum bytes to receive

# Test text
TEXT = """Finance Minister Arun Jaitley Tuesday hit out at former RBI governor Raghuram Rajan for predicting that the next banking crisis would be triggered by MSME lending, saying postmortem is easier than taking action when it was required. Rajan, who had as the chief economist at IMF warned of impending financial crisis of 2008, in a note to a parliamentary committee warned against ambitious credit targets and loan waivers, saying that they could be the sources of next banking crisis. Government should focus on sources of the next crisis, not just the last one.

In particular, government should refrain from setting ambitious credit targets or waiving loans. Credit targets are sometimes achieved by abandoning appropriate due diligence, creating the environment for future NPAs," Rajan said in the note." Both MUDRA loans as well as the Kisan Credit Card, while popular, have to be examined more closely for potential credit risk. Rajan, who was RBI governor for three years till September 2016, is currently."""

def measure_transmission_speed(start_time, bytes_count, current_time):
    elapsed_time = current_time - start_time
    if elapsed_time > 0:
        # Calculating bits per second (8 bits per byte + 2 bits for start/stop)
        bits = bytes_count * 10
        bits_per_second = bits / elapsed_time
        return bits_per_second
    return 0

def send_data(ser, data):
    print("\nSending data to MCU...")
    print(f"Text length: {len(data)} characters")
    
    # Add EOT marker (end of transmission)
    data_with_eot = data + EOT_MARKER
    
    # Prepare for transmission
    total_bytes = len(data_with_eot)
    bytes_sent = 0
    start_time = time.time()
    last_update_time = start_time
    
    # Sending data in chunks
    try:
        for i in range(0, total_bytes):
            ser.write(data_with_eot[i].encode())
            bytes_sent += 1
            
            # Updating speed every 10 bytes or 0.5 seconds whichever comes first
            current_time = time.time()
            if bytes_sent % 10 == 0 or (current_time - last_update_time) > 0.5:
                speed = measure_transmission_speed(start_time, bytes_sent, current_time)
                sys.stdout.write(f"\rSent: {bytes_sent}/{total_bytes} bytes | Speed: {speed:.2f} bits/sec")
                sys.stdout.flush()
                last_update_time = current_time
                
            # Small delay to prevent buffer overflows
            time.sleep(0.01)
    except Exception as e:
        print(f"\nError during sending: {e}")
    
    # Final update
    total_time = time.time() - start_time
    final_speed = measure_transmission_speed(start_time, bytes_sent, time.time())
    print(f"\nSent {bytes_sent} bytes in {total_time:.2f} seconds | Average speed: {final_speed:.2f} bits/sec")
    
    # Making sure all data is sent
    ser.flush()
    print("Data flushed to device")

def receive_data(ser):
    print("\nReceiving data from MCU...")
    
    received_data = bytearray()
    bytes_received = 0
    start_time = time.time()
    last_update_time = start_time
    last_receive_time = start_time
    
    # Wait a bit for MCU to process and start sending
    print("Waiting for MCU to start sending data...")
    time.sleep(3)  # Increased wait time
    
    # Receive data - wait for the expected amount of data
    expected_bytes = len(TEXT)
    print(f"Expecting approximately {expected_bytes} bytes...")
    
    try:
        # First, wait until we start receiving data
        while ser.in_waiting == 0:
            current_time = time.time()
            if (current_time - start_time) > 10:  # 10 seconds timeout for first byte
                print("Timeout waiting for first byte")
                break
            time.sleep(0.1)
        
        # Reset timers once we start receiving
        if ser.in_waiting > 0:
            start_time = time.time()
            last_update_time = start_time
            last_receive_time = start_time
        
        # Main receive loop
        while True:
            current_time = time.time()
            
            # Check for global timeout
            if (current_time - start_time) > TIMEOUT:
                print(f"\nGlobal timeout after {TIMEOUT} seconds")
                break
                
            # Check for inactivity timeout - but only if we've received at least 90% of expected data
            if bytes_received > expected_bytes * 0.9 and (current_time - last_receive_time) > 8:
                print(f"\nInactivity timeout after 8 seconds with no new data (received {bytes_received} bytes)")
                break
                
            # Check for maximum receive limit
            if bytes_received >= MAX_RECEIVE_BYTES:
                print(f"\nReached maximum receive limit of {MAX_RECEIVE_BYTES} bytes")
                break
                
            # Check if data is available
            if ser.in_waiting > 0:
                # Read available bytes (up to 64 at a time)
                chunk = ser.read(min(64, ser.in_waiting))
                if chunk:
                    received_data.extend(chunk)
                    bytes_received += len(chunk)
                    last_receive_time = current_time
                    
                    # Update speed display
                    if (current_time - last_update_time) > 0.5:
                        speed = measure_transmission_speed(start_time, bytes_received, current_time)
                        elapsed = current_time - start_time
                        progress = min(100, int(bytes_received / expected_bytes * 100))
                        sys.stdout.write(f"\rReceived: {bytes_received} bytes ({progress}%) | Speed: {speed:.2f} bits/sec | Time: {elapsed:.1f}s")
                        sys.stdout.flush()
                        last_update_time = current_time
            else:
                # Small delay when no data is available
                time.sleep(0.1)
                
                # If we've received at least the expected amount, increment a counter
                if bytes_received >= expected_bytes:
                    # Check if no data for 5 seconds after receiving expected amount
                    if (current_time - last_receive_time) > 5:
                        print(f"\nReceived expected amount ({bytes_received} bytes) and no more data for 5 seconds")
                        break
    except KeyboardInterrupt:
        print("\nReceive operation interrupted by user")
    except Exception as e:
        print(f"\nError during receiving: {e}")
        import traceback
        traceback.print_exc()
    
    # Final update
    if bytes_received > 0:
        total_time = time.time() - start_time
        final_speed = measure_transmission_speed(start_time, bytes_received, time.time())
        print(f"\nReceived {bytes_received} bytes in {total_time:.2f} seconds | Average speed: {final_speed:.2f} bits/sec")
        
        # Save raw data first
        with open("received_data.bin", "wb") as f:
            f.write(received_data)
        print(f"Saved raw binary data to {os.path.abspath('received_data.bin')}")
    else:
        print("No data was received from the MCU")

def main():
    ser = None
    try:
        # Open serial port
        print(f"Opening serial port {PORT} at {BAUD_RATE} baud...")
        ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
        time.sleep(2)  # Wait for connection to establish
        
        # Send data to MCU
        send_data(ser, TEXT)
        
        # Receive data back from MCU
        receive_data(ser)
        
        # Close serial port
        ser.close()
        print("Serial port closed.")
        
    except serial.SerialException as e:
        print(f"Error: {e}")
        print(f"Make sure the device is connected to {PORT} and not in use by another program.")
        print(f"Available ports:")
        for port in serial.tools.list_ports.comports():
            print(f"  {port.device}: {port.description}")
    except KeyboardInterrupt:
        print("\nOperation canceled by user.")
        if ser is not None:
            try:
                ser.close()
                print("Serial port closed.")
            except:
                pass
    except Exception as e:
        print(f"Unexpected error: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()