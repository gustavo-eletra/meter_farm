#include "common.h"
#include "memory.h"
#include <stdio.h>

CommandQueue *create_command_queue(size_t size)
{
    CommandQueue *cq = (CommandQueue *)malloc(sizeof(CommandQueue));
    cq->command_arr = (int *)malloc(sizeof(int) * size);
    cq->data_arr = (CommandData *)malloc(sizeof(CommandData) * size);
    cq->queue = (int *)malloc(sizeof(int) * size);
    cq->current_size = 0;
    cq->front = -1;
    cq->counter = -1;
    cq->rear = 0;
    cq->total_size = size;
    cq->processed = false;

    for(int i = 0; i < size;i++)
    {
        cq->queue[i] = -1;
        cq->command_arr[i] = -1;
    }
    return cq;
}

bool is_command_queue_empty(CommandQueue *cq)
{
    if(cq->current_size == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool is_command_queue_full(CommandQueue *cq)
{
    if(cq->current_size == cq->total_size)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool enqueue_command_buffer(CommandQueue *cq, int command)
{
    if(is_command_queue_full(cq) || cq->processed == true)
    {
        printf("PLEASE RESET QUEUE\n");
        return false;
    }
    if(cq->front < 0)
    {
        cq->front = 0;
    }
    cq->queue[cq->current_size] = command;
    cq->current_size++;
    cq->rear = cq->current_size;
    return true;
}

bool dequeue_command_buffer(CommandQueue *cq)
{
    if(cq->front < 0 || cq->front >= cq->current_size
    || cq->processed == true || is_command_queue_empty(cq))
    {
        printf("PLEASE RESET/POPULATE QUEUE\n");
        return false;
    }
    // cq->function_arr[cq->front].
    // func[0](cq->function_arr[0].data);

    int command = cq->queue[cq->front];
    int a = 0;

    for(int i = 0; i < cq->current_size; i++)
    {
        if(command == cq->command_arr[i])
        {
            break;
        }
        a++;
    }
    cq->func_arr[a](cq->data_arr[a].data);
 
    cq->front++;
    if(cq->front >= cq->rear)
    {
        cq->processed = true;
    }
    return true;
}

void process_command_queue(CommandQueue *cq)
{
    while(dequeue_command_buffer(cq) == true)
    {
        printf("Processing commands\n");
    }
}

bool reset_command_queue(CommandQueue *cq)
{
    if(cq->front < 0)
    {
        printf("QUEUE EMPTY\n");
        return false;
    }
    cq->front = 0;
    cq->processed = false;
    return true;
}

void clear_command_queue(CommandQueue *cq)
{
    for(int i = 0; i < cq->current_size; i++)
    {
        cq->command_arr[i] = -1;
        cq->func_arr[i] = NULL;
        free(cq->data_arr[i].data);
        cq->queue[i] = -1;
    }
    cq->counter = 0;
    cq->current_size = 0;
    cq->rear = 0;
    cq->front = -1;
    cq->processed = false;
}

void register_func_to_command_queue(CommandQueue *cq, void (*func)(void *parameters), void *func_args, int command)
{

    if(cq->counter >= cq->total_size)
    {
        return;
    }

    cq->counter++;
    cq->func_arr[cq->counter] = *(*func);
    if(func_args != NULL)
    {
        printf("ptr : %i\n", *(int *)func_args);
        cq->data_arr[cq->counter].data = malloc(sizeof(*func_args));
        memcpy(cq->data_arr[cq->counter].data, func_args, sizeof(*func_args));
        printf("ptr : %i\n", *(int *)cq->data_arr[cq->counter].data);
    }
    else
    {
         cq->data_arr[cq->counter].data = func_args;
    }
    cq->command_arr[cq->counter] = command; 
}

void cq_state(CommandQueue *cq)
{
    printf("ComandQueue front: %i\n", cq->front);
    printf("ComandQueue rear: %i\n", cq->rear);
    printf("ComandQueue counter: %i\n", cq->counter);
    printf("ComandQueue current size: %i\n", cq->current_size);
    printf("ComandQueue is processed: %i\n", cq->processed);
}