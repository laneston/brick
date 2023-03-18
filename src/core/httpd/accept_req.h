/**
 * @file accept_req.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-10-21
 *
 * @copyright Copyright (c) 2022
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ACCEPT_REQ_H
#define __ACCEPT_REQ_H

#ifdef __cplusplus
extern "C"
{
#endif

    extern void accept_request(void *arg);
	extern int get_line(int sock, char *buf, int size);

#ifdef __cplusplus
}
#endif

#endif /* __ACCEPT_REQ_H */