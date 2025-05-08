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
Thread Thread2(osPriorityNormal, 1024);
Thread thread3(osPriorityNormal, 1024);

typedef struct {
    float accX = 0.0;
    float accY = 0.0;
    float accZ = 0.0;
} mail_t;

Mail<mail_t, 5> mail_box;
Mail<mail_t, 1> mail_calibrate;

Timer dataTimer;

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

    printf("Calibration done: %f, %f, %f ", accX, accY, accZ);
}

void readData(float corrX, float corrY, float corrZ){
    mail_t *mail = mail_box.try_alloc();
    if (mail != nullptr){
        mail->accX = IMU.getAccX() - corrX;
        mail->accY = IMU.getAccY() - corrY;
        mail->accZ = IMU.getAccZ() - corrZ;
        mail_box.put(mail);
    }
}

void calculate(){
    float accX = 0.0;
    float accY = 0.0;
    float accZ = 0.0;

    while (true) {

    mail_t *mail = mail_box.try_get();
    if (mail != nullptr) {
        accX = mail->accX;
        accY = mail->accY;
        accZ = mail->accZ;
        mail_box.free(mail);

        float roll = 0.0;
        float pitch = 0.0;

        printf("still alive %f, %f, %f", accX, accY, accZ);
        roll = 180/3.1415 * atan2(accY, accZ);
        pitch = 180/3.1415 * atan2(-1* accX, sqrt(pow(accY, 2) + pow(accZ, 2)));
    
        printf("Roll: %f\n", roll);
        printf("Pitch: %f\n", pitch);
    }
    }
}

void thread2(){
    mail_t *mail = mail_calibrate.try_get();
    float correctionAccX = mail->accX;
    float correctionAccY = mail->accY;
    float correctionAccZ = mail->accZ;
    mail_calibrate.free(mail);

    while (true){
        if(dataTimer.read() > 0.05){
            readData(correctionAccX, correctionAccY, correctionAccZ);
            dataTimer.reset();
        }else{
            ThisThread::sleep_for(10ms);
        }
    }
}


int main()
{
    printf("hallo\n");
    thread1.start(calibrate);
    thread1.join();  

    dataTimer.start();
    Thread2.start(thread2);
    thread3.start(calculate);
}
