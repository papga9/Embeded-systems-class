#include "mbed.h"
#include "rtos.h"
#include <cstdio>
#include <stdint.h>
#include <stdio.h>
#include <vector>
#include "MMA8451Q.h"

#define STOP_FLAG 1

MMA8451Q IMU(PTE25, PTE24, 0x3A);

Thread thread1(osPriorityNormal, 1024);
Thread thread2(osPriorityNormal, 1024);
Thread thread3(osPriorityNormal, 1024);

typedef struct {
    float accX = 0.0;
    float accY = 0.0;
    float accZ = 0.0;
} mail_t;

Mail<mail_t, 5> mail_box;
Mail<mail_t, 1> mail_calibrate;

void calibrate() {
    int NumOfMeasurements = 50;

    float accX = 0.0;
    float accY = 0.0;
    float accZ = 0.0;

    for (int i = 0; i < NumOfMeasurements; i++){
        accX += IMU.getAccX();
        accY += IMU.getAccY();
        accZ += IMU.getAccZ();
    }

    mail_t *mail = mail_calibrate.try_alloc();
    mail->accX = accX / NumOfMeasurements;
    mail->accY = accY / NumOfMeasurements;
    mail->accZ = accZ / NumOfMeasurements;
    mail_calibrate.put(mail);

    printf("Calibration done");
}


int main()
{
    printf("hallo");
    thread1.start(calibrate);
    thread1.join();    
}
