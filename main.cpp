/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "TSISensor.h"
#include "rtos.h"
#include <cstdio>
#include <stdint.h>

PwmOut redLed   (LED_RED);
PwmOut greenLed (LED_GREEN);
PwmOut blueLed  (LED_BLUE);

TSISensor tsi;
Thread ThreadTSI(osPriorityNormal, 2048);

typedef struct {
    float percentage;
    int distance;
    int selected;
} mail_t;

Mail<mail_t, 1> mail_box;

void GetTSI() {
    float perc = 0;
    int sel = 0;
    int dist = 0;

    while (true) {
        perc = tsi.readPercentage();
        dist = tsi.readDistance();

        mail_t *mail = mail_box.try_alloc();
        if (mail != nullptr) {
            mail->percentage = perc;
            mail->distance = dist;
        

        if (dist <= 13 && perc != 0) {
            mail->selected = 1;
        }

        if (dist > 13 && dist <= 26 && perc != 0) {
            mail->selected = 2;
        }

        if (dist > 26 && perc != 0)
        {
            mail->selected = 3;
        }

        if (perc == 0)
        {
            mail->selected = 0;
        }
        mail_box.put(mail);
    }
    ThisThread::sleep_for(100ms);
    }
}

void SetLED() {
    while (true) {
        float perc = 0;
        int sel = 0;
        int dist = 0;

        mail_t *mail = mail_box.try_get();
        if (mail != nullptr) {
            perc = mail->percentage;
            sel = mail->selected;
            dist = mail->distance;
            mail_box.free(mail);

            switch (sel) {
            case 0:
                redLed.write(1);
                greenLed.write(1);
                blueLed.write(1);
                break;

            case 1:
                redLed.write(1);
                greenLed.write(0);
                blueLed.write(0);
                break;

            case 2:
                redLed.write(0);
                greenLed.write(1);
                blueLed.write(0);
                break;

            case 3:
                redLed.write(0);
                greenLed.write(0);
                blueLed.write(1);
                break;

            default:
            break;
            }

            printf("Perc.: %f \n", perc);
        }

        ThisThread::sleep_for(100ms);
    }
}

int main()
{
    redLed.period(30);
    greenLed.period(30);
    blueLed.period(30);

    ThreadTSI.start(GetTSI);
    SetLED();
}
