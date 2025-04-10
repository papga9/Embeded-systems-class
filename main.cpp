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
#include <stdio.h>

Timer timer;

Semaphore sem1(1);
Semaphore sem2(1);
Semaphore sem3(1);
Semaphore sem4(0);

PwmOut redLed   (LED_RED);
PwmOut greenLed (LED_GREEN);
PwmOut blueLed  (LED_BLUE);

TSISensor tsi;
Thread ThreadTSI(osPriorityNormal, 2048);

Thread Thread2(osPriorityNormal, 1024);
Thread Thread3(osPriorityNormal, 1024);
Thread Thread4(osPriorityNormal, 1024);

typedef struct {
    float currPercent;
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
        mail->currPercent = tsi.readPercentage();
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

        mail_t *mail = mail_box.try_get();
        if (mail != nullptr) {
            redLed.write(mail->currPercent);
            mail_box.free(mail);
        }

        ThisThread::sleep_for(100ms);
        if(timer.read() >= 10) {
            mail_t *mail = mail_box.try_alloc();
            if (mail != nullptr) {
                mail->redPercentage = mail->currPercent;
                mail_box.put(mail);
                sem2.release();
                timer.reset();
            }
        }
        else {
            printf("th1, %f\n", timer.read());
        }
    }
}

void thread2() {
    printf("Thread2 \n");
    float perc = 0.0;
    while(true) {
        printf("Thread2 \n");
        sem2.acquire();

        mail_t *mail = mail_box.try_get();
        if (mail != nullptr) {
            greenLed.write(mail->currPercent);
            mail_box.free(mail);
        }
        
        ThisThread::sleep_for(100ms);
        if(timer.read() >= 10) {
            mail_t *mail = mail_box.try_alloc();
            if (mail != nullptr) {
                mail->greenPercentage = mail->currPercent;
                mail_box.put(mail);
                sem3.release();
                timer.reset();
            }
        }
        else {
            printf("th2, %f\n", timer.read());
        }
    }
}

void thread3() {
    printf("Thread3 \n");
    float perc = 0.0;
    while(true) {
        printf("Thread3 \n");
        sem3.acquire();
         mail_t *mail = mail_box.try_get();
        if (mail != nullptr) {
            blueLed.write(mail->currPercent);
            mail_box.free(mail);
        }
        
        ThisThread::sleep_for(100ms);
        if(timer.read() >= 10) {
            mail_t *mail = mail_box.try_alloc();
            if (mail != nullptr) {
                mail->bluePercentage = mail->currPercent;
                mail_box.put(mail);
                sem4.release();
                timer.reset();
            }
        }
        else {
            printf("th3, %f\n", timer.read());
        }
    }
}

void thread4() {
    printf("wohoo");
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
            timer.reset();
        }
    }
}

int main()
{
    timer.start();
    redLed.period(30);
    greenLed.period(30);
    blueLed.period(30);

    redLed.write(0.5);
    greenLed.write(0.5);
    blueLed.write(0.5);

    ThreadTSI.start(GetTSI);
    thread1();
    Thread2.start(thread2);
    Thread3.start(thread3);
    Thread4.start(thread4);
}
