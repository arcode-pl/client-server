#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <curses.h>
#include "dto.h"

#define MAX_CLIENTS 10

typedef struct
{
    int server_sd;
    struct sockaddr_in serv_addr;
    socklen_t sock_len;
    int clients_sd[MAX_CLIENTS];
} server_t;

static server_t server;
static dto_t dto;

int server_init(server_t *ctx)
{
    /* CREATE */
    ctx->server_sd = socket(AF_INET, SOCK_STREAM, 0);
    if (ctx->server_sd < 0)
    {
        perror("socket failed");
        return -1;
    }

    /* CONFIG */

    //memset(&ctx->serv_addr, 0, ctx->sock_len);
    ctx->serv_addr.sin_family = AF_INET;
    ctx->serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ctx->serv_addr.sin_port = htons(PORT_ADDRESS);
    ctx->sock_len = sizeof(ctx->serv_addr);
    if (bind(ctx->server_sd, (struct sockaddr *)&(ctx->serv_addr), ctx->sock_len) < 0)
    {
        perror("bind failed");
        return -1;
    }

    /* LISTEN */
    if (listen(ctx->server_sd, 10) < 0)
    {
        perror("listen");
        return -1;
    }

    return 0;
}

static timeval select_timeout_tv;
static char sendBuff[256];
static fd_set read_sds;

int server_listen(server_t *ctx, int timeout_ms)
{
    int max_sd;
    int activity;
    time_t ticks;

    select_timeout_tv.tv_sec = 0;
    select_timeout_tv.tv_usec = timeout_ms * 1000;

    FD_ZERO(&read_sds);
    FD_SET(ctx->server_sd, &read_sds);
    max_sd = ctx->server_sd;

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        int sd = ctx->clients_sd[i];
        if (sd > 0)
        {
            FD_SET(sd, &read_sds);
        }
        if (sd > max_sd)
        {
            max_sd = sd;
        }
    }

    int ready_sds = select(max_sd + 1, &read_sds, NULL, NULL, &select_timeout_tv);
    if (ready_sds > 0)
    {
        if (FD_ISSET(ctx->server_sd, &read_sds))
        {
            int client_sd;
            if ((client_sd = accept(ctx->server_sd, (struct sockaddr *)&(ctx->serv_addr),
                                    (socklen_t *)&(ctx->sock_len))) < 0)
            {
                printf("accept error\r\n");
                return 0;
            }

            printf("New connection , socket fd is %d , ip is : %s , port : %d\r\n", client_sd, inet_ntoa(ctx->serv_addr.sin_addr), ntohs(ctx->serv_addr.sin_port));

            //add new socket to array of sockets
            dto.id = -1;
            dto.command = MAX_CLIENTS_REACHED;
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (ctx->clients_sd[i] == 0)
                {
                    ctx->clients_sd[i] = client_sd;
                    printf("Adding to list of sockets as %d\r\n", i);

                    dto.id = i;
                    dto.command = NEW_CLIENT;
                    break;
                }
            }

            ticks = time(NULL);
            snprintf((char *)&(dto.data), sizeof(dto.data), "%.24s\r\n", ctime(&ticks));
            dto.len = strlen((char *)&(dto.data));

            // REPLY TO NEWLY CONNECTED CLIENT WITH TRUE OR FALSE
            common_write(client_sd, &dto);
        }
    }

    return 0;
}

void server_close(server_t *ctx)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (ctx->clients_sd[i] != 0)
        {
            close(ctx->clients_sd[i]);
        }
    }
}

void server_ping(server_t *ctx)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (ctx->clients_sd[i] != 0)
        {
            common_read(ctx->clients_sd[i], &dto);
            dto.id = SERVER_ID;
            dto.data[0] = dto.data[0] + 1;
            common_write(ctx->clients_sd[i], &dto);
        }
    }
}

int main(int argc, char *argv[])
{
    int ch;
    int i = 0;

    initscr();
    clear();
    noecho();
    nodelay(stdscr, TRUE);
    cbreak();
    timeout(1000);

    memset(&server, 0, sizeof(server_t));
    server_init(&server);

    while ((ch = getch()) != 27)
    {
        server_listen(&server, 1);
        server_ping(&server);

        usleep(10);
    }

    server_close(&server);
    endwin();

    return 0;
}