#pragma once
#include <esp_log.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_system.h>
#include <stdlib.h>
#include <stdbool.h>

typedef void (*command_func[256])(void *data);

//TODO:make function to change the data of the command

typedef struct
{
    void *data;
}CommandData;

//
typedef struct
{
    bool processed;
    int *command_arr;
    int *queue;
    int counter;
    int front;
    int rear;
    int current_size;
    int total_size;
    CommandData *data_arr;
    command_func func_arr;
}CommandQueue;

CommandQueue *create_command_queue(size_t size);
CommandQueue *resize_command_queue(CommandQueue cq, size_t size);
bool is_command_queue_empty(CommandQueue *cq);
bool is_command_queue_full(CommandQueue *cq);
void register_func_to_command_queue(CommandQueue *cq, void (*func)(void *parameters), void *func_args, size_t args_size,int command);
bool dequeue_command_buffer(CommandQueue *cq);
bool enqueue_command_buffer(CommandQueue *cq, int command);
void process_command_queue(CommandQueue *cq);
void clear_command_queue(CommandQueue *cq);
bool reset_command_queue(CommandQueue *cq);
void free_command_queue(CommandQueue *cq);
void cq_state(CommandQueue *cq);
void cq_fsm(CommandQueue *cq, int command, void *data);