/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "TSISensor.h"
#include "rtos.h"
#include <cstdio>
#include <exception>
#include <stdint.h>

Timer t;

Semaphore sem1(1);
Semaphore sem2(1);
Semaphore sem3(1);
Semaphore sem4(0);

PwmOut redLed   (LED_RED);
PwmOut greenLed (LED_GREEN);
PwmOut blueLed  (LED_BLUE);

TSISensor tsi;
Thread ThreadTSI(osPriorityNormal, 2028);

Thread Thread2(osPriorityNormal, 1024);
Thread Thread3(osPriorityNormal, 1024);
Thread Thread4(osPriorityNormal, 1024);

typedef struct {
    float redPercentage;
    float greenPercentage;
    float bluePercentage;
} mail_t;

Mail<mail_t, 1> mail_box;

void GetTSI() {
    float perc = 0;

    while (true) {
        perc = tsi.readPercentage();

        mail_t *mail = mail_box.try_alloc();
        if (mail != nullptr) {
            //mail->touched = perc != 0.0 ? true : false;
        mail_box.put(mail);
    }
    ThisThread::sleep_for(100ms);
    }
}




void thread1() {
    float perc = 0.0;
    while(true) {
        printf("Thread1 \n");
        sem1.acquire();
        perc = tsi.readPercentage();
        redLed.write(perc);
        ThisThread::sleep_for(5s);

        mail_t *mail = mail_box.try_alloc();
        if (mail != nullptr) {
            mail->redPercentage = perc;
            mail_box.put(mail);
        }
        ThisThread::sleep_for(100ms);

        redLed.write(1);
        sem2.release();
    }
}

void thread2() {
    float perc = 0.0;
    while(true) {
        printf("Thread2 \n");
        sem2.acquire();
        perc = tsi.readPercentage();
        greenLed.write(perc);
        ThisThread::sleep_for(5s);

        mail_t *mail = mail_box.try_alloc();
        if (mail != nullptr) {
            mail->greenPercentage = perc;
            mail_box.put(mail);
        }
        ThisThread::sleep_for(100ms);

        greenLed.write(1);
        sem3.release();
    }
}

void thread3() {
    float perc = 0.0;
    while(true) {
        printf("Thread3 \n");
        sem3.acquire();
        perc = tsi.readPercentage();
        blueLed.write(perc);
        ThisThread::sleep_for(5s);

        mail_t *mail = mail_box.try_alloc();
        if (mail != nullptr) {
            mail->bluePercentage = perc;
            mail_box.put(mail);
        }
        ThisThread::sleep_for(100ms);

        blueLed.write(1);
        sem4.release();
    }
}

void thread4() {
    float perc = 0.0;
    while(true) {
        printf("Thread4 \n");
        sem4.acquire();
        perc = tsi.readPercentage();

        mail_t *mail = mail_box.try_get();
        if (mail != nullptr) {
            redLed.write(mail->redPercentage);
            greenLed.write(mail->greenPercentage);
            blueLed.write(mail->bluePercentage);
        }
        ThisThread::sleep_for(100ms);

        if (perc != 0.0) {
            sem1.release();
        }
    }
}

int main()
{
    redLed.period(30);
    greenLed.period(30);
    blueLed.period(30);

    redLed.write(1);
    greenLed.write(1);
    blueLed.write(1);

    thread1();
    Thread2.start(thread2);
    Thread3.start(thread3);
    Thread4.start(thread4);
}
