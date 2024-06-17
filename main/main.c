#include <stdio.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "common.h"

typedef enum
{
    A,
    B,
    C
}TEST;

void init()
{

}

void a(void *args)
{
    printf("Hello World!\n");
    vTaskDelay(500);
}

void b(void *args)
{
    int *i = (int *)args;
    printf("Managed to print this: %i\n", *i);
    vTaskDelay(500);
}

void c(void *args)
{
    printf("Goodbye World!\n");
    vTaskDelay(500);
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

    int x = 10;
    CommandQueue *queue = create_command_queue(4);
    register_func_to_command_queue(queue, &a, NULL, A);
    register_func_to_command_queue(queue, &b, &x, B);
    register_func_to_command_queue(queue, &c, NULL, C);
    enqueue_command_buffer(queue, A);
    enqueue_command_buffer(queue, B);
    enqueue_command_buffer(queue, C);

    process_command_queue(queue);
}