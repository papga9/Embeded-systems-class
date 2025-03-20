#include "mbed.h"
#include "rtos.h"
#include <chrono>
#include <cstdlib>

Mutex stdio_mutex;
Mutex forks[5];

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

void philosopher(const int *num){
    int philNum = (int)num;

    while (true) {
        if(forks[philNum - 1].trylock()){
            if(forks[philNum % 5].trylock()){
                Notify(philNum, 1);
                ThisThread::sleep_for(
                    chrono::milliseconds(rand() % 1000 + 1000)
                );
                forks[philNum % 5].unlock();
                forks[philNum - 1].unlock();
                Notify(philNum, 0);
                ThisThread::sleep_for(
                    chrono::milliseconds(rand() % 2000 + 2000)
                );
            }
            else {
                forks[philNum - 1].unlock();
                ThisThread::sleep_for(
                    chrono::milliseconds(rand() % 100 + 100)
                );
            }
        }
        else {
            ThisThread::sleep_for(
                chrono::milliseconds(rand() % 100 + 100)
            );
        }
    }
}

int main()
{
    Thread2.start(callback(philosopher, (const int *)2));
    Thread3.start(callback(philosopher, (const int *)2));
    Thread4.start(callback(philosopher, (const int *)2));
    Thread5.start(callback(philosopher, (const int *)2));
    philosopher((const int *)1);
}


