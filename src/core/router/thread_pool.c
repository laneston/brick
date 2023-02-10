/**
 * @file thread_pool.h
 * @author lanceli (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-10-21
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "thread_pool.h"
#include "main.h"
#include "plugin.h"

// 任务加入队列 -> 解锁 -> 线程池收到非空广播信息 -> 上锁 -> 执行队列任务 -> 解锁
// 问题：线程池收到非空广播后，所有任务线程都执行，如队列无任务会强制执行空指针，会引发潜在BUG
// 解决：任务线程收到非空广播后需额外判断任务队列中是否有任务存在

/**
 * @brief  threadPool_task
 * @note
 * @param  *arg:
 * @retval None
 */
static void *threadPool_task(void *arg)
{

    assert(arg != NULL);
    threadPool_TypeDef *pool = (threadPool_TypeDef *)arg;

    taskFunc_listList *p_task = NULL;

    while (pool->flag_pool_running)
    {
        // lock the thread pool.
        pthread_mutex_lock(&(pool->mutex));

        // if the thread pool close, exit.
        if (pool->flag_pool_close)
        {
            log_printf("threadPool_task >> flag_pool_close\n");
            pthread_mutex_unlock(&(pool->mutex));
            return NULL;
        }

        // check the state of the pool.
        while ((pool->queue_nums == 0) && (pool->flag_queue_close == false))
        {
            // log_printf("threadPool_task >> queue_nums abnormal[1]\n");
            pthread_cond_wait(&(pool->cond_queue_nonempty), &(pool->mutex)); // wait the nonempty message.

            // unlock the thread pool.
            pthread_mutex_unlock(&(pool->mutex));
        }

        // lock the thread pool.
        pthread_mutex_lock(&(pool->mutex));

        p_task = pool->p_head;
        pool->p_head = p_task->next;

        if (p_task == NULL)
        {
            log_printf("threadPool_task >> [p_task == NULL] queue_nums: %d\n", pool->queue_nums);
            continue;
        }

        if (pool->queue_nums > QUEUE_MAX_NUM - 1)
            pthread_cond_broadcast(&(pool->cond_queue_underfull)); // send underfull message

        pool->queue_nums--;

        // log_printf("threadPool_task [pool->queue_nums: %d]\n", pool->queue_nums);
        // unlock the thread pool.
        pthread_mutex_unlock(&(pool->mutex));

        (*(p_task->callback_func))(p_task->arg); // perform a mission

        free(p_task);
        p_task = NULL;
    }

    return NULL;
}

/**
 * @brief  threadpool_add_task
 * @note
 * @param  *pool:
 * @param  *(*callback_func:
 * @retval
 */
int threadpool_add_task(threadPool_TypeDef *pool, void *(*callback_func)(void *arg), void *arg)
{

    assert(pool != NULL && callback_func != NULL);

    // lock the thread pool.
    pthread_mutex_lock(&(pool->mutex));

    if (pool->flag_queue_close || pool->flag_pool_close)
    {
        ERROR_ASSERT();
        pthread_mutex_unlock(&(pool->mutex));
        return -1;
    }

    if ((pool->queue_nums > QUEUE_MAX_NUM - 1) && !(pool->flag_queue_close || pool->flag_pool_close))
    {
        log_printf("threadpool_add_task >> queue_nums abnormal[0]!\n");
        pthread_cond_wait(&(pool->cond_queue_underfull), &(pool->mutex));
        pthread_mutex_unlock(&(pool->mutex));
    }

    pthread_mutex_lock(&(pool->mutex));

    /**build the cache for tasks*/
    taskFunc_listList *p_task = (taskFunc_listList *)malloc(sizeof(taskFunc_listList));
    assert(p_task != NULL);

    /*take the funcuion and param to the pionter*/
    p_task->callback_func = callback_func;
    p_task->arg = arg;
    p_task->next = NULL;

    /*if the first Linked list is NULL*/
    if (pool->p_head == NULL)
    {
        pool->p_head = pool->p_tail = p_task;
        pthread_cond_broadcast(&(pool->cond_queue_nonempty)); // send the nonempty message.
    }
    /*if not the first Linked list node*/
    else
    {
        p_task->next = pool->p_tail->next;
        pool->p_tail->next = p_task;
        pool->p_tail = p_task;
    }

    pool->queue_nums++;

    pthread_mutex_unlock(&(pool->mutex));

    return 0;
}

/**
 * @brief  threadPool_init
 * @note   thread pool init.
 * @param  thread_nums: the thread num.
 * @retval
 */
threadPool_TypeDef *threadPool_init(unsigned short thread_nums)
{
    assert(thread_nums != 0);

    int ret;

    threadPool_TypeDef *pool = (threadPool_TypeDef *)malloc(sizeof(threadPool_TypeDef));
    assert(pool != NULL);

    pool->thread_nums = thread_nums;
    pool->queue_nums = 0;
    pool->flag_queue_close = false;
    pool->flag_pool_close = false;
    pool->flag_pool_running = true;
    pool->p_head = NULL;
    pool->p_tail = NULL;

    ret = pthread_mutex_init(&(pool->mutex), NULL);
    assert(ret == 0);
    ret = pthread_cond_init(&(pool->cond_queue_empty), NULL);
    assert(ret == 0);
    ret = pthread_cond_init(&(pool->cond_queue_nonempty), NULL);
    assert(ret == 0);
    ret = pthread_cond_init(&(pool->cond_queue_underfull), NULL);
    assert(ret == 0);

    pool->pthreads = (pthread_t *)malloc((sizeof(pthread_t) * thread_nums));
    assert(NULL != pool->pthreads);

    for (unsigned short i = 0; i < pool->thread_nums; i++)
    {
        ret = pthread_create(&(pool->pthreads[i]), NULL, threadPool_task, (void *)pool);
        assert(ret == 0);
        log_printf("threadPool_init: [pool->pthreads: %ld]\n", pool->pthreads[i]);
    }

    return pool;
}

/**
 * @brief  threadPool_wait
 * @note
 * @param  *pool:
 * @retval
 */
int threadPool_wait(threadPool_TypeDef *pool)
{
    assert(pool != NULL);

    int ret;

    for (unsigned short i = 0; i < pool->thread_nums; i++)
    {
        ret = pthread_join(pool->pthreads[i], NULL);
        log_printf("threadPool_wait: [pool->pthreads: %ld]\n", pool->pthreads[i]);
    }

    return ret;
}

/**
 * @brief  threadPool_destroy
 * @note
 * @param  *pool:
 * @retval
 */
int threadPool_destroy(threadPool_TypeDef *pool)
{

    assert(pool != NULL);
    taskFunc_listList *p_task = NULL;
    int ret;

    /**队列与线程池都在工作状态*/
    assert(pool->flag_queue_close || pool->flag_pool_close);

    /**队列关闭标志，禁止继续添加任务*/
    pool->flag_queue_close = true;

    /*等待队列任务处理完成*/
    if (pool->queue_nums != 0)
        pthread_cond_wait(&(pool->cond_queue_empty), &(pool->mutex));

    /*置线程池关闭标志*/
    pool->flag_pool_close = true;

    // 等待线程执行完毕
    for (unsigned short i = 0; i < pool->thread_nums; i++)
    {
        ret = pthread_join(pool->pthreads[i], NULL);
        assert(ret == 0);
        log_printf("threadPool_destroy: [pool->pthreads: %ld]\n", pool->pthreads[i]);
    }

    pool->flag_pool_running = false;

    ret = pthread_mutex_destroy(&(pool->mutex));
    assert(ret == 0);
    ret = pthread_cond_destroy(&(pool->cond_queue_empty));
    assert(ret == 0);
    ret = pthread_cond_destroy(&(pool->cond_queue_nonempty));
    assert(ret == 0);
    ret = pthread_cond_destroy(&(pool->cond_queue_underfull));
    assert(ret == 0);

    /* Confirm that the thread pool task chain p_header node is not empty*/
    while (pool->p_head != NULL)
    {
        p_task = pool->p_head;
        pool->p_head = p_task->next;

        free(p_task);
    }

    free(pool->pthreads);
    free(pool);

    return 0;
}