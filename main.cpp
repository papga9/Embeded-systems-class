#include "mbed.h"
#include "rtos.h"
#include <chrono>
#include <cstdlib>

Mutex stdio_mutex;
Semaphore forks(5);

Thread Thread2(osPriorityNormal, 1024);
Thread Thread3(osPriorityNormal, 1024);
Thread Thread4(osPriorityNormal, 1024);
Thread Thread5(osPriorityNormal, 1024);

void Notify(int num, int state){
    stdio_mutex.lock();
    if(state){
        printf("Philosopher %d is eating \n\r", num);
    } else{
        printf("Philosopher %d is thinking \n\r", num);
    }
    stdio_mutex.unlock();
}

void philosopher(void *args){
    int philNum = (int)args;

    while (true) {
        Notify(philNum, 1);
        ThisThread::sleep_for(
            chrono::milliseconds(rand() % 1000 + 1000)
        );
        forks.acquire();
        forks.acquire();
        Notify(philNum, 0);
        ThisThread::sleep_for(
            chrono::milliseconds(rand() % 1000 + 500)
        );
        forks.release();
        forks.release();
    }
}

int main()
{
    Thread2.start(callback(philosopher, (void *)2));
    Thread3.start(callback(philosopher, (void *)3));
    Thread4.start(callback(philosopher, (void *)4));
    Thread5.start(callback(philosopher, (void *)5));
    philosopher((void *)1);
}


