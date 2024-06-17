#include "meter_events.h"
#include "common.h"

char METER_ID[4];

void setup_uart()
{
    abnt_data.dr = (uint8_t *)calloc(UART_BUF_SIZE, 8);
    abnt_data.ds = (uint8_t *)calloc(66, 8);

    for(int i = 0; i < 64; i++)
    {
        switch (i)
        {
        case 0:
            abnt_data.ds[i] = 0x14;
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
    vTaskDelete(NULL);
}

void abnt_uart_task(void *parameter)
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
            vTaskDelete(NULL);
        }
        vTaskDelay(550);
    }
}

void uart_event_task()
{

}