/**
 * @file plugin.h
 * @author lanceli (you@domain.com)
 * @brief plugin register
 * @version 0.1
 * @date 2022-10-21
 *
 * @copyright Copyright (c) 2022
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PLUGIN_H
#define __PLUGIN_H

#ifdef __cplusplus
extern "C"
{
#endif

// the module enable congfig file direction.
#define FILE_PLUGIN_DIR "./routeMapping.json"
#define FILE_PLUGIN_LEN 2048

// the route mapping config file
#define FILE_ROUTE_DIR "./routeMapping.json"
#define FILE_ROUTE_LEN 2048

#define THREADPOOL_NUMS 32
#define QUEUE_MAX_NUM (THREADPOOL_NUMS * 2)

// the target name and direction.
#define __TARGET_NAME__ "brick"
#define __TARGET_DIR__ "/userdata/dgri/brick"
    // #define __TARGET_DIR__ "/root/brick"

#include "route_client.h"

    /**
     * @brief  brickPluginRegister
     * @note   The following interfaces are provided for BRICK calls.
     */
    typedef struct brickPluginRegister
    {

        int (*init)(void);      // Used to initialize process resources
        int (*destroy)(void);   // Used to destroy process resources
        int (*start)(void);     // Enable process when BRICK is called
        int (*stop)(void);      // Disable process when BRICK is called
        char moduleName[128];   // Plug in name
        char moduleVersion[64]; // Plug in version

    } brickPluginRegister_TypeDef;

    // plugin register array declare
    typedef const brickPluginRegister_TypeDef *brickPluginRegister_regArray;

    // route handle declare
    typedef brickRoute_TypeDef brickRouteClient_TypeDef;

    /**
     * @brief  route_client_setDefault
     * @note   get the route handle.
     * @param  *p_plugin:
     * @retval the obj of the route client.
     */
    extern brickRouteClient_TypeDef *route_client_setDefault(const brickPluginRegister_TypeDef *p_plugin);

    /**
     * @brief  route_client_set
     * @note
     * @param  *p_plugin:
     * @param  timeout: 0~65535 ms
     * @param  flag_nonblock: The receive function returns immediately, set true or false.
     * @retval
     */
    extern brickRoute_TypeDef *route_client_set(const brickPluginRegister_TypeDef *p_plugin, short timeout, bool flag_nonblock);

    /**
     * @brief  route_client_free
     * @note
     * @param  *t_route: the client handle
     * @retval
     */
    extern int route_client_free(brickRoute_TypeDef *t_route);

    /**
     * @brief  route_client_send
     * @note
     * @param  *t_route:
     * @param  *data:
     * @param  len:
     * @retval return 0 if success, or return other if failed.
     */
    extern int route_client_send(brickRoute_TypeDef *t_route, void *data, size_t len);

    /**
     * @brief  route_client_recv
     * @note
     * @param  *t_route:
     * @param  *data:
     * @retval return 0 if success, or return other if failed.
     */
    extern int route_client_recv(brickRoute_TypeDef *t_route, void *data);

#ifdef __cplusplus
}
#endif

#endif /* __PLUGIN_H */
