/**
 * @file plugin_reg.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-10-21
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "plugin_reg.h"
#include "cJSON.h"
#include "main.h"
#include "moduleReg.h"





/**
 * @brief  plugin_reg_add
 * @note
 * @param  *p_plugin:
 * @param  *module_t:
 * @retval
 */
static int plugin_reg_add(pluginModuleTypeDef *p_plugin, const brickPluginRegister_TypeDef *module_t)
{
    assert(((p_plugin != NULL) && (module_t != NULL)));

    pthread_mutex_lock(&p_plugin->mutex);

    pluginRegLinkList *pdata = (pluginRegLinkList *)malloc(sizeof(pluginRegLinkList));
    assert(pdata != NULL);

    // 此处应加函数判断
    pdata->moduleMsg = module_t;
    pdata->status = MODULE_STOP;
    pdata->next = NULL;

    if (p_plugin->p_head == NULL) // if the first pack
    {
        p_plugin->p_head = p_plugin->p_tail = pdata;
    }
    else
    {
        pdata->next = p_plugin->p_tail->next;
        p_plugin->p_tail->next = pdata;
        p_plugin->p_tail = pdata;
    }

    p_plugin->moduleNum++; // 插件数目计数
    log_printf("plugin_reg_add: [moduleNum %d]\n", p_plugin->moduleNum);

    pthread_mutex_unlock(&(p_plugin->mutex));

    return 0;
}

/**
 * @brief  plugin_reg_getModule
 * @note
 * @param  *p_plugin:
 * @param  *module_i:
 * @retval
 */
static int plugin_reg_getModule(pluginModuleTypeDef *p_plugin, brickPluginRegister_regArray *module_i)
{
    assert(((p_plugin != NULL) && (module_i != NULL)));

    brickPluginRegister_regArray *node = module_i;
    uint8_t iModule = sizeof(moduleRegister) / sizeof(moduleRegister[0]);
    log_printf("plugin_reg_getModule >> iModule: %d\n", iModule);

    for (size_t i = 0; i < iModule; i++)
    {
        if (plugin_reg_add(p_plugin, *node) < 0)
        {
            ERROR_ASSERT();
            return -1;
        }
        else
        {
            node++;
        }
    }

    if (p_plugin->moduleNum != iModule)
    {
        log_printf("plugin_reg_getModule >> (p_plugin->moduleNum != iModule)\n");
        return -1;
    }

    return 0;
}

/**
 * @brief  moduleEnable_json_get
 * @note
 * @param  *p_plugin:
 * @retval
 */
static int moduleEnable_json_get(pluginModuleTypeDef *p_plugin)
{

    assert(p_plugin != NULL);

    static char configString[FILE_PLUGIN_LEN] = {0};

    // 获取JSON文件
    int FILE_p = open(FILE_PLUGIN_DIR, O_RDONLY);
    int read_len = read(FILE_p, configString, FILE_PLUGIN_LEN);
    if (read_len > 0)
    {
        log_printf("moduleEnable_json_get: [read_len %d]\n%s\n", read_len, configString);
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
        log_printf("configFileGet: cJSON_Parse error!\n");
        return -1;
    }
    else
    {
        if (root->type != cJSON_Object)
        {
            log_printf("configFileGet: root->type error!\n");
            return -1;
        }
    }

    // 解析JSON文件
    pluginRegLinkList *node = p_plugin->p_head;

    cJSON *item = NULL;
    cJSON *status = NULL;

    for (size_t i = 0; (i < p_plugin->moduleNum) && (node != NULL); i++)
    {
        // 获取配置文件键值
        item = cJSON_GetObjectItem(root, node->moduleMsg->moduleName);
        if (item == NULL)
        {
            log_printf("configFileGet [ERROR] item = cJSON_GetObjectItem(root, node->moduleMsg->moduleName);\n");
            node = node->next; // 获取失败则对比下一节点
            continue;
        }
        else
        {
            if (item->type != cJSON_Object)
            {
                log_printf("configFileGet [ERROR] if (item->type != cJSON_Object)\n");
                node = node->next;
                continue;
            }
        }

        status = cJSON_GetObjectItem(item, "status");
        if (status == NULL)
        {
            log_printf("configFileGet [ERROR] status = cJSON_GetObjectItem(item, \" status \");\n");
            return -1;
        }
        else
        {
            if (status->type != cJSON_String)
            {
                log_printf("configFileGet [ERROR] if (status->type != cJSON_String)\n");
                return -1;
            }
        }
        // 获取成功则确定是否需要使能
        if (strcmp(status->valuestring, "enable"))
        {
            node->status = MODULE_STOP;
        }
        else
        {
            node->status = MODULE_START;
        }

        node = node->next;
    }

    return 0;
}

/**
 * @brief  plugin_reg_init
 * @note
 * @retval p_plugin
 */
pluginModuleTypeDef *plugin_reg_init(void)
{
    pluginModuleTypeDef *p_plugin = (pluginModuleTypeDef *)malloc(sizeof(pluginModuleTypeDef));
    assert(p_plugin != NULL);

    p_plugin->p_head = NULL;
    p_plugin->p_tail = NULL;

    p_plugin->moduleNum = 0;

    int ret = pthread_mutex_init(&(p_plugin->mutex), NULL);
    assert(ret == 0);

    if (plugin_reg_getModule(p_plugin, moduleRegister) < 0)
    {
        ERROR_ASSERT();
        free(p_plugin);
        return NULL;
    }

    if (moduleEnable_json_get(p_plugin) < 0)
    {
        ERROR_ASSERT();
        return NULL;
    }

    return p_plugin;
}

/**
 * @brief  plugin_reg_destroy
 * @note
 * @param  *p_plugin:
 * @retval
 */
int plugin_reg_destroy(pluginModuleTypeDef *p_plugin)
{
    assert(p_plugin != NULL);
    if (pthread_mutex_destroy(&(p_plugin->mutex)) < 0)
    {
        ERROR_ASSERT();
        return -1;
    }

    return 0;
}
