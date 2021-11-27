#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "dto.h"
#include <time.h>

typedef struct
{
    int client_sd;
    struct sockaddr_in serv_addr;
    socklen_t sock_len;
    int client_id;
} client_t;

static client_t client;
static dto_t dto;

static int client_init(client_t *ctx, char *ip)
{
    /* CREATE */
    if ((ctx->client_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    /* CONFIG */
    ctx->sock_len = sizeof(ctx->serv_addr);
    memset(&ctx->serv_addr, 0, ctx->sock_len);
    ctx->serv_addr.sin_family = AF_INET;
    ctx->serv_addr.sin_port = htons(PORT_ADDRESS);
    if (inet_pton(AF_INET, ip, &(ctx->serv_addr.sin_addr)) <= 0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    }

    /* CONNECT */
    if (connect(ctx->client_sd, (struct sockaddr *)&(ctx->serv_addr), ctx->sock_len) < 0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }

    return 0;
}

static void client_ping(client_t *ctx)
{
    /* PING TO SERVER */
    int random = rand();
    dto.id = ctx->client_id;
    dto.command = PING;
    dto.len = 1;
    dto.data[0] = random;
    common_write(ctx->client_sd, &dto);

    /* PING FROM SERVER */
    common_read(ctx->client_sd, &dto);
    if (dto.id == SERVER_ID && dto.command == PING && dto.len == 1)
    {
        if (dto.data[0] == (random + 1))
        {
            printf("Received valid ping from server\n");
        }
    }
}

static void initialize(void)
{
    srand(time(NULL));
}

int main(int argc, char *argv[])
{
    initialize();
    memset(&client, 0, sizeof(client_t));

    if (argc != 2)
    {
        printf("\n Usage: %s <ip of server> \n", argv[0]);
        return 1;
    }

    client_init(&client, argv[1]);

    /* CHECK ASSIGNMENT ON SERVER */
    common_read(client.client_sd, &dto);
    if (dto.command == NEW_CLIENT || dto.command == MAX_CLIENTS_REACHED)
    {
        char *message = (char *)&(dto.data);
        message[dto.len] = 0;
        if (fputs(message, stdout) == EOF)
        {
            printf("\n Error : Fputs error\n");
        }
    }
    if (dto.id == SERVER_ID)
    {
        printf("Any server slots available ;(");
        return 0;
    }
    client.client_id = dto.id;

    while (1)
    {
        /* PING SERVER ONE TIME PER SEC */
        client_ping(&client);

        usleep(100000);
    }

    return 0;
}