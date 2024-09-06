#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_
#include <pthread.h>
#include "TaskQueue.h"

class ThreadPool {
public:
    ThreadPool(int min, int max);
    ~ThreadPool();

private:
    static void* worker(void* arg);
    static void* manager(void* arg);

private:
    // 互斥锁
    pthread_mutex_t m_lock;
    // 任务队列为空的条件变量
    pthread_cond_t m_notEmpty;

    // 消费者线程数组
    pthread_t* m_threadIDs;
    // 管理者线程数组
    pthread_t m_managerID;

    // 任务队列
    TaskQueue* m_taskQ;

    // 线程数上下限
    int m_maxNum;
    int m_minNum;

    // 现有的线程数
    int m_aliveNum;
    // 在忙的线程数
    int m_busyNum;
    // 需要销毁的线程数
    int m_exitNum;

    // 是否销毁线程池
    bool m_shutdown = false;
};

#endif