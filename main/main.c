#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "common.h"
#include "meter_events.h"
#include "mbedtls/md.h"

char *mensagem = "iniciando mqtt";

static const char *TAG = "MQTT_EXAMPLE";
const char *ssid = "AndroidAP2CC1";
const char *passwd = "rzue0381";
int retry_num = 0;

#define BUF_SIZE (1024)
#define BAUD_RATE 9600

const uart_port_t uart_num = UART_NUM_2;
const char *UART_TAG2 = "UART";
QueueHandle_t uart_queue;
uint8_t *dt;
uint8_t ds[66];
int uart_retries = 0;
uint16_t crc_a = 0;
uint16_t crc_b = 0;
char *uart_msg;


uint16_t replaceByte(uint16_t value, uint8_t b, uint16_t pos)
{
    return (value & ~(0xFF << (pos * 8))) | ((b & 0xFF) << (pos * 8));
}

void uart_test()
{
    int uart_retries = 0;
    
    while(1)
    {
        if(uart_retries >= 20)
        {
            break;
        }

        uart_write_bytes(UART_NUM_2, (void *)ds, 66);
        int code = uart_read_bytes(UART_NUM_2, dt, (BUF_SIZE - 1), 100 / portTICK_PERIOD_MS);

        if(code > -1)
        {
            // for(int i = 0; i < 258; i++)
            // {
            //     if(i < 257)
            //     {
            //         printf("%x | ", dt[i]);
            //     }
            //     else
            //     {
            //         printf("%x\n", dt[i]);
            //     }
            // }
            crc_b = replaceByte(crc_b, dt[256], 1);
            crc_b = replaceByte(crc_b, dt[257], 0);
            crc_a = crc16arc_bit(0, dt, 256);

            if(crc_a == crc_b && dt[0] != 0)
            {
                ESP_LOGI(UART_TAG2, "Message validated!");
                break;
            }
            else
            {
                ESP_LOGE(UART_TAG2, "Message invalid");
            }
        }
        else
        {
            uart_retries++;
            ESP_LOGE(UART_TAG2,"Couldn't get UART message. num of retries: %i", uart_retries);
        }
        vTaskDelay(500);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    char *topico_alive = "eletra/meter-farm/238191212838/alive";
    char *topico_pub = "eletra/meter-farm/238191212838/log";
    

    esp_mqtt_event_handle_t event = event_data;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

        char *topico_sub = "eletra/meter-farm/238191212838";
       
        esp_mqtt_client_publish(handler_args, topico_alive, mensagem, 0, 0 ,0 );
        esp_mqtt_client_subscribe_single(handler_args, topico_sub, 0);

        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "Dados recebidos: ");
        printf("TOPICO=%.*s\r\n", event->topic_len, event->topic);
        printf("DADOS=%.*s\r\n", event->data_len, event->data);
        esp_mqtt_client_publish(handler_args, topico_alive, "mensagem recebida", 0, 0 ,0 );
        int x = strcmp(event->data, "log");
        printf("%i\n" , x);
        if(x >= 0)
        {
            uart_test();

            char *tmp = uart_msg;
            for (size_t i = 0; i < 258; i++)
            {
                tmp += sprintf(tmp, "%x", dt[i]);
            }
    
            esp_mqtt_client_publish(handler_args, topico_pub, uart_msg, 0, 0, 0);
        }
        else
        {
            esp_mqtt_client_publish(handler_args, topico_pub, "invalid command", 0, 0, 0);
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://broker.hivemq.com:1883",
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "Tentando conexao wifi....\n");
    }
    else if (event_id == WIFI_EVENT_STA_CONNECTED)
    {
        ESP_LOGI(TAG, "WiFi conenctado");
    }
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG, "WiFi desconectado\n");
        if (retry_num < 5)
        {
            esp_wifi_connect();
            retry_num++;
            ESP_LOGI(TAG, "Tentando reconexao wifi...\n");
        }
    }
    else if (event_id == IP_EVENT_STA_GOT_IP)
    {
        ESP_LOGI(TAG, "IP recebido");

        //Inicia procedimento de conexão ao MQTT
        mqtt_app_start();
    }
}

esp_err_t wifi_app_start()
{
    //Inicializa o armazenamento flash (requerido pelo driver de wifi)
    ESP_ERROR_CHECK(nvs_flash_init());

    //Inicializa o sistema  Wi-fi e a pilha TCP/IP
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation);

    //Registra as funçoes de callback para os eventos de wifi e ip
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);

    //Estrutura de incialização
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = "",
            .password =  "",
        }
    };

    //Faz uma cópia das variaveis globais para dentro da estrutura de inicialização
    strcpy((char*)wifi_configuration.sta.ssid, ssid);
    strcpy((char*)wifi_configuration.sta.password, passwd);

    //Atribui os parametros de inicialização
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);

    //Inicializa a conexão
    esp_wifi_start();
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_connect();

    return ESP_OK;
}

// typedef enum
// {
//     A,
//     B,
//     C
// }TEST;

// void init()
// {

// }

// void a(void *args)
// {
//     printf("Hello World!\n");
//     vTaskDelay(500);
// }

// void b(void *args)
// {
//     int *i = (int *)args;
//     printf("Managed to print this: %i\n", *i);
//     vTaskDelay(500);
// }

// void c(void *args)
// {
//     printf("Goodbye World!\n");
//     vTaskDelay(500);
// }

void setup()
{
    dt = (uint8_t *)calloc(BUF_SIZE, sizeof(uint8_t));
    for(int i = 0; i < 64; i++)
    {
        switch (i)
        {
        case 0:
            ds[i] = 0x14;
            break;

        case 1:
            ds[i] = 0x12;
            break;

        case 2:
            ds[i] = 0x34;
            break;
                    
        case 3:
            ds[i] = 0x56;
            break;
        
        default:
            ds[i] = 0x00;
            break;
        }
    }
    
    uint16_t a = 0;
    a = crc16arc_bit(a, ds, 64);
    ds[64] = (a >> (1 * 8)) & 0xFF;
    ds[65] = (a >> (0 * 8)) & 0xFF;

    for(int i = 0; i < 66; i++)
    {
        if(i < 65)
        {
            printf("%x | ", ds[i]);
        }
        else
        {
            printf("%x\n", ds[i]);
        }
    }

        uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
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

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, BUF_SIZE * 2, BUF_SIZE * 2, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));
    uart_set_line_inverse(UART_NUM_2, UART_SIGNAL_TXD_INV | UART_SIGNAL_RXD_INV);
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, 18, 5, 4, 2));
}

void app_main(void)
{
    // BaseType_t returned;
    // TaskHandle_t b = NULL;
    // returned = xTaskCreate(a, "a", 1000, NULL, 1, &b);
    // xTaskCreate(c, "c", 1000, &b, 0, NULL);
    // if(returned == pdPASS)
    // {
    //     vTaskDelete(b);
    // }

    //vTaskSuspend(b);

    // int x = 50;
    // CommandQueue *queue = create_command_queue(10);
    // register_func_to_command_queue(queue, &a, NULL, 0, A);
    // register_func_to_command_queue(queue, &b, &x, 4, B);
    // register_func_to_command_queue(queue, &c, NULL, 0, C);
    // enqueue_command_buffer(queue, A);
    // enqueue_command_buffer(queue, C);
    // enqueue_command_buffer(queue, B);
    // enqueue_command_buffer(queue, B);
    // enqueue_command_buffer(queue, C);
    // enqueue_command_buffer(queue, B);
    // enqueue_command_buffer(queue, A);

    // process_command_queue(queue);

    // int y = 10;
    // cq_fsm(queue, B, &y);

    //esp_log_level_set("*", ESP_LOG_NONE);
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);
    mbedtls_md_free(&ctx);
    uart_msg = (char *)calloc(BUF_SIZE, sizeof(char));
    setup();
    //setup_uart();
    ESP_ERROR_CHECK(wifi_app_start());
    //xTaskCreatePinnedToCore(wifi_app_start, "mqtt", 5000, NULL, 1, NULL, 0);
}