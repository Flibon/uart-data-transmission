#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "uart_task";

// UART configuration
#define UART_NUM UART_NUM_0
#define BUF_SIZE 1024
#define BAUD_RATE 2400

// UART pin0 with gpio 1 and 3
#define TXD_PIN (1)
#define RXD_PIN (3)

//Tested on ESP32-WROOM, encountered overflow at 512 bytes
//Update: Tried to use NVS, but it didn't work
//Now working with RAM-based storage
#define MAX_TEXT_SIZE 2048
static uint8_t ram_storage[MAX_TEXT_SIZE];
static size_t ram_storage_size = 0;

// Defining state machine structure
typedef enum {
    STATE_RECEIVING,
    STATE_SENDING,
    STATE_IDLE
} uart_state_t;

static uart_state_t current_state = STATE_RECEIVING;
static size_t total_bytes_received = 0;
static size_t total_bytes_sent = 0;
static bool end_of_transmission = false;
static const uint8_t EOT_MARKER[] = {'\n', '\n', '\n'};  
static const size_t EOT_MARKER_LEN = 3;
static uint8_t eot_buffer[3] = {0};

//Checking for end of transmission
bool check_eot(uint8_t byte) {
    // Shift buffer
    eot_buffer[0] = eot_buffer[1];
    eot_buffer[1] = eot_buffer[2];
    eot_buffer[2] = byte;
    
    // Check if buffer matches EOT marker
    return (memcmp(eot_buffer, EOT_MARKER, EOT_MARKER_LEN) == 0);
}

// UART communication initialisation
void init_uart(void) {
    ESP_LOGI(TAG, "Initializing UART with TX pin: %d, RX pin: %d", TXD_PIN, RXD_PIN);
    
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk = UART_SCLK_APB,
    };
    
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 0, NULL, 0));
    
    ESP_LOGI(TAG, "UART initialized successfully at %d baud", BAUD_RATE);
}

// Storing byte in RAM as memory (sending data)
void store_byte(uint8_t byte, size_t position) {
    if (position < MAX_TEXT_SIZE) {
        ram_storage[position] = byte;
        ram_storage_size = position + 1;
        
        if ((position % 100) == 0) {
            ESP_LOGI(TAG, "Stored %u bytes so far", (unsigned int)(position + 1));
        }
    } else {
        ESP_LOGE(TAG, "Storage overflow at position %u", (unsigned int)position);
    }
}

// Retrieving byte from RAM (received data)
uint8_t retrieve_byte(size_t position) {
    if (position < ram_storage_size) {
        return ram_storage[position];
    } else {
        ESP_LOGE(TAG, "Attempt to retrieve beyond storage at position %u", (unsigned int)position);
        return 0;
    }
}

// UART receiving task
void uart_receive_task(void *pvParameters) {
    uint8_t data[BUF_SIZE];
    
    ESP_LOGI(TAG, "Receiving task started, waiting for data...");
    
    while (current_state == STATE_RECEIVING) {
        int len = uart_read_bytes(UART_NUM, data, BUF_SIZE, 50 / portTICK_PERIOD_MS);
        
        if (len > 0) {
            ESP_LOGI(TAG, "Received %d bytes", len);
            
            // Process each byte
            for (int i = 0; i < len; i++) {
                // Store in RAM
                store_byte(data[i], total_bytes_received);
                total_bytes_received++;
                
                // Check for end of transmission
                if (check_eot(data[i])) {
                    ESP_LOGI(TAG, "End of transmission detected after %u bytes", 
                             (unsigned int)total_bytes_received);
                    
                    end_of_transmission = true;
                    
                    // Store actual data size (excluding EOT marker)
                    ram_storage_size = total_bytes_received - EOT_MARKER_LEN;
                    ESP_LOGI(TAG, "Final data size: %u bytes", (unsigned int)ram_storage_size);
                    //Reciving of data is complete
                    // changing state to sending
                    current_state = STATE_SENDING;
                    break;
                }
            }
            
            // If end of transmission is detected, then breaking out of the loop
            if (end_of_transmission) {
                break;
            }
        }
        
        // Small delay to prevent watchdog trigger
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    
    // Signal that we're ready to send data back
    ESP_LOGI(TAG, "Received %u bytes total", (unsigned int)total_bytes_received);
    ESP_LOGI(TAG, "Switching to sending mode");
    
    // Transition to sending state
    vTaskDelete(NULL);
}

// UART sending task
void uart_send_task(void *pvParameters) {
    uint8_t data[BUF_SIZE];
    
    ESP_LOGI(TAG, "Send task started, waiting for state transition...");
    
    // Waiting until we're in sending state
    while (current_state != STATE_SENDING) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    size_t actual_size = ram_storage_size;
    ESP_LOGI(TAG, "Starting to send exactly %u bytes back", (unsigned int)actual_size);
    
    // Small delay before starting transmission
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    // Sending data back in chunks
    while (total_bytes_sent < actual_size) {
        size_t chunk_size = (actual_size - total_bytes_sent > BUF_SIZE) ? 
                            BUF_SIZE : (actual_size - total_bytes_sent);
        
        ESP_LOGI(TAG, "Preparing to send chunk of %u bytes", (unsigned int)chunk_size);
        
        // Reading data from RAM
        for (size_t i = 0; i < chunk_size; i++) {
            data[i] = retrieve_byte(total_bytes_sent + i);
        }
        
        // Sending data via UART
        int bytes_sent = uart_write_bytes(UART_NUM, (const char*)data, chunk_size);
        if (bytes_sent > 0) {
            total_bytes_sent += bytes_sent;
            ESP_LOGI(TAG, "Sent %d bytes, total: %u/%u", 
                     bytes_sent, (unsigned int)total_bytes_sent, (unsigned int)actual_size);
        } else {
            ESP_LOGE(TAG, "Error sending data: %d", bytes_sent);
        }
        
        // Ensuring that we don't overwhelm the UART buffer
        uart_wait_tx_done(UART_NUM, 100 / portTICK_PERIOD_MS);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    
    ESP_LOGI(TAG, "Transmission complete. Sent %u bytes", (unsigned int)total_bytes_sent);
    current_state = STATE_IDLE;
    
    vTaskDelete(NULL);
}

void app_main(void) {
    // Initialize UART
    ESP_LOGI(TAG, "Initializing UART...");
    init_uart();
    
    ESP_LOGI(TAG, "Using RAM-based storage instead of NVS");
    ESP_LOGI(TAG, "Maximum storage capacity: %u bytes", MAX_TEXT_SIZE);
    
    ESP_LOGI(TAG, "Starting UART tasks...");
    
    // Create tasks of sending and receiving data
    xTaskCreate(uart_receive_task, "uart_rx", 4096, NULL, 5, NULL);
    xTaskCreate(uart_send_task, "uart_tx", 4096, NULL, 5, NULL);
}