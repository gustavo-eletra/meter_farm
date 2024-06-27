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

typedef enum
{
    SEND_COMMAND_TO_METER,
    SET_ID,
    SEND_DATA_TO_QUEUE,
    OPEN_SESSION,
    CLOSE_SESSION
}ABNT_COMMANDS;

// typedef enum
// {

// }DLMS_COMMANDS;

typedef struct
{
    uint8_t *ds; //To send
    uint8_t *dr; //To receive
    int ds_size;
    int dr_size;
} UARTData;

extern char METER_ID[4];
extern UARTData abnt_data;
extern CommandQueue abnt_command_queue;
extern SemaphoreHandle_t uart_mutex;

//Helper functions
uint16_t crc16arc_bit(uint16_t crc, void const *mem, size_t len);

void setup_uart();
void uart_event_task();

void set_meter_id_task();
void check_crc(UARTData *data);
void set_crc(uint8_t *data, size_t size);
void setup_abnt_command(uint8_t command, uint8_t *data);
void abnt_verify_data_received(const uint8_t *data, size_t len);
void abnt_uart_task(void *parameter);
