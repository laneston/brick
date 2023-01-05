/**
 * @file moduleDeclare.h
 * @author lanceli (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-10-17
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __MODULEREG_H
#define __MODULEREG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "aInit.h"
#include "bInit.h"
#include "cInit.h"
#include "plugin.h"

    brickPluginRegister_regArray moduleRegister[] = {
        &testa,
        &testb,
        &testc,
    };

#ifdef __cplusplus
}
#endif

#endif //__MODULEREG_H
