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

#include "main.h"
#include "plugin_reg.h"

/**
 * @brief  monitor
 * @note
 * @param  *arg:
 * @retval None
 */
static void *fork_monitor(void *arg)
{
    pid_t tpid;
    int retWait;

    while (true)
    {
        // recycle the child process
        tpid = waitpid(0, &retWait, WNOHANG);
        if (tpid > 0)
        {
            log_printf("fork_monitor >> [%ld] exit code: %d\n", tpid, retWait);
        }

        usleep(50000);
    }

    return NULL;
}

/**
 * @brief  plugin_fork
 * @note
 * @param  *p_plugin:
 * @retval
 */
int plugin_fork(pluginModuleTypeDef *p_plugin)
{
    assert(p_plugin != NULL);
    pthread_t tid_monitor;

    char target_dir[1024] = {0};

    pluginRegLinkList *node = p_plugin->p_head;
    if (node == NULL)
    {
        ERROR_ASSERT();
        return -1;
    }

    // 获取执行文件当前路径
    if (getcwd(target_dir, sizeof(target_dir)) < 0)
    {
        ERROR_ASSERT();
    }

    strcat(target_dir, __TARGET_NAME__);
    printf("plugin_fork >> target_dir: %s\n", target_dir);

    for (size_t i = 0; i < p_plugin->moduleNum; i++)
    {
        if (node->status == MODULE_START)
        {
            pid_t tpid = fork();
            if (tpid < 0)
            {
                ERROR_ASSERT();
            }
            else if (tpid == 0)
            {
                // int rev = execl(__TARGET_DIR__, __TARGET_NAME__, "-s", node->moduleMsg->moduleName, NULL);
                int rev = execl(target_dir, __TARGET_NAME__, "-s", node->moduleMsg->moduleName, NULL);

                if (rev != 0)
                {
                    log_printf("plugin_fork: execv error code: [%d]\n", rev);
                    _exit(-1);
                }
                return 0;
            }
            else
            {
                node->tpid = tpid; // save the process id
                log_printf("plugin_fork: %s node->tpid : %ld\n", node->moduleMsg->moduleName, node->tpid);
            }
        }
        else
        {
            log_printf("plugin_fork: %s MODULE_STOP!\n", node->moduleMsg->moduleName);
        }

        node = node->next;
    }

    if (pthread_create(&tid_monitor, NULL, fork_monitor, NULL) < 0)
    {
        ERROR_ASSERT();
        return -1;
    }

    return 0;
}
