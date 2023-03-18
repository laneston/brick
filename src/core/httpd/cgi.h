/**
 * @file cgi.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-10-21
 *
 * @copyright Copyright (c) 2022
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CGI_H
#define __CGI_H

#ifdef __cplusplus
extern "C"
{
#endif

    extern void execute_cgi(int client, const char *path,
                            const char *method, const char *query_string);
#ifdef __cplusplus
}
#endif

#endif /* __CGI_H */