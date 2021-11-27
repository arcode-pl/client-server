#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include "dto.h"

void common_print(dto_t *dto)
{
    int i;

    printf("ID: %d\r\n", dto->id);
    printf("COMMAND: %d\r\n", dto->command);
    printf("LEN: %d\r\n", dto->len);
    printf("DATA:\r\n");

    for (i = 0; i < dto->len; i++)
    {
        printf("0x%02x ", ((char *)dto->data)[i]);
        if ((i % 16) == 15)
        {
            printf("\r\n");
        }
    }
    if ((i % 16) != 15)
    {
        printf("\r\n");
    }
}

void common_read(int sd, dto_t *dto)
{
    int bytes = read(sd, (char *)dto, sizeof(dto_t));
    //printf("readed %d bytes\r\n", bytes);
    //common_print(dto);

    if (bytes != sizeof(dto_t))
    {
        printf("\n Read error \n");
    }
}

void common_write(int sd, dto_t *dto)
{
    int bytes = write(sd, (char *)dto, sizeof(dto_t));
    //printf("writed %d bytes\r\n", bytes);
    //common_print(dto);

    if (bytes != sizeof(dto_t))
    {
        printf("\n Write error \n");
    }
}