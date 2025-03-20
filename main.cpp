#include "mbed.h"
#include "rtos.h"
#include <chrono>
#include <ratio>

Thread t2(osPriorityNormal, 1024);
Thread t3(osPriorityNormal, 1024);

Semaphore sem1(1);
Semaphore sem2(1);
Semaphore sem3(0);

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);

void thread1() {
    while(true) {
        sem1.acquire();
        led1 = 0;
        ThisThread::sleep_for(
            chrono::milliseconds(rand() % 2000)
        );
        led1 = 1;
        sem3.release();
    }
}

void thread2() {
    while(true) {
        sem2.acquire();
        led2 = 0;
        ThisThread::sleep_for(
            chrono::milliseconds(rand() % 2000)
        );
        led2 = 1;
        sem3.release();
    }
}

void thread3() {
    while(true) {
        sem3.acquire();
        sem3.acquire();
        led3 = 1;
        for(int i=0; i<9; i++){
            ThisThread::sleep_for(chrono::milliseconds(1000));
            led3 == 1 ? led3 = 0 : led3 = 1;
        }
        
        led3 = 0;
        sem1.release();
        sem2.release();
    }
}

int main()
{
    led1 = 1;
    led2 = 1;
    led3 = 1;
    t2.start(thread2);
    t3.start(thread3);
    thread1();
}


