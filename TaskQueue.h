// 任务队列
#ifndef _TASKQUEUE_H_
#define _TASKQUEUE_H_
#include <queue>
#include <pthread.h>

using callback = void(*)(void*);
struct Task {
    Task() {
        function = nullptr;
        arg = nullptr;
    }
    Task(callback f, void* a) {
        function = f;
        arg = a;
    }

    callback function;
    void* arg;
};


class TaskQueue {
public:
    TaskQueue();
    ~TaskQueue();


    // 获取任务
    Task getTask();
    // 添加任务
    void addTask(callback f, void* a);
    void addTask(Task t);
    // 获取任务个数
    inline int getNum() {
        return m_queue.size();
    }
private:
    // 队列
    std::queue<Task> m_queue;
    // 互斥锁
    pthread_mutex_t m_mutex;
};


#endif _TASKQUEUE_H_