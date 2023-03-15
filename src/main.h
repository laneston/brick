/**
 * @file main.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-03-08
 *
 * @copyright Copyright (c) 2022
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* c lib---------------------------------------------------------------------*/
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <netinet/in.h>
#include <pthread.h>
#include <pwd.h>
#include <semaphore.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <syslog.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

// #define NDEBUG
#include <assert.h>

#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

/**日志调用*/
#include "logPrintf.h"

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
#define ERROR(format, ...) printf("["__FILE__                                      \
                                  "][Line: %d][%s]: \033[31m" format "\033[32;0m", \
                                  __LINE__, __func__, ##__VA_ARGS__)
#define ERROR_ASSERT() ERROR("[ERROR]\n")

#endif /* __MAIN_H */
