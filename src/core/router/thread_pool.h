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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __THREAD_POOL_H
#define __THREAD_POOL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <pthread.h>
#include <stdbool.h>

    // thread function struct.
    typedef struct taskFunc_listList
    {
        void *(*callback_func)(void *arg); // function
        void *arg;                         // params
        struct taskFunc_listList *next;    // the pointer

    } taskFunc_listList;

    typedef struct threadPool
    {
        unsigned short thread_nums;          // thread total num
        unsigned short queue_nums;           // the queue nums
        taskFunc_listList *p_head;           // head pointer
        taskFunc_listList *p_tail;           // taail pointer
        pthread_t *pthreads;                 //
        pthread_mutex_t mutex;               //
                                             //
        pthread_cond_t cond_queue_empty;     //队列以空信号
        pthread_cond_t cond_queue_nonempty;  //队列非空信号
        pthread_cond_t cond_queue_underfull; //队列非满信号
                                             //
        bool flag_queue_close;               //
        bool flag_pool_close;                //
        bool flag_pool_running;              //

    } threadPool_TypeDef;

    /**
     * @brief  threadPool_init
     * @note   thread pool init.
     * @param  thread_nums: the thread num.
     * @retval
     */
    extern threadPool_TypeDef *threadPool_init(unsigned short thread_nums);

    /**
     * @brief  threadPool_destroy
     * @note
     * @param  *pool:
     * @retval
     */
    extern int threadPool_destroy(threadPool_TypeDef *pool);

    /**
     * @brief  threadPool_wait
     * @note
     * @param  *pool:
     * @retval
     */
    extern int threadPool_wait(threadPool_TypeDef *pool);

    /**
     * @brief  threadpool_add_task
     * @note
     * @param  *pool:
     * @param  *(*callback_func:
     * @retval
     */
    extern int threadpool_add_task(threadPool_TypeDef *pool, void *(*callback_func)(void *arg), void *arg);

#ifdef __cplusplus
}
#endif

#endif /* __THREAD_POOL_H */
