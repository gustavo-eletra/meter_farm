

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

static const char *TAG = "MQTT_EXAMPLE";
const char *ssid = "AndroidAP2CC1";
const char *passwd = "rzue0381";
int retry_num = 0;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

        char *topico = "testtopic/238191212838";
        char *mensagem = "Olá mundo";

        esp_mqtt_client_publish(handler_args, topico, mensagem, 0, 0 ,0 );
        esp_mqtt_client_subscribe_single(handler_args, topico, 0);

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

    ESP_ERROR_CHECK(wifi_app_start());
}