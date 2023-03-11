/**
 * @file main.c
 * @author lanceli (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-03-08
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "main.h"
#include "brick_init.h"
#include "getopt.h"
#include "sighandle.h"

/**
 * @brief mian interface
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char **argv)
{
    /* configure signal handling */
    signal(SIGPIPE, signal_handler);
    signal(SIGINT, signal_handler);

#ifdef RSYSLOG
    printf("rsyslog star openlog...\n");
    openlog("brick", LOG_PID | LOG_NDELAY, LOG_USER);
#endif

    /* parse command line options */
    if (optionGet(argc, argv) < 0)
    {
        ERROR_ASSERT();
        exit(-1);
    }

    if (brick_init() < 0)
    {
        ERROR_ASSERT();
    }

    pause();

    return 0;
}
