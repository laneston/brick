/**
 * @file listen.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-10-21
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "listen.h"
#include "cJSON.h"
#include "main.h"
#include "nng/protocol/pair1/pair.h"
#include "thread_pool.h"

// Global declaration
threadPool_TypeDef *threadPool;
pluginRouteTypeDef *pluginRoute_handle;

/**
 *
 * @brief  listen_recv_loopback
 * @note   loopback test.
 * @param  *arg:
 * @retval None
 */
static void *listen_recv_loopback(void *arg)
{
    assert(arg != NULL);
    routeLinkList *p_route = (routeLinkList *)arg;

    char *buf = NULL;
    int rv;
    size_t sz;

    if ((rv = nng_socket_set_ms(p_route->o_route->socket, NNG_OPT_RECVTIMEO, 100)) != 0)
    {
        log_printf("nng_socket_set_ms: %d\n", rv);
        return NULL;
    }

    while (p_route->flag_rev)
    {
        if ((rv = nng_recv(p_route->o_route->socket, &buf, &sz, NNG_FLAG_ALLOC)) == 0)
        {
            if ((rv = nng_send(p_route->o_route->socket, buf, sz, NNG_FLAG_ALLOC)) != 0)
            {
                printf("listen_recv_loopback: nng_send failed [%d]!\n", rv);
            }
        }
    }

    return NULL;
}

static queue_stringTypeDef *queue_malloc(nng_socket sock, char *strBuff, int strLen, char *src_sock, char *dir_sock)
{

    if (strBuff == NULL)
    {
        return NULL;
    }

    queue_stringTypeDef *queue_temp = (queue_stringTypeDef *)malloc(sizeof(queue_stringTypeDef));
    queue_temp->strBuff = (char *)malloc(sizeof(char) * strLen);
    memcpy(queue_temp->strBuff, strBuff, strLen); // buffer
    queue_temp->strLen = strLen;                  // data length
    queue_temp->sock = sock;                      // socket

    if (src_sock != NULL)
    {
        memset(queue_temp->src_sock, 0x00, 128);
        strcpy(queue_temp->src_sock, src_sock);
    }
    if (dir_sock != NULL)
    {
        memset(queue_temp->dir_sock, 0x00, 128);
        strcpy(queue_temp->dir_sock, dir_sock);
    }

    return queue_temp;
}

static void queue_free(queue_stringTypeDef *queue_temp)
{
    if (queue_temp == NULL)
        return;

    free(queue_temp);
    queue_temp = NULL;
}

// receive process queue
static void *queue_relay(void *arg)
{
    unsigned short counter = 5; // 最大重传次数
    if (arg == NULL)
    {
        ERROR_ASSERT();
        return NULL;
    }

    queue_stringTypeDef *queue_o = arg;
    int rv;

    log_printf("[%s >> %s](%d) %s\n", queue_o->src_sock, queue_o->dir_sock, queue_o->strLen, queue_o->strBuff);
    while (counter--)
    {
        if ((rv = nng_send(queue_o->sock, queue_o->strBuff, queue_o->strLen, NNG_FLAG_NONBLOCK | NNG_FLAG_ALLOC)) != 0)
        {
            log_printf("queue_relay >> nng_send failed [%d]! [%s >> %s]\n", rv, queue_o->src_sock, queue_o->dir_sock);
        }
        else
        {
            break;
        }
        usleep(__RETRY_PERIOD_US_);
    }

    if (!counter)
    {
        // free the malloc space if failure.
        // ERROR_ASSERT();
        log_printf("queue_relay >> free the malloc space if failure.\n");
        if (queue_o->strBuff != NULL)
        {
            free(queue_o->strBuff);
            queue_o->strBuff = NULL;
        }
    }

    // free the malloc
    queue_free(queue_o);

    return NULL;
}

/**
 * @brief  listen_recv_mapping
 * @note   创建接收函数
 * @param  *arg:
 * @retval None
 */
static void *listen_recv_mapping(void *arg)
{
    assert(arg != NULL);
    routeLinkList *p_route = (routeLinkList *)arg;
    assert(p_route != NULL);

    assert(p_route->relayNums > 1);

    char *buf = NULL;
    int rv, counter = 0;
    size_t sz, buff_len;
    char buff[4096] = {0};
    char sock_name[16][128] = {0};

    routeLinkList *tmp_node = pluginRoute_handle->p_head;
    assert(tmp_node != NULL);

    // Cache socket
    log_printf("listen_recv_mapping >> p_route->relayNums: %d\n", p_route->relayNums);
    nng_socket *sock_tmp = (nng_socket *)malloc(sizeof(nng_socket) * p_route->relayNums + 1);
    assert(sock_tmp != NULL);

    // 对比节点名称，
    // for (size_t i = 0; (i < pluginRoute_handle->pluginRouteNum) && (tmp_node != NULL); i++)
    // {
    //     for (size_t j = 0; j < p_route->relayNums; j++)
    //     {
    //         if (!strcmp(p_route->relayNode[j], tmp_node->routeName))
    //         {
    //             printf("listen_recv_mapping: %s\n", tmp_node->routeName);
    //             sock_tmp[j] = tmp_node->o_route->socket;
    //         }
    //         else
    //         {
    //             log_printf("listen_recv_mapping >> relayNode: %s\n", p_route->relayNode[j]);
    //         }
    //     }
    //     tmp_node = tmp_node->next;
    // }

MAPPING_LOOP:
    for (size_t i = 0; i < p_route->relayNums; i++)
    {
        usleep(1000);
        tmp_node = pluginRoute_handle->p_head; // 每一轮将指针置到队列前端

        while (tmp_node->next != NULL)
        {
            if (!strcmp(p_route->relayNode[i], tmp_node->routeName))
            {
                log_printf("listen_recv_mapping >> routeName: %s\n", tmp_node->routeName);
                sock_tmp[i] = tmp_node->o_route->socket;
                strcpy(sock_name[i], tmp_node->routeName);
                /**设置发送方超时时间，避免程序阻塞*/
                // if ((rv = nng_socket_set_ms(tmp_node->o_route->socket, NNG_OPT_RECVTIMEO, 100)) != 0)
                // {
                //     log_printf("nng_socket_set_ms: %d\n", rv);
                //     return NULL;
                // }
                counter++; // 每轮计数+1
            }

            tmp_node = tmp_node->next;
        }
        if (!strcmp(p_route->relayNode[i], tmp_node->routeName))
        {
            log_printf("listen_recv_mapping >> routeName: %s\n", tmp_node->routeName);
            sock_tmp[i] = tmp_node->o_route->socket;
            strcpy(sock_name[i], tmp_node->routeName);
            /**设置发送方超时时间，避免程序阻塞*/
            // if ((rv = nng_socket_set_ms(tmp_node->o_route->socket, NNG_OPT_RECVTIMEO, 100)) != 0)
            // {
            //     log_printf("nng_socket_set_ms: %d\n", rv);
            //     return NULL;
            // }

            counter++; // 每轮计数+1
        }
    }

    // check the socket...
    nng_duration to;
    size_t nng_sz;
    nng_sz = sizeof(to);
    log_printf("nng_socket_get >> checking...\n");
    for (size_t i = 0; i < p_route->relayNums; i++)
    {
        rv = nng_socket_get(sock_tmp[i], NNG_OPT_RECVTIMEO, &to, &nng_sz);
        if (rv != 0)
        {
            log_printf("nng_socket_get >> %s - NNG_OPT_RECVTIMEO return val: %d\n", sock_name[i], rv);
            goto MAPPING_LOOP;
        }
        rv = nng_socket_get(sock_tmp[i], NNG_OPT_SENDTIMEO, &to, &nng_sz);
        if (rv != 0)
        {
            log_printf("nng_socket_get >> %s - NNG_OPT_SENDTIMEO return val: %d\n", sock_name[i], rv);
            goto MAPPING_LOOP;
        }
        rv = nng_socket_get(sock_tmp[i], NNG_OPT_RECONNMAXT, &to, &nng_sz);
        if (rv != 0)
        {
            log_printf("nng_socket_get >> %s - NNG_OPT_RECONNMAXT return val: %d\n", sock_name[i], rv);
            goto MAPPING_LOOP;
        }
        rv = nng_socket_get(sock_tmp[i], NNG_OPT_RECONNMINT, &to, &nng_sz);
        if (rv != 0)
        {
            log_printf("nng_socket_get >> %s - NNG_OPT_RECONNMINT return val: %d\n", sock_name[i], rv);
            goto MAPPING_LOOP;
        }
    }
    log_printf("nng_socket_get >> checked.\n");

    log_printf("listen_recv_mapping >> relayNums: %d, counter: %d\n", p_route->relayNums, counter);
    assert(counter == p_route->relayNums); // 计数应相同

    if ((rv = nng_socket_set_ms(p_route->o_route->socket, NNG_OPT_RECVTIMEO, 100)) != 0)
    {
        log_printf("nng_socket_set_ms: error code %d\n", rv);
        return NULL;
    }

    queue_stringTypeDef *queue_temp = NULL;

    while (p_route->flag_rev)
    {
        if ((rv = nng_recv(p_route->o_route->socket, &buf, &sz, NNG_FLAG_ALLOC)) == 0)
        {
            // In order to increase the asynchronous receiving efficiency,
            // the group sending operation is processed in the thread pool.
            // free the tmp_buf in task function
            memset(buff, 0x00, 4096);
            memcpy(buff, buf, sz + 1);
            buff_len = sz;

            for (size_t i = 0; i < p_route->relayNums; i++)
            {
                // 优化方向：将发送函数放入线程池处理，使发送操作可并发
                // if ((rv = nng_send(sock_tmp[i], buff, buff_len, NNG_FLAG_ALLOC)) != 0)
                //     printf("listen_recv_mapping: nng_send failed [%d]!\n", rv);

                // 制作结构体参数
                queue_temp = queue_malloc(sock_tmp[i], buff, buff_len, p_route->routeName, sock_name[i]);
                if (queue_temp == NULL)
                {
                    log_printf("queue_malloc error!\n");
                    continue;
                }
                // 将任务放入线程池
                if (threadpool_add_task(threadPool, queue_relay, (void *)queue_temp) < 0)
                {
                    log_printf("threadpool_add_task: queue_relay error!\n");
                }
            }

            nng_free(buf, sz);
        }
    }

    free(sock_tmp);
    sock_tmp = NULL;

    return NULL;
}

/**
 * @brief  listen_recv_mapping_pair
 * @note
 * @param  *arg:
 * @retval None
 */
static void *listen_recv_mapping_pair(void *arg)
{
    assert(arg != NULL);
    routeLinkList *p_route = (routeLinkList *)arg;
    assert(p_route->relayNums == 1);

    char *buf = NULL;
    int rv;
    size_t sz;
    // unsigned short counter = 0;
    routeLinkList *tmp_node = pluginRoute_handle->p_head;
    assert(tmp_node != NULL);

PAIR_LOOP:
    // 找出与当前节点名称相同的节点信息
    for (size_t i = 0; (i < pluginRoute_handle->pluginRouteNum) && (tmp_node != NULL); i++)
    {
        usleep(1000);
        if (!strcmp(p_route->relayNode[0], tmp_node->routeName))
        {
            printf("listen_recv_mapping_pair: %s\n", p_route->relayNode[0]);
            /**设置发送方超时时间，避免程序阻塞*/
            // if ((rv = nng_socket_set_ms(tmp_node->o_route->socket, NNG_OPT_RECVTIMEO, 100)) != 0)
            // {
            //     log_printf("nng_socket_set_ms: %d\n", rv);
            //     return NULL;
            // }
            break;
        }
        tmp_node = tmp_node->next;
    }

    if ((rv = nng_socket_set_ms(p_route->o_route->socket, NNG_OPT_RECVTIMEO, 100)) != 0)
    {
        log_printf("nng_socket_set_ms: %d\n", rv);
        return NULL;
    }

    nng_duration to;
    size_t nng_sz;
    nng_sz = sizeof(to);
    log_printf("nng_socket_get >> checking...\n");
    rv = nng_socket_get(tmp_node->o_route->socket, NNG_OPT_RECVTIMEO, &to, &nng_sz);
    if (rv != 0)
    {
        log_printf("nng_socket_get >> %s - NNG_OPT_RECVTIMEO return val: %d\n", tmp_node->routeName, rv);
        goto PAIR_LOOP;
    }
    rv = nng_socket_get(tmp_node->o_route->socket, NNG_OPT_SENDTIMEO, &to, &nng_sz);
    if (rv != 0)
    {
        log_printf("nng_socket_get >> %s - NNG_OPT_SENDTIMEO return val: %d\n", tmp_node->routeName, rv);
        goto PAIR_LOOP;
    }
    rv = nng_socket_get(tmp_node->o_route->socket, NNG_OPT_RECONNMAXT, &to, &nng_sz);
    if (rv != 0)
    {
        log_printf("nng_socket_get >> %s - NNG_OPT_RECONNMAXT return val: %d\n", tmp_node->routeName, rv);
        goto PAIR_LOOP;
    }
    rv = nng_socket_get(tmp_node->o_route->socket, NNG_OPT_RECONNMINT, &to, &nng_sz);
    if (rv != 0)
    {
        log_printf("nng_socket_get >> %s - NNG_OPT_RECONNMINT return val: %d\n", tmp_node->routeName, rv);
        goto PAIR_LOOP;
    }
    log_printf("nng_socket_get >> checked.\n");

    queue_stringTypeDef *queue_temp = NULL;

    while (p_route->flag_rev)
    {
        // 接收p_route->o_route->socket数据，发送至tmp_node->o_route->socket
        if ((rv = nng_recv(p_route->o_route->socket, &buf, &sz, NNG_FLAG_ALLOC)) == 0)
        {
            // 使用线程池实现将传输数据动作，增加重传操作，且不影响数据接收
            // 制作结构体参数
            queue_temp = queue_malloc(tmp_node->o_route->socket, buf, sz, p_route->routeName, tmp_node->routeName);
            if (queue_temp == NULL)
            {
                log_printf("queue_malloc error!\n");
                continue;
            }
            // 将任务放入线程池
            if ((rv = threadpool_add_task(threadPool, queue_relay, (void *)queue_temp)) < 0)
            {
                log_printf("threadpool_add_task >> queue_relay error [%d]!\n", rv);
            }

            nng_free(buf, sz);
        }
    }

    return NULL;
}

/**
 * @brief  route_listen_nodeCreate
 * @note
 * @param  *pluginName:
 * @retval
 */
static int route_listen_nodeCreate(routeLinkList *p_route, void *(*task_func)(void *))
{
    assert(p_route != NULL);
    int ret;

    printf("route_listen_nodeCreate: %s\n", p_route->routeName);

    if ((ret = nng_pair1_open(&p_route->o_route->socket)) != 0)
    {
        log_printf("route_listen_nodeCreate: nng_pair1_open error! [%d]\n", ret);
        return -1;
    }

    if ((ret = nng_listener_create(&p_route->o_route->p_listener, p_route->o_route->socket, p_route->o_route->url)) != 0)
    {
        log_printf("nng_listener_create: ret %d\n", ret);
        return -1;
    }

    if ((ret = nng_listener_start(p_route->o_route->p_listener, 0)) != 0)
    {
        log_printf("nng_listener_start: ret %d\n", ret);
        return -1;
    }

    // 将接收函数放入线程池
    //  if (threadpool_add_task(threadPool, task_func, (void *)p_route) < 0)
    //  {
    //      ERROR_ASSERT();
    //      return -1;
    //  }
    // 线城池提供给非LOOP任务函数
    // 创建接收函数线程
    ret = pthread_create(&p_route->pid, NULL, task_func, (void *)p_route);
    assert(ret == 0);
    log_printf("route_listen_nodeCreate >> [pthread_create->pthreads: %ld]\n", p_route->pid);

    return 0;
}

/**
 * @brief  route_listen_routeCreate
 * @note
 * @param  *p_route:
 * @retval
 */
static int route_listen_routeCreate(pluginRouteTypeDef *p_route)
{
    assert(p_route != NULL);
    int ret;

    routeLinkList *t_route = p_route->p_head;
    assert(t_route != NULL);

    switch (p_route->mode)
    {
    case LOOPBACK:
        for (size_t i = 0; i < p_route->pluginRouteNum; i++)
        {
            if ((ret = route_listen_nodeCreate(t_route, listen_recv_loopback)) < 0)
            {
                ERROR("route_listen_routeCreate: route_listen_nodeCreate!\n");
                return -1;
            }
            t_route = t_route->next;
        }
        break;

    case RELAY:
    case ROUTER:
        printf("route_listen_routeCreate >> pluginRouteNum: %d\n", p_route->pluginRouteNum);
        for (size_t i = 0; i < p_route->pluginRouteNum; i++)
        {
            // the router with single node
            if (t_route->relayNums == 1)
            {
                log_printf("route_listen_nodeCreate >>> listen_recv_mapping_pair\n");
                if ((ret = route_listen_nodeCreate(t_route, listen_recv_mapping_pair)) < 0)
                {
                    ERROR("route_listen_routeCreate: route_listen_nodeCreate!\n");
                    return -1;
                }
            }
            // the router with multi node
            else
            {
                log_printf("route_listen_nodeCreate >>> listen_recv_mapping\n");
                if ((ret = route_listen_nodeCreate(t_route, listen_recv_mapping)) < 0)
                {
                    ERROR("route_listen_routeCreate: route_listen_nodeCreate!\n");
                    return -1;
                }
            }

            t_route = t_route->next;
        }
        break;

    default:
        ERROR_ASSERT();
        break;
    }

    return 0;
}

/**
 * @brief  route_listen_wait
 * @note
 * @retval 0 if success, or other if failed.
 */
int route_listen_wait(void)
{
    return (threadPool_wait(threadPool));
}

/**
 * @brief  route_listen_nodeAdd
 * @note
 * @param  *nodeName:
 * @param  **relayNode:
 * @param  relayNums:
 * @retval
 */
static int route_listen_nodeAdd(const char *nodeName, char **relayNode, short relayNums)
{
    assert(((nodeName != NULL) && (relayNode != NULL)));
    assert(relayNums >= 0);

    routeLinkList *pdata = (routeLinkList *)malloc(sizeof(routeLinkList));
    assert(pdata != NULL);

    pdata->o_route = (brickRoute_TypeDef *)malloc(sizeof(brickRoute_TypeDef));
    assert(pdata->o_route != NULL);

    memset(pdata->o_route->url, 0x00, 256);
    sprintf(pdata->o_route->url, "ipc:///tmp/pair_%s.ipc", nodeName);

    pdata->flag_rev = true;

    memset(pdata->routeName, 0x00, 128);
    strcpy(pdata->routeName, nodeName);

    pdata->relayNode = (char **)malloc(sizeof(char *) * relayNums);
    assert(pdata->relayNode != NULL);

    for (short i = 0; i < relayNums; i++)
    {
        // printf("route_listen_nodeAdd: relayNode %s\n", relayNode[i]);
        pdata->relayNode[i] = (char *)malloc(sizeof(char) * 128);
        assert(pdata->relayNode[i] != NULL);
        memset(pdata->relayNode[i], 0x00, 128);
        strcpy(pdata->relayNode[i], relayNode[i]);
    }

    pdata->relayNums = relayNums;
    pdata->next = NULL;

    // Insert Node
    if (pluginRoute_handle->p_head == NULL) // if the first pack
    {
        pluginRoute_handle->p_head = pluginRoute_handle->p_tail = pdata;
    }
    else
    {
        pdata->next = pluginRoute_handle->p_tail->next;
        pluginRoute_handle->p_tail->next = pdata;
        pluginRoute_handle->p_tail = pdata;
    }

    // pluginRoute_handle->pluginRouteNum++;
    // log_printf("route_listen_nodeAdd: [pluginRouteNum %d]\n", pluginRoute_handle->pluginRouteNum);

    return 0;
}

/**
 * @brief  route_listen_getMapping
 * @note
 * @param  *configFile:
 * @retval
 */
static int route_listen_getMapping(pluginModuleTypeDef *p_plugin)
{
    assert(p_plugin != NULL);

    static char configString[FILE_ROUTE_LEN] = {0};

    int FILE_p = open(FILE_ROUTE_DIR, O_RDONLY);
    int read_len = read(FILE_p, configString, FILE_ROUTE_LEN);
    if (read_len > 0)
    {
        // log_printf("route_listen_getMapping: [read_len %d]\n%s\n", read_len, configString);
        close(FILE_p);
    }
    else
    {
        ERROR("openFile error!\n");
        close(FILE_p);
        return -1;
    }

    cJSON *root = cJSON_Parse(configString);
    if (root == NULL)
    {
        log_printf("route_listen_getMapping: cJSON_Parse error!\n");
        return -1;
    }
    else
    {
        if (root->type != cJSON_Object)
        {
            log_printf("route_listen_getMapping: root->type error!\n");
            return -1;
        }
    }

    // 解析JSON文件
    pluginRegLinkList *node = p_plugin->p_head;
    pluginRegLinkList *node_tmp = NULL;

    cJSON *item = NULL;
    cJSON *item_route = NULL;
    cJSON *array_route = NULL;
    int array_size = 0;
    char **array_node = (char **)malloc(sizeof(char *) * 64); // 64 strings
    short node_count = 0;

    // printf("route_listen_getMapping: p_plugin->moduleNum %d\n", p_plugin->moduleNum);
    for (size_t i = 0; (i < p_plugin->moduleNum) && (node != NULL); i++)
    {
        // 获取插件同名路由信息
        item = cJSON_GetObjectItem(root, node->moduleMsg->moduleName);
        if (item == NULL)
        {
            log_printf("route_listen_getMapping [ERROR] item = cJSON_GetObjectItem(root, node->moduleMsg->moduleName);\n");
            node = node->next;
            continue;
        }
        else
        {
            if (item->type != cJSON_Object)
            {
                log_printf("route_listen_getMapping [ERROR] if (item->type != cJSON_Object)\n");
                node = node->next;
                continue;
            }
        }

        item_route = cJSON_GetObjectItem(item, "route");
        if (item_route == NULL)
        {
            log_printf("route_listen_getMapping [ERROR] item_route = cJSON_GetObjectItem(item, \"route\");\n");
            node = node->next;
            continue;
        }
        else
        {
            if (item_route->type != cJSON_Array)
            {
                log_printf("route_listen_getMapping [ERROR] if (item_route->type != cJSON_Array)\n");
                node = node->next;
                continue;
            }
        }

        array_size = cJSON_GetArraySize(item_route);
        // printf("route_listen_getMapping: array_size %d\n", array_size);

        for (size_t j = 0; j < array_size; j++)
        {
            array_route = cJSON_GetArrayItem(item_route, j);
            if (array_route == NULL)
            {
                log_printf("route_listen_getMapping: cJSON_GetObjectItem error!\n");
                continue;
            }
            else
            {
                if (array_route->type != cJSON_String)
                {
                    log_printf("route_listen_getMapping: array_route->type error!\n");
                    continue;
                }
            }

            node_tmp = p_plugin->p_head;
            for (size_t k = 0; (k < p_plugin->moduleNum) && (node_tmp != NULL); k++)
            {
                if (!strcmp(array_route->valuestring, node_tmp->moduleMsg->moduleName))
                {
                    array_node[node_count] = (char *)malloc(sizeof(char) * 128); // 128 bytes

                    // 将插件名称填入路由节点, 将映射节点填入路由节点中
                    memset(array_node[node_count], 0x00, 128);
                    strcpy(array_node[node_count], array_route->valuestring);
                    printf("array_node[%d]: %s\n", node_count, array_node[node_count]);
                    node_count++;
                }
                node_tmp = node_tmp->next;
            }
        }

        route_listen_nodeAdd(node->moduleMsg->moduleName, array_node, node_count);
        for (size_t k = 0; k < node_count; k++)
        {
            free(array_node[k]);
            array_node[k] = NULL;
        }
        node_count = 0;

        node = node->next;
    }

    return 0;
}

/**
 * @brief  route_listen_init
 * @note   route inited according to each plug-in
 * @param  *pluginRoute_handle:
 * @retval return 0 is success or other if failed.
 */
int route_listen_init(pluginModuleTypeDef *p_plugin)
{
    assert(p_plugin != NULL);

    // create the pthread.
    threadPool = threadPool_init(THREADPOOL_NUMS);
    assert(threadPool != NULL);

    // init the pluginRouteTypeDef
    pluginRoute_handle = (pluginRouteTypeDef *)malloc(sizeof(pluginRouteTypeDef));
    assert(pluginRoute_handle != NULL);

    pluginRoute_handle->p_head = NULL;
    pluginRoute_handle->p_tail = NULL;
    pluginRoute_handle->pluginRouteNum = 0;

    int ret = pthread_mutex_init(&(pluginRoute_handle->mutex), NULL);
    assert(ret == 0);

    pluginRoute_handle->mode = p_plugin->mode;
    pluginRoute_handle->pluginRouteNum = p_plugin->moduleNum;

    // get the route mapping list, for knowing what the route to point.
    if (route_listen_getMapping(p_plugin) < 0)
    {
        log_printf("route_listen_getMapping error!\n");
        return -1;
    }

    if (route_listen_routeCreate(pluginRoute_handle) < 0)
    {
        log_printf("route_listen_routeCreate error!\n");
        return -1;
    }

    return 0;
}

/**
 * @brief  route_listen_destroy
 * @note
 * @retval 0 or -1
 */
int route_listen_destroy(void)
{
    int ret = threadPool_destroy(threadPool);
    assert(ret == 0);

    free(pluginRoute_handle);

    return 0;
}