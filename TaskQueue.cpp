#include "TaskQueue.h"

template <typename T>
TaskQueue<T>::TaskQueue()
{
    pthread_mutex_init(&m_mutex, nullptr);
}

template <typename T>
TaskQueue<T>::~TaskQueue()
{
    pthread_mutex_destroy(&m_mutex);
}

template <typename T>
Task<T> TaskQueue<T>::getTask()
{
    Task<T> t;
    pthread_mutex_lock(&m_mutex);
    t = m_queue.front();
    m_queue.pop();
    pthread_mutex_unlock(&m_mutex);
    return t;
}

template <typename T>
void TaskQueue<T>::addTask(callback f, void *a)
{
    Task t(f, a);
    this->addTask(t);
}

template <typename T>
void TaskQueue<T>::addTask(Task<T> t)
{
    pthread_mutex_lock(&m_mutex);
    m_queue.push(t);
    pthread_mutex_unlock(&m_mutex);
}
