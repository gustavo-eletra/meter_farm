#include <stdio.h>
#include <string.h>
#include <memory.h>
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

    typedef struct
    {
        int x;
        int y;
    }st;

    const char *o = "oi";
    st x;
    x.x = 40;
    x.y = 30;
    void *y = &o;
    char *v = *((char *)y);
    int i = *(&v + 1) - v;
    void *z = calloc(1, sizeof(st));
    printf("size: %i\n", sizeof(st));
    printf("size: %i\n", i);
    memcpy(z, y, sizeof(*y));
    st *w = (st *)z;
    printf("struct test: %i\n", w->x);
    printf("struct test: %i\n", w->y);
    //printf("struct test: %i\n", w->y);
    // CommandQueue *queue = create_command_queue(4);
    // register_func_to_command_queue(queue, &a, NULL, A);
    // register_func_to_command_queue(queue, &b, &x, B);
    // register_func_to_command_queue(queue, &c, NULL, C);
    // enqueue_command_buffer(queue, A);
    // enqueue_command_buffer(queue, B);
    // enqueue_command_buffer(queue, C);

    // process_command_queue(queue);
}