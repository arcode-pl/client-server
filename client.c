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
#include <time.h>
#include <stdbool.h>
#include "dto.h"

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
    int conn_sd;
    if ((conn_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return -1;
    }

    /* CONFIG */
    ctx->sock_len = sizeof(ctx->serv_addr);
    memset(&ctx->serv_addr, 0, ctx->sock_len);
    ctx->serv_addr.sin_family = AF_INET;
    ctx->serv_addr.sin_port = htons(PORT_ADDRESS);
    if (inet_pton(AF_INET, ip, &(ctx->serv_addr.sin_addr)) <= 0)
    {
        printf("\n inet_pton error occured\n");
        return -1;
    }

    /* CONNECT */
    if (connect(conn_sd, (struct sockaddr *)&(ctx->serv_addr), ctx->sock_len) < 0)
    {
        printf("\n Error : Connect Failed \n");
        return -1;
    }

    ctx->client_sd = conn_sd;

    return 0;
}

static int client_assign(client_t *ctx)
{
    /* CHECK ASSIGNMENT FROM SERVER */
    if (common_read(&(ctx->client_sd), &dto) != 0)
    {
        return -1;
    }

    /* PRINT MESSAGE FROM SERVER */
    if (dto.command == NEW_CLIENT || dto.command == MAX_CLIENTS_REACHED)
    {
        char *message = (char *)&(dto.data);
        message[dto.len] = 0;
        if (fputs(message, stdout) == EOF)
        {
            printf("Error : Fputs error\r\n");
        }
    }

    // CLOSE NEWLY CREATED CONNECTION IF CAN'T ASSIGN
    if (dto.id == SERVER_ID)
    {
        printf("Any server slots available ;(\r\n");
        close(ctx->client_sd);
        ctx->client_sd = 0;
        return -1;
    }

    // ASSIGN CLIENT ID
    ctx->client_id = dto.id;

    return 0;
}

static void client_ping(client_t *ctx)
{
    /* PREPARE DATA */
    int random = rand();
    dto.id = ctx->client_id;
    dto.command = PING;
    dto.len = 1;
    dto.data[0] = random;

    /* WRITE PING TO SERVER */
    if (common_write(&(ctx->client_sd), &dto) != 0)
    {
        ctx->client_id = UNSET_ID;
        return;
    }

    /* READ PING FROM SERVER */
    if (common_read(&(ctx->client_sd), &dto) != 0)
    {
        ctx->client_id = UNSET_ID;
        return;
    }

    /* CHECK PING DATA */
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
    memset(&client, 0, sizeof(client_t));
    client.client_sd = 0;
    client.client_id = UNSET_ID;
}

static bool is(client_t *ctx)
{
    return ((ctx->client_sd > 0) && (ctx->client_id >= 0));
}

static inline bool isCreated(client_t *ctx)
{
    return (ctx->client_sd > 0);
}

static inline bool isAssigned(client_t *ctx)
{
    return (ctx->client_id >= 0);
}

static inline bool isConnected(client_t *ctx)
{
    return isCreated(ctx) && isAssigned(ctx);
}

int main(int argc, char *argv[])
{
    initialize();

    if (argc != 2)
    {
        printf("\n Usage: %s <ip of server> \n", argv[0]);
        return 1;
    }

    while (1)
    {
        /* TRY CONNECT */
        if (!isCreated(&client))
        {
            printf("init\r\n");
            client_init(&client, argv[1]);
        }

        /* TRY ASSIGN */
        if (isCreated(&client) && !isAssigned(&client))
        {
            printf("assign\r\n");
            client_assign(&client);
        }

        /* PING SERVER */
        if (isConnected(&client))
        {
            client_ping(&client);
        }

        usleep(100000);
    }

    return 0;
}