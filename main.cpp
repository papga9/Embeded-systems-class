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

Mutex ioMutex;

typedef struct {
    float accX = 0.0;
    float accY = 0.0;
    float accZ = 0.0;
} mail_t;

Mail<mail_t, 5> mail_box;
Mail<mail_t, 1> mail_calibrate;

Timer dataTimer;

static float x[5] = {0}, y[5] = {0}, z[5] = {0};

void newInput(char axis, float data){
    switch (axis) {
    case 'x' :
        for (int i = 0; i < 4; i++){
            x[i+1] = x[i];
        }
        x[0] = data;
    break;

    case 'y' :
        for (int i = 0; i < 4; i++){
            y[i+1] = y[i];
        }
        y[0] = data;
    break;

    case 'z' :
        for (int i = 0; i < 4; i++){
            z[i+1] = z[i];
        }
        z[0] = data;
    break;
    }
}

float getAVG(char axis){
    float result = 0.0;

    switch (axis) {
    case 'x' :
        for (int i = 0; i < 5; i++){
            result += x[i];
        }
    break;

    case 'y' :
        for (int i = 0; i < 5; i++){
            result += y[i];
        }
    break;

    case 'z' :
        for (int i = 0; i < 5; i++){
            result += z[i];
        }
    break;
    }
    return result / 5;
}

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
    if (mail != nullptr){
        mail->accX = accX / NumOfMeasurements;
        mail->accY = accY / NumOfMeasurements;
        mail->accZ = accZ / NumOfMeasurements;
        mail_calibrate.put(mail);
    }
    

    ioMutex.lock();
    printf("Kalibrálás elvégezve");
    ioMutex.unlock();
}

void readData(float corrX, float corrY, float corrZ){
    mail_t *mail = mail_box.try_alloc();
    if (mail != nullptr){
        newInput('x', IMU.getAccX());
        newInput('y', IMU.getAccY());
        newInput('z', IMU.getAccZ());

        mail->accX = getAVG('x') - corrX;
        mail->accY = getAVG('y') - corrY;
        mail->accZ = getAVG('z') - corrZ;
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
        roll = 180/3.1415 * atan2(accY, accZ);
        pitch = (180/3.1415) * atan2(-1* accX, sqrt(pow(accY, 2) + pow(accZ, 2)));
    
        ioMutex.lock();
        printf("Roll: %f : Pitch: %f\n", roll, pitch);
        ioMutex.unlock();
    }
    }
}

void thread2(){
    mail_t *mail = mail_calibrate.try_get();

    float correctionAccX = 0.0;
    float correctionAccY = 0.0;
    float correctionAccZ = 0.0;

    if (mail != nullptr){
        correctionAccX = mail->accX;
        correctionAccY = mail->accY;
        correctionAccZ = mail->accZ;
        mail_calibrate.free(mail);
    }

    while (true){
        if(dataTimer.read() > 0.05){
            readData(correctionAccX, correctionAccY, correctionAccZ);
            dataTimer.reset();
        }else{
            ThisThread::sleep_for(50ms);
        }
    }
}


int main()
{
    thread1.start(calibrate);
    thread1.join();  

    dataTimer.start();
    Thread2.start(thread2);
    thread3.start(calculate);
}
