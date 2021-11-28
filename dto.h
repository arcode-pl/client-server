#ifndef __DTO_H__
#define __DTO_H__

#define PORT_ADDRESS 8888
#define SERVER_ADDRESS "127.0.0.1"

#define UNSET_ID -2
#define SERVER_ID -1
#define MAX_DATA_LEN 256
#define MAX_DATA_SIZE (MAX_DATA_LEN * 4)
#define DTO_SIZE ((MAX_DATA_LEN + 3) * 4)

typedef enum command
{
    PING,
    NEW_CLIENT,
    MAX_CLIENTS_REACHED,
} command_t;

typedef struct dto
{
    int id; //client or server id
    command_t command;
    int len; //len of buffer
    int data[MAX_DATA_LEN];
} dto_t;

void common_print(dto_t *dto);
int common_write(int *sd, dto_t *dto);
int common_read(int *sd, dto_t *dto);

#endif