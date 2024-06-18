#include "meter_events.h"
#include "common.h"

const char *UART_TAG = "UART";
char METER_ID[4];
UARTData abnt_data;

uint16_t crc16arc_bit(uint16_t crc, void const *mem, size_t len) {
    unsigned char const *data = mem;
    if (data == NULL)
        return 0;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (unsigned k = 0; k < 8; k++) {
            crc = crc & 1 ? (crc >> 1) ^ 0xa001 : crc >> 1;
        }
    }

    uint32_t tmp = crc;
    crc = (crc << 8) | (tmp >> 8 & 0xff);
    return crc;
}

void setup_uart()
{
    abnt_data.dr = (uint8_t *)calloc(UART_BUF_SIZE, 8);
    abnt_data.ds = (uint8_t *)calloc(66, 8);

    for(int i = 0; i < 64; i++)
    {
        switch (i)
        {
        case 0:
            abnt_data.ds[i] = 0x00;
            break;

        case 1:
            abnt_data.ds[i] = 0x12;
            break;

        case 2:
            abnt_data.ds[i] = 0x34;
            break;
                    
        case 3:
            abnt_data.ds[i] = 0x56;
            break;
        }
    }

    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, UART_BUF_SIZE * 2, UART_BUF_SIZE * 2, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));
    uart_set_line_inverse(UART_NUM_2, UART_SIGNAL_TXD_INV | UART_SIGNAL_RXD_INV);
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, ABNT_TX, ABNT_TX, 4, 2));
}

void set_meter_id_task()
{
    if(abnt_data.dr == NULL || abnt_data.dr[2] == 0)
    {
        ESP_LOGW(UART_TAG, "Sem ID. Deletando task");
        vTaskDelete(NULL);
    }

    for(int i = 0; i < 4; i++)
    {
        METER_ID[i] = abnt_data.dr[i+1];
    }
}

void abnt_uart(void *parameter)
{
    while(true)
    {
        uart_write_bytes(UART_NUM_2, (void *)abnt_data.ds, 66);
        int code = uart_read_bytes(UART_NUM_2, abnt_data.dr, (UART_BUF_SIZE - 1), 100 / portTICK_PERIOD_MS);
        if(code > -1)
        {
             for(int i = 0; i < 258; i++)
            {
                if(i < 257)
                {
                    printf("%x | ", abnt_data.dr[i]);
                }
                else
                {
                    printf("%x\n", abnt_data.dr[i]);
                }
            }
        }
        if(abnt_data.dr[0] != 0)
        {
            break;
        }
        vTaskDelay(550);
    }
}

void setup_abnt_command(uint8_t command, uint8_t *data)
{
    abnt_data.ds[0] = command;
    if(data != NULL)
    {
        for(int i = 0; i < 59;i++)
        {
            abnt_data.ds[i + 4] = data[i];
        }
    }

    set_crc(abnt_data.ds, 64);
}

void set_crc(uint8_t *data, size_t size)
{
    uint16_t a = 0;
    a = crc16arc_bit(a, data, size);
    data[size] = (a >> (1 * 8)) & 0xFF;
    data[size + 1] = (a >> (0 * 8)) & 0xFF;
}

void uart_event_task()
{

}