#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "meter_events.h"

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

    int x = 50;
    CommandQueue *queue = create_command_queue(10);
    register_func_to_command_queue(queue, &a, NULL, 0, A);
    register_func_to_command_queue(queue, &b, &x, 4, B);
    register_func_to_command_queue(queue, &c, NULL, 0, C);
    enqueue_command_buffer(queue, A);
    enqueue_command_buffer(queue, C);
    enqueue_command_buffer(queue, B);
    enqueue_command_buffer(queue, B);
    enqueue_command_buffer(queue, C);
    enqueue_command_buffer(queue, B);
    enqueue_command_buffer(queue, A);

    process_command_queue(queue);

     int y = 10;
    cq_fsm(queue, B, &y);
}