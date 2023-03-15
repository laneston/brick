/**
 * @file signalhandle.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-10-17
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "main.h"
#include "plugin_init.h"

/**
 * @brief signal_handler
 *
 * @param sig
 */
void signal_handler(int sig)
{
    switch (sig)
    {
    case SIGINT: /* ctrl + c */
        printf("\n<<< the brick exit! >>>\n");
        plugin_destroy();
        exit(0);
        break;

    case SIGPIPE:
        break;

    default:
        break;
    }
}
