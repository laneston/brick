/**
 * @file log_printf.h
 * @author lanceli (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-03-08
 *
 * @copyright Copyright (c) 2022
 *
 */



/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LOG_PRINTF_H
#define __LOG_PRINTF_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif




    /**
     * @brief  log_printf
     * @note
     * @param  *fmt:
     * @retval None
     */
    extern void log_printf(const char *fmt, ...);



#ifdef __cplusplus
}
#endif

#endif /* __LOG_PRINTF_H */
