/**
 * @file httpd_listen.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-10-21
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "accept_req.h"
#include "main.h"
#include <stdbool.h>


volatile int server_sock = -1;

/**********************************************************************/
/* Print out an error message with perror() (for system errors; based
 * on value of errno, which indicates system call errors) and exit the
 * program indicating an error. */
/**********************************************************************/
static void error_die(const char *sc)
{
    perror(sc);
    exit(1);
}

/**********************************************************************/
/* This function starts the process of listening for web connections
 * on a specified port.  If the port is 0, then dynamically allocate a
 * port and modify the original port variable to reflect the actual
 * port.
 * Parameters: pointer to variable containing the port to connect on
 * Returns: the socket */
/**********************************************************************/
static int startup(u_short *port)
{
    int httpd = 0;
    int on = 1;
    struct sockaddr_in name;

    assert(port != NULL);

    /// 1. socket init.
    httpd = socket(PF_INET, SOCK_STREAM, 0);
    if (httpd == -1)
        error_die("socket");

    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_port = htons(*port); // 将端口号由主机字节序转换为网络字节序的整数值
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    if ((setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0)
    {
        error_die("setsockopt failed");
    }

    /// 2. socket bind.
    if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0)
        error_die("bind");

    if (*port == 0) /* if dynamically allocating a port */
    {
        socklen_t namelen = sizeof(name);
        if (getsockname(httpd, (struct sockaddr *)&name, &namelen) == -1)
            error_die("getsockname");
        *port = ntohs(name.sin_port);
    }

    /// 3. socket listen.
    if (listen(httpd, 5) < 0)
        error_die("listen");

    return (httpd);
}

/**
 * httpd start function.
 */
int httpd_init(void)
{

    
    u_short port = 4000;
    int client_sock = -1;
    struct sockaddr_in client_name;
    socklen_t client_name_len = sizeof(client_name);
    pthread_t newthread;

    server_sock = startup(&port);
    log_printf("httpd_init >> httpd running on port %d\n", port);

    while (true)
    {
        /// 4. socket accept.
        client_sock = accept(server_sock, (struct sockaddr *)&client_name, &client_name_len);
        printf("accept >> client_sock: %d\n", client_sock);

        if (client_sock == -1)
            error_die("accept");

        /* accept_request(&client_sock); */
        if (pthread_create(&newthread, NULL, (void *)accept_request, (void *)(intptr_t)client_sock) != 0)
            perror("pthread_create");
    }

    return 0;
}

/**
 * http server destroy.
 */
int httpd_destroy(void)
{
    close(server_sock);
    return 0;
}
