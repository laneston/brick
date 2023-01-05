/**
 * @file listen.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-10-21
 *
 * @copyright Copyright (c) 2022
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LISTEN_H
#define __LISTEN_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "plugin_reg.h"
#include "route_client.h"
#include <pthread.h>
#include <stdbool.h>

    // queue_string
    typedef struct queue_string
    {
        char *strBuff;
        int strLen;
        char src_sock[128];
        char dir_sock[128];

        nng_socket sock;

    } queue_stringTypeDef;

    // route list message
    typedef struct routeLinkList
    {
        brickRoute_TypeDef *o_route; // route msg
        bool flag_rev;               // enable receive
        char routeName[128];         // route name
        char **relayNode;            // relay node name
        short relayNums;             // relay numbers
        pthread_t pid;               // pthread_t

        struct routeLinkList *next;

    } routeLinkList;

    typedef struct pluginRoute
    {
        routeLinkList *p_head;
        routeLinkList *p_tail;

        short pluginRouteNum;
        E_ModeStatus mode;

        pthread_mutex_t mutex;

    } pluginRouteTypeDef;

    /**
     * @brief  route_listen_wait
     * @note
     * @retval 0 if success, or other if failed.
     */
    extern int route_listen_wait(void);

    /**
     * @brief  route_listen_init
     * @note   route inited according to each plug-in
     * @param  *p_plugin:
     * @retval
     */
    extern int route_listen_init(pluginModuleTypeDef *p_plugin);

    /**
     * @brief  route_listen_destroy
     * @note
     * @retval 0 or -1
     */
    extern int route_listen_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* __LISTEN_H */
