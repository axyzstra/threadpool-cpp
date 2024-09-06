#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "ThreadPool.h"
#include "ThreadPool.cpp"

pthread_mutex_t lock;

void taskFunc(void* arg) {
    int* num = static_cast<int*>(arg);
    pthread_mutex_lock(&lock);
    std::cout << "thread " << pthread_self() << " is working, number = " << *num << std::endl;
    pthread_mutex_unlock(&lock);
    sleep(1);
}


int main() {

    pthread_mutex_init(&lock, nullptr);

    ThreadPool<int> pool(3, 10);
    for (int i = 0; i < 100; i++) {
        int* num = new int(i + 100);
        pool.addTask(Task<int>(taskFunc, num));
    }
    sleep(20);

    pthread_mutex_destroy(&lock);
    return 0;
}