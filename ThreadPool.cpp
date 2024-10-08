#include "ThreadPool.h"
#include <string.h>
#include <iostream>
#include <string>
#include <unistd.h>

template <typename T>
ThreadPool<T>::ThreadPool(int min, int max)
{
    m_taskQ = new TaskQueue<T>;

    m_minNum = min;
    m_maxNum = max;
    m_busyNum = 0;
    m_aliveNum = min;

    m_threadIDs = new pthread_t[max];
    memset(m_threadIDs, 0, sizeof(pthread_t) * max);
    
    if (
        pthread_mutex_init(&m_lock, nullptr) != 0 || 
        pthread_cond_init(&m_notEmpty, nullptr) != 0
    ) 
    {
        std::cout << "mutex or cond init fail..." << std::endl;
    }

    for (int i = 0; i < min; i++) {
        pthread_create(&m_threadIDs[i], nullptr, worker, this);
        std::cout << "创建子线程，ID = " << std::to_string(m_threadIDs[i]) << std::endl;
    }
    pthread_create(&m_managerID, nullptr, manager, this);
}

template <typename T>
ThreadPool<T>::~ThreadPool()
{
    m_shutdown = true;
    // 销毁管理者线程
    pthread_join(m_managerID, nullptr);
    // 销毁管理者线程
    for (int i = 0; i < m_aliveNum; i++) {
        pthread_cond_signal(&m_notEmpty);
    }
    if (m_taskQ) delete m_taskQ;
    if (m_threadIDs) delete[] m_threadIDs;
    pthread_mutex_destroy(&m_lock);
    pthread_cond_destroy(&m_notEmpty);
}

template <typename T>
void ThreadPool<T>::addTask(Task<T> task)
{
    if (m_shutdown) {
        return;
    }
    // 添加时不用枷锁，添加函数中有锁
    m_taskQ->addTask(task);
    // 唤醒工作线程
    pthread_cond_signal(&m_notEmpty);
}

template <typename T>
int ThreadPool<T>::getBusyNum()
{
    pthread_mutex_lock(&m_lock);
    int num = m_busyNum;
    pthread_mutex_unlock(&m_lock);
    return num;
}

template <typename T>
int ThreadPool<T>::getAliveNum()
{
    pthread_mutex_lock(&m_lock);
    int num = m_aliveNum;
    pthread_mutex_unlock(&m_lock);
    return 0;
}

template <typename T>
void ThreadPool<T>::ThreadExit()
{
    pthread_t tid = pthread_self();
    for (int i = 0; i < m_maxNum; i++) {
        if (m_threadIDs[i] == tid) {
            std::cout << "threadExit() function: thread " 
                << std::to_string(pthread_self()) << " exiting..." << std::endl;
            m_threadIDs[i] = 0;
            break;
        }
    }
    pthread_exit(&tid);
}

// 工作者线程，从任务队列中取出任务进行执行
template <typename T>
void *ThreadPool<T>::worker(void *arg)
{
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    while (true) {
        pthread_mutex_lock(&pool->m_lock);
        // 当任务队列为空时进行阻塞
        while (pool->m_taskQ->getNum() == 0 && !pool->m_shutdown) {
            std::cout << "thread " << std::to_string(pthread_self()) << " waiting..." << std::endl;
            pthread_cond_wait(&pool->m_notEmpty, &pool->m_lock);
            // 唤醒后
            if (pool->m_exitNum > 0) {
                pool->m_exitNum--;
                if (pool->m_aliveNum > pool->m_minNum) {
                    pool->m_aliveNum--;
                    pthread_mutex_unlock(&pool->m_lock);
                    pool->ThreadExit();
                }
            }
        }
        // 若线程池关闭则退出当前线程
        if (pool->m_shutdown) {
            pthread_mutex_unlock(&pool->m_lock);
            pool->ThreadExit();
        }
        // 任务队列不为空时
        Task task = pool->m_taskQ->getTask();
        pool->m_busyNum++;
        pthread_mutex_unlock(&pool->m_lock);

        // 执行任务
        pthread_mutex_lock(&lock);
        std::cout << "thread " << std::to_string(pthread_self()) << " start working" << std::endl;
        pthread_mutex_unlock(&lock);
        
        task.function(task.arg);
        delete task.arg;
        task.arg = nullptr;
        // 任务结束
        pthread_mutex_lock(&lock);
        std::cout << "thread " << std::to_string(pthread_self()) << " end working..." << std::endl;
        pthread_mutex_unlock(&lock);

        pthread_mutex_lock(&pool->m_lock);
        pool->m_busyNum--;
        pthread_mutex_unlock(&pool->m_lock);
    }
    return nullptr;
}

// 管理者线程，每隔一段时间读取线程池中的数据对线程进行生成和销毁
template <typename T>
void *ThreadPool<T>::manager(void *arg)
{
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    while (!pool->m_shutdown) {
        sleep(5);
        // 取出线程数和任务数
        pthread_mutex_lock(&pool->m_lock);
        int aliveNum = pool->m_aliveNum;
        int taskNum = pool->m_taskQ->getNum();
        int busyNum = pool->m_busyNum;
        pthread_mutex_unlock(&pool->m_lock);

        const int NUMBER = 2;
        // 若线程数不够
        if (aliveNum < taskNum && aliveNum < pool->m_maxNum) {
            pthread_mutex_lock(&pool->m_lock);
            int num = 0;
            // 每次增加 NUMBER 个，线程数不能大于最大线程数，线程 id 必须在范围内
            for (int i = 0; num < NUMBER && 
                            pool->m_aliveNum < pool->m_maxNum &&
                            i < pool->m_maxNum; i++)
            {
                // 对未创建的线程进行创建
                if (pool->m_threadIDs[i] == 0) {
                    pthread_create(&pool->m_threadIDs[i], nullptr, worker, pool);
                    num++;
                    pool->m_aliveNum++;
                }
            }
            pthread_mutex_unlock(&pool->m_lock);
        }

        // 若线程数过多，忙碌线程数 * 2 小于 存活线程数
        if (busyNum * 2 < aliveNum && aliveNum > pool->m_minNum) {
            pthread_mutex_lock(&pool->m_lock);
            // 需要销毁的线程数
            pool->m_exitNum = NUMBER;
            pthread_mutex_unlock(&pool->m_lock);
            // 唤醒等量的线程让其自动销毁
            for (int i = 0; i < NUMBER; i++) {
                pthread_cond_signal(&pool->m_notEmpty);
            }
        }
    }
    return nullptr;
}
