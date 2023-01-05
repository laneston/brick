/**
 * @file brick_init.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-10-21
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "listen.h"
#include "main.h"
#include "plugin_fork.h"
#include "plugin_reg.h"


pluginModuleTypeDef *handle_pluginModule;


/**
 * @brief  brick_init
 * @note
 * @retval 0 is successful or -1 if failed.
 */
int brick_init(void)
{
    // 获取注册信息(配置文件+结构体注册信息)
    handle_pluginModule = plugin_reg_init();
    assert(handle_pluginModule != NULL);

    handle_pluginModule->mode = ROUTER; // 默认为路由模式

    // 根据注册信息生成路由
    if (route_listen_init(handle_pluginModule) < 0)
    {
        ERROR_ASSERT();
        return -1;
    }

    // fork所需子进程
    if (plugin_fork(handle_pluginModule) < 0)
    {
        ERROR_ASSERT();
        return -1;
    }

    return 0;
}

/**
 * @brief  brick_destroy
 * @note
 * @retval int
 */
int brick_destroy(void)
{
    int ret = plugin_reg_destroy(handle_pluginModule);
    assert(ret == 0);

    return 0;
}
