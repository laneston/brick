/**
 * @file brick_init.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-10-21
 *
 * @copyright Copyright (c) 2022
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BRICK_INIT_H
#define __BRICK_INIT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "plugin_reg.h"


    extern pluginModuleTypeDef *handle_pluginModule;

    /**
     * @brief  brick_init
     * @note
     * @retval 0 is successful or -1 if failed.
     */
    extern int brick_init(void);

    /**
     * @brief  brick_destroy
     * @note
     * @retval int
     */
    extern int brick_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* __BRICK_INIT_H */
