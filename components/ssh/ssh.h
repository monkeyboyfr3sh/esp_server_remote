#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char* username;
    char* password;
    char* host;
    char* command;
} ssh_cmd_input_t;

void ssh_command_task(void *pvParameters);
void ssh_task(void *pvParameters);

#ifdef __cplusplus
}
#endif