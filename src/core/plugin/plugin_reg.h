/**
 * @file plugin_reg.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-10-21
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __PLUGIN_REG_H
#define __PLUGIN_REG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "plugin.h"
#include <pthread.h>

    typedef enum
    {
        LOOPBACK,
        RELAY,
        ROUTER,

    } E_ModeStatus;

    typedef enum
    {
        MODULE_START,
        MODULE_RUNNING,
        MODULE_STOP

    } E_ModuleStatus;

    /**
     * @brief  pluginRegLinkList
     * @note 存储插件信息与状态
     */
    typedef struct pluginRegLinkList
    {
        const brickPluginRegister_TypeDef *moduleMsg;
        E_ModuleStatus status;
        pid_t tpid;

        struct pluginRegLinkList *next;

    } pluginRegLinkList;

    typedef struct pluginModule
    {
        pluginRegLinkList *p_head;
        pluginRegLinkList *p_tail;

        short moduleNum;
        E_ModeStatus mode;

        pthread_mutex_t mutex;

    } pluginModuleTypeDef;

    /**
     * @brief  plugin_reg_init
     * @note
     * @retval p_plugin
     */
    extern pluginModuleTypeDef *plugin_reg_init(void);

    /**
     * @brief  plugin_reg_destroy
     * @note
     * @param  *p_plugin:
     * @retval
     */
    extern int plugin_reg_destroy(pluginModuleTypeDef *p_plugin);

#ifdef __cplusplus
}
#endif

#endif //__PLUGIN_REG_H
