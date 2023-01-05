/**
 * @file plugin_fork.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-10-21
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __PROCESS_FORK_H
#define __PROCESS_FORK_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "plugin_reg.h"

    /**
     * @brief  plugin_fork
     * @note
     * @param  *p_plugin:
     * @retval
     */
    extern int plugin_fork(pluginModuleTypeDef *p_plugin);

    /**
     * @brief process_monitor
     * @note
     * @retval 0 or -1
     */
    extern int process_monitor(void);

#ifdef __cplusplus
}
#endif

#endif //__PROCESS_FORK_H
