/**
 * @file cgi.c
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

#define STDIN 0
#define STDOUT 1

/**********************************************************************/
/* Inform the client that a CGI script could not be executed.
 * Parameter: the client socket descriptor. */
/**********************************************************************/
static void cannot_execute(int client)
{
    char buf[1024];

    sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<P>Error prohibited CGI execution.\r\n");
    send(client, buf, strlen(buf), 0);
}

/**********************************************************************/
/* Inform the client that a request it has made has a problem.
 * Parameters: client socket */
/**********************************************************************/
static void bad_request(int client)
{
    char buf[1024];

    sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "Content-type: text/html\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "<P>Your browser sent a bad request, ");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "such as a POST without a Content-Length.\r\n");
    send(client, buf, sizeof(buf), 0);
}

/**********************************************************************/
/* Execute a CGI script.  Will need to set environment variables as
 * appropriate.
 * Parameters: client socket descriptor
 *             path to the CGI script */
/**********************************************************************/
void execute_cgi(int client, const char *path, const char *method, const char *query_string)
{
    char buf[1024];
    int cgi_output[2];
    int cgi_input[2];
    pid_t pid;
    int status;
    int i;
    char c;
    int numchars = 1;
    int content_length = -1;

    buf[0] = 'A';
    buf[1] = '\0';

    if (strcasecmp(method, "GET") == 0)
    {
        while ((numchars > 0) && strcmp("\n", buf)) /* read & discard headers */
            numchars = get_line(client, buf, sizeof(buf));
    }
    else if (strcasecmp(method, "POST") == 0) /*POST*/
    {
        numchars = get_line(client, buf, sizeof(buf));

        while ((numchars > 0) && strcmp("\n", buf))
        {
            buf[15] = '\0';

            if (strcasecmp(buf, "Content-Length:") == 0)
                content_length = atoi(&(buf[16]));

            numchars = get_line(client, buf, sizeof(buf));
        }

        if (content_length == -1)
        {
            bad_request(client);
            return;
        }
    }
    else /*HEAD or other*/
    {
        printf("execute_cgi >> HEAD or othe\n");
    }

    if (pipe(cgi_output) < 0)
    {
        cannot_execute(client);
        return;
    }
    if (pipe(cgi_input) < 0)
    {
        cannot_execute(client);
        return;
    }

    if ((pid = fork()) < 0)
    {
        cannot_execute(client);
        return;
    }

    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    send(client, buf, strlen(buf), 0);

    if (pid == 0) /* child: CGI script */
    {
        char meth_env[255];
        char query_env[255];
        char length_env[255];

        dup2(cgi_output[1], STDOUT);
        dup2(cgi_input[0], STDIN);
        close(cgi_output[0]);
        close(cgi_input[1]);
        sprintf(meth_env, "REQUEST_METHOD=%s", method);
        putenv(meth_env);

        if (strcasecmp(method, "GET") == 0)
        {
            sprintf(query_env, "QUERY_STRING=%s", query_string);
            putenv(query_env);
        }
        else /* POST */
        {
            sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
            putenv(length_env);
        }

        execl(path, NULL);

        exit(0);
    }
    else /* parent */
    {
        close(cgi_output[1]);
        close(cgi_input[0]);

        if (strcasecmp(method, "POST") == 0)
        {
            for (i = 0; i < content_length; i++)
            {
                recv(client, &c, 1, 0);
                write(cgi_input[1], &c, 1);
            }
        }

        while (read(cgi_output[0], &c, 1) > 0)
            send(client, &c, 1, 0);

        close(cgi_output[0]);
        close(cgi_input[1]);

        waitpid(pid, &status, 0);
    }
}
