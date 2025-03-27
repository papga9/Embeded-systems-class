/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "rtos.h"
#include "MMA8451Q.h"
#include <cstdio>

//LED PWM outputs

PwmOut red(LED_RED);
PwmOut green(LED_GREEN);
PwmOut blue(LED_BLUE);

//tread
Thread ThIMU(osPriorityNormal, 2048);

//IMU
MMA8451Q IMU(PTE25, PTE24, 0x3A);

typedef struct {
    float acc_x;
    float acc_y;
    float acc_z;
} message_t;

//Queue and Pool object
Queue<message_t, 1> queue;
MemoryPool<message_t, 1> mpool;

void GetIMU(){
    while(true){
        message_t *message = mpool.alloc();
        message->acc_x = IMU.getAccX();
        message->acc_y = IMU.getAccY();
        message->acc_z = IMU.getAccZ();
        queue.put(message);
        ThisThread::sleep_for(50ms);
    }
}

void SetLED(){
    while(true){
        osEvent evt = queue.get();
        if (evt.status == osEventMessage){
            message_t *message = (message_t*)evt.value.p;
            red.write(message->acc_x);
            green.write(message->acc_y);
            blue.write(message->acc_z);
            printf("x: %f, y: %f, z: %f \n", message->acc_x, message->acc_y, message->acc_z);
            mpool.free(message);
        }
    }
}

int main()
{
    red.period_ms(20);
    green.period_ms(20);
    blue.period_ms(20);

    ThIMU.start(GetIMU);
    SetLED();
}
