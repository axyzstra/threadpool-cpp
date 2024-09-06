#include "TaskQueue.h"

TaskQueue::TaskQueue()
{
    pthread_mutex_init(&m_mutex, nullptr);
}

TaskQueue::~TaskQueue()
{
    pthread_mutex_destroy(&m_mutex);
}

Task TaskQueue::getTask()
{
    Task t;
    pthread_mutex_lock(&m_mutex);
    t = m_queue.front();
    m_queue.pop();
    pthread_mutex_unlock(&m_mutex);
    return t;
}

void TaskQueue::addTask(callback f, void *a)
{
    Task t(f, a);
    this->addTask(t);
}

void TaskQueue::addTask(Task t)
{
    pthread_mutex_lock(&m_mutex);
    m_queue.push(t);
    pthread_mutex_unlock(&m_mutex);
}
