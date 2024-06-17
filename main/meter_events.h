#pragma once
#include "common.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "stdint.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "stdarg.h"

#define UART_BUF_SIZE (1024)
#define ABNT_TX (18)
#define ABNT_RX (5)

const static uart_port_t uart_port = UART_NUM_2;
const char *UART_TAG = "UART";

typedef enum
{
    SEND_COMMAND_TO_METER,
    SEND_DATA_TO_QUEUE,
    OPEN_SESSION,
    CLOSE_SESSION
}ABNT_COMMANDS;

typedef enum
{
    A
}DLMS_COMMANDS;

typedef struct
{
    uint8_t *ds; //To send
    uint8_t *dr; //To receive
    int ds_size;
    int dr_size;
} UARTData;

UARTData abnt_data;
CommandQueue abnt_command_queue;

//Helper functions
uint16_t crc16arc_bit(uint16_t crc, void const *mem, size_t len);

void setup_uart();
void uart_event_task();

void set_meter_id_task();
void check_crc();
void setup_abnt_command(uint8_t command, uint8_t *data);
void abnt_verify_data_received(const uint8_t *data, size_t len);
void abnt_uart_task(void *parameter);

