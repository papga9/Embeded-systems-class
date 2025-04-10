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

Semaphore sem1(0);
Semaphore sem2(0);
Semaphore sem3(0);
Semaphore sem4(1);

PwmOut redLed   (LED_RED);
PwmOut greenLed (LED_GREEN);
PwmOut blueLed  (LED_BLUE);

TSISensor tsi;
Thread ThreadTSI(osPriorityNormal, 2048);

Thread Thread2(osPriorityNormal, 1024);
Thread Thread3(osPriorityNormal, 1024);
Thread Thread4(osPriorityNormal, 1024);

typedef struct {
    float redPercentage = 0;
    float greenPercentage = 0;
    float bluePercentage = 0;
} mail_t;

Mail<mail_t, 1> mail_box;

void GetTSI() {
    float perc = 0;

    while (true) {
        perc = tsi.readPercentage();

        mail_t *mail = mail_box.try_alloc();
        if (mail != nullptr) {
        mail_box.put(mail);
    }
    ThisThread::sleep_for(100ms);
    }
}




void thread1() {
    float perc = 0.0;
    float green;
    float blue;

    while(true) {
        printf("Thread1 \n");
        sem1.acquire();
        
        redLed.write(1);
        greenLed.write(1);
        blueLed.write(1);

        mail_t *mail = mail_box.try_get();
        green = mail->greenPercentage;
        blue = mail->bluePercentage;
        mail_box.free(mail);

        while(timer.read() < 10){
            
            if (tsi.readPercentage() != 0.0) perc = tsi.readPercentage();
            redLed.write(perc);
            ThisThread::sleep_for(100ms);
        }
        mail_t *newMail = mail_box.try_alloc();
        if (newMail != nullptr) {
            newMail->redPercentage = perc;
            newMail->greenPercentage = green;
            newMail->bluePercentage = blue;
            mail_box.put(newMail);
            sem2.release();
            printf("extra melanin");
            timer.reset();
        }
    }
}

void thread2() {
    printf("Thread2 \n");
    float perc = 0.0;
    float red;
    float blue;

    while(true) {
        printf("Thread2 \n");
        sem2.acquire();

        redLed.write(1);
        greenLed.write(1);
        blueLed.write(1);

        mail_t *mail = mail_box.try_get();
        red = mail->redPercentage;
        blue = mail->bluePercentage;
        mail_box.free(mail);

        while(timer.read() < 10){
            
            if (tsi.readPercentage() != 0.0) perc = tsi.readPercentage();
            greenLed.write(perc);
            ThisThread::sleep_for(100ms);
        }
        mail_t *newMail = mail_box.try_alloc();
        if (newMail != nullptr) {
            newMail->redPercentage = red;
            newMail->greenPercentage = perc;
            newMail->bluePercentage = blue;
            mail_box.put(newMail);
            sem3.release();
            printf("extra melanin");
            timer.reset();
        }
    }
}

void thread3() {
    printf("Thread3 \n");
    float perc = 0.0;
    float red;
    float green;
    while(true) {
        printf("Thread3 \n");
        sem3.acquire();

        redLed.write(1);
        greenLed.write(1);
        blueLed.write(1);

        mail_t *mail = mail_box.try_get();
        green = mail->greenPercentage;
        red = mail->redPercentage;
        mail_box.free(mail);

        while(timer.read() < 10){
            
            if (tsi.readPercentage() != 0.0) perc = tsi.readPercentage();
            blueLed.write(perc);
            ThisThread::sleep_for(100ms);
        }
        mail_t *newMail = mail_box.try_alloc();
        if (newMail != nullptr) {
            newMail->redPercentage = red;
            newMail->greenPercentage = green;
            newMail->bluePercentage = perc;
            mail_box.put(newMail);
            sem4.release();
            printf("extra melanin");
            timer.reset();
        }
    }
}

void thread4() {
    printf("wohoo");
    float perc = 0.0;
    float red = 0.0;
    float green = 0.0;
    float blue = 0.0;
    while(true) {
        printf("Thread4 \n");
        sem4.acquire();
        perc = 0.0;

        mail_t *mail = mail_box.try_get();
        if (mail != nullptr) {
            red = mail->redPercentage;
            redLed.write(mail->redPercentage);
            green = mail->greenPercentage;
            greenLed.write(mail->greenPercentage);
            blue = mail->bluePercentage;
            blueLed.write(mail->bluePercentage);
            mail_box.free(mail);
        }
        ThisThread::sleep_for(100ms);
        printf("read: %f\n", tsi.readPercentage());

        timer.reset();
        bool reverse = false;

        while (perc <= 0.1) {
            float time = timer.read();
            if (timer.read() <= 1) {
                if (reverse)
                {
                   redLed.write(1 - red * time);
                greenLed.write(1 - green * time);
                blueLed.write(1 - blue * time); 
                }
                redLed.write(red * time);
                greenLed.write(green * time);
                blueLed.write(blue * time);
            }
            else {
            timer.reset();
            reverse = !reverse;
            }
            perc = tsi.readPercentage();
            ThisThread::sleep_for(100ms);
        }
        redLed.write(0);
        greenLed.write(0);
        blueLed.write(0);
        sem1.release();
        timer.reset();
    }
}

int main()
{
    timer.start();
    redLed.period(30);
    greenLed.period(30);
    blueLed.period(30);

    redLed.write(1);
    greenLed.write(1);
    blueLed.write(1);

    Thread2.start(thread2);
    Thread3.start(thread3);
    Thread4.start(thread4);
    thread1();
    
}
