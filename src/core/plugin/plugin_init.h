/**
 * @file plugin_init.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-10-21
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __PLUGIN_INIT_H
#define __PLUGIN_INIT_H

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief  plugin_init
     * @note
     * @param  *param:
     * @retval
     */
    extern int plugin_init(char *param);
    extern int plugin_destroy(void);

#ifdef __cplusplus
}
#endif

#endif //__PLUGIN_INIT_H
