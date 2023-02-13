/**
 * @file plugin_fork.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-10-21
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "brick_init.h"
#include "listen.h"
#include "main.h"

pluginModuleTypeDef *global_pluginModule = NULL;
static char global_moduleName[128] = {0};

/**
 * @brief  plugin_printfVer
 * @note
 * @param  *param:
 * @retval
 */
int plugin_printfVer(void)
{
    global_pluginModule = plugin_reg_init();
    assert(global_pluginModule != NULL);

    pluginRegLinkList *node = global_pluginModule->p_head;

    // log_printf("global_pluginModule->moduleNum: %d\n", global_pluginModule->moduleNum);
    for (size_t i = 0; (i < global_pluginModule->moduleNum) && (node != NULL); i++)
    {
        printf(">>> %s version: %s\n", node->moduleMsg->moduleName, node->moduleMsg->moduleVersion);
        node = node->next;
    }

    return 0;
}

/**
 * @brief  plugin_init
 * @note
 * @param  *param:
 * @retval
 */
int plugin_init(char *param)
{
    int ret;

    assert(param != NULL);
    log_printf("plugin_init >> param [%s]\n", param);

    global_pluginModule = plugin_reg_init();
    assert(global_pluginModule != NULL);

    strcpy(global_moduleName, param); // save the module name.

    // loopback mode.
    if (!strcmp(param, "loopback"))
    {
        global_pluginModule->mode = LOOPBACK;

        if ((ret = route_listen_init(global_pluginModule)) < 0)
        {
            printf("plugin_init: route_listen_init error [%d]\n", ret);
            ERROR_ASSERT();
            return -1;
        }
        // wait the loopback listener.
        ret = route_listen_wait();
        printf("plugin_init: route_listen_wait [%d]\n", ret);
    }
    else if (!strcmp(param, "relay"))
    {
        global_pluginModule->mode = RELAY;

        if ((ret = route_listen_init(global_pluginModule)) < 0)
        {
            printf("plugin_init: route_listen_init error [%d]\n", ret);
            ERROR_ASSERT();
            return -1;
        }
        // wait the loopback listener.
        ret = route_listen_wait();
        printf("plugin_init: route_listen_wait [%d]\n", ret);
    }

    // route mode.
    else
    {
        global_pluginModule->mode = ROUTER;
        pluginRegLinkList *node = global_pluginModule->p_head;
        if (node == NULL)
        {
            ERROR_ASSERT();
            return -1;
        }

        for (size_t i = 0; (i < global_pluginModule->moduleNum) && (node != NULL); i++)
        {
            if (!strcmp(node->moduleMsg->moduleName, param))
            {
                ret = node->moduleMsg->init();
                log_printf("init >> moduleName %s, ret: %d\n", node->moduleMsg->moduleName, ret);
                ret = node->moduleMsg->start();
                log_printf("start >> moduleName %s, ret: %d\n", node->moduleMsg->moduleName, ret);
            }

            node = node->next;
        }
    }

    return 0;
}

int plugin_destroy(void)
{
    int ret;

    pluginRegLinkList *node = global_pluginModule->p_head;
    if (node == NULL)
    {
        ERROR_ASSERT();
        return -1;
    }

    for (size_t i = 0; (i < global_pluginModule->moduleNum) && (node != NULL); i++)
    {
        if (!strcmp(node->moduleMsg->moduleName, global_moduleName))
        {
            ret = node->moduleMsg->destroy();
            log_printf("destroy >> moduleName %s, ret: %d\n", node->moduleMsg->moduleName, ret);
        }

        node = node->next;
    }

    return 0;
}