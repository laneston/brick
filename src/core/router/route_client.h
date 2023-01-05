/**
 * @file route_client.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-10-21
 *
 * @copyright Copyright (c) 2022
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ROUTE_CLIENT_H
#define __ROUTE_CLIENT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "nng/nng.h"

    /**
     * @brief  brickRoute_TypeDef
     * @note   插件路由信息
     * @retval None
     */
    typedef struct brickRoute
    {
        nng_socket socket;       // socket
        nng_listener p_listener; // listen handler
        nng_dialer p_dialer;     // dialer handler
        char url[256];           // url describe
        int flag_nonblock;       // nonblock flag
        short timeout;

    } brickRoute_TypeDef;

#ifdef __cplusplus
}
#endif

#endif /* __ROUTE_CLIENT_H */
