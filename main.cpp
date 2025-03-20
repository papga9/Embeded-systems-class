/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"


// Blinking rate in milliseconds
#define BLINKING_RATE     500ms

DigitalOut led(LED1);

int main()
{
    // Initialise the digital pin LED1 as an output

    while (true) {
        led = !led;
        ThisThread::sleep_for(BLINKING_RATE);
    }
}
