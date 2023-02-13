/**
 * @file route_client.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-10-21
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "main.h"
#include "nng/nng.h"
#include "nng/protocol/pair1/pair.h"
#include "plugin.h"

/**
 * @brief  route_client_setDefault
 * @note   get the route handle.
 * @param  *p_plugin:
 * @retval the obj of the route client.
 */
brickRoute_TypeDef *route_client_setDefault(const brickPluginRegister_TypeDef *p_plugin)
{
    assert(p_plugin != NULL);

    int ret;

    brickRoute_TypeDef *t_route = (brickRoute_TypeDef *)malloc(sizeof(brickRoute_TypeDef));

    memset(t_route->url, 0x00, 256);
    sprintf(t_route->url, "ipc:///tmp/pair_%s.ipc", p_plugin->moduleName);
    if (nng_pair1_open(&t_route->socket) != 0)
    {
        ERROR_ASSERT();
        return NULL;
    }

    do
    {
        if ((ret = nng_dialer_create(&t_route->p_dialer, t_route->socket, t_route->url)) != 0)
        {
            printf("nng_dialer_create %s error:  ret %d\n", t_route->url, ret);
            continue;
        }

        if ((ret = nng_dialer_start(t_route->p_dialer, 0)) != 0)
        {
            printf("nng_dialer_start: %s ret %d\n", t_route->url, ret);
        }

        sleep(1);

    } while (ret != 0);

    t_route->flag_nonblock = 0; // default is block.

    return t_route;
}

/**
 * @brief  route_client_set
 * @note
 * @param  *p_plugin:
 * @param  timeout: 0~65535 ms
 * @param  flag_nonblock: The receive function returns immediately, set true or false.
 * @retval
 */
brickRoute_TypeDef *route_client_set(const brickPluginRegister_TypeDef *p_plugin, short timeout, bool flag_nonblock)
{
    assert(p_plugin != NULL);

    brickRoute_TypeDef *t_route = (brickRoute_TypeDef *)malloc(sizeof(brickRoute_TypeDef));
    int ret;

    memset(t_route->url, 0x00, 256);
    sprintf(t_route->url, "ipc:///tmp/pair_%s.ipc", p_plugin->moduleName);
    if (nng_pair1_open(&t_route->socket) != 0)
    {
        ERROR_ASSERT();
        return NULL;
    }

    do
    {
        if ((ret = nng_dialer_create(&t_route->p_dialer, t_route->socket, t_route->url)) != 0)
        {
            printf("nng_dialer_create %s error:  ret %d\n", t_route->url, ret);
            continue;
        }

        if ((ret = nng_dialer_start(t_route->p_dialer, 0)) != 0)
        {
            printf("nng_dialer_start: %s ret %d\n", t_route->url, ret);
        }

        sleep(1);

    } while (ret != 0);

    flag_nonblock ? (t_route->flag_nonblock = NNG_FLAG_NONBLOCK) : (t_route->flag_nonblock = 0);

    if (timeout > 0 && timeout < 65535)
    {
        if ((ret = nng_socket_set_ms(t_route->socket, NNG_OPT_RECVTIMEO, timeout)) != 0)
        {
            log_printf("nng_socket_set_ms: %d\n", ret);
            return NULL;
        }
    }

    return t_route;
}

/**
 * @brief  route_client_send
 * @note
 * @param  *t_route:
 * @param  *data:
 * @param  len:
 * @retval return 0 if success, or return other if failed.
 */
int route_client_send(brickRoute_TypeDef *t_route, void *data, size_t len)
{

    if ((t_route == NULL) || (data == NULL) || (len < 1))
    {
        printf("route_client_send: input param is NULL! or len less than 0!\n");
        return -1;
    }

    int rv = 0;

    // The returned memory can be used to hold message buffers,
    // in which case it can be directly passed to nng_send() using the flag NNG_FLAG_ALLOC.
    // Alternatively, it can be freed when no longer needed using nng_free().
    void *buf = (void *)malloc(sizeof(char) * len);
    memset(buf, 0x00, len);
    memcpy(buf, data, len);

    if ((rv = nng_send(t_route->socket, buf, len + 1, t_route->flag_nonblock | NNG_FLAG_ALLOC)) != 0)
    {
        printf("route_client_send: nng_send failed [%d]!\n", rv);
        return rv;
    }

    // nng_free(data, len + 1);

    return 0;
}

/**
 * @brief  route_client_recv
 * @note
 * @param  *t_route:
 * @param  *data:
 * @retval return 0 if success, or return other if failed.
 */
int route_client_recv(brickRoute_TypeDef *t_route, void *data)
{
    if ((t_route == NULL) || (data == NULL))
    {
        printf("route_client_recv >> input param is NULL or the len less than 0!\n");
        return -1;
    }

    char *buf = NULL;
    int rv;
    size_t sz;

    if ((rv = nng_recv(t_route->socket, &buf, &sz, t_route->flag_nonblock | NNG_FLAG_ALLOC)) == 0)
    {
        memcpy(data, (void *)buf, sz);
        rv = sz;
        nng_free(buf, sz);
    }
    else
        rv = 0;

    return rv;
}

/**
 * @brief  route_client_free
 * @note
 * @param  *t_route: the client handle
 * @retval
 */
int route_client_free(brickRoute_TypeDef *t_route)
{
    if (t_route == NULL)
    {
        printf("route_client_free: t_route is NULL");
        return -1;
    }

    free(t_route);
    t_route = NULL;

    return 0;
}
