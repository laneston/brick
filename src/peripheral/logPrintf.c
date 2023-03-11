/**
 * @file log_printf.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-03-08
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "main.h"

#ifdef RSYSLOG
/**
 * @brief  log_printf
 * @note
 * @param  *fmt:
 * @retval None
 */
void log_printf(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vsyslog(LOG_INFO, fmt, ap);
    vprintf(fmt, ap);
    va_end(ap);
}
#else
/**
 * @brief  log_printf
 * @note
 * @param  *fmt:
 * @retval None
 */
void log_printf(const char *fmt, ...)
{
    va_list ap;
    struct tm *pTM;
    time_t nSeconds;

    va_start(ap, fmt);

    time(&nSeconds);
    pTM = localtime(&nSeconds);

    printf("%4d-%02d-%02d %02d:%02d:%02d ", pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
    vprintf(fmt, ap);

    va_end(ap);
}
#endif
