#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define COMMAND_MAX_SIZE	1024
#define NUM_COMMANDS		20
#define NUM_BUTTONS         5

void gpio_task_example(void* arg);

#ifdef __cplusplus
}
#endif