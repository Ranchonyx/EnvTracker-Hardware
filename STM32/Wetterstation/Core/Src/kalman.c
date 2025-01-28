/*
 * kalman.c
 *
 *  Created on: May 27, 2024
 *      Author: erikl
 */
#include "kalman.h"

void KalmanFilter_Init(KalmanFilter* filter, float Q, float R, float initial_P, float initial_X_hat) {
    filter->Q = Q;
    filter->R = R;
    filter->P = initial_P;
    filter->X_hat = initial_X_hat;
}

float KalmanFilter_Update(KalmanFilter* filter, float measurement) {
    // Prediction update
    filter->P = filter->P + filter->Q;
    // Measurement update
    filter->K = filter->P / (filter->P + filter->R);
    filter->X_hat = filter->X_hat + filter->K * (measurement - filter->X_hat);
    filter->P = (1 - filter->K) * filter->P;

    return filter->X_hat;
}

void KalmanFilter_PreInitialize(KalmanFilter* filter, float (*readSensorDataFunc)(void)) {
    float sum = 0.0;
    for (int i = 0; i < 3; i++) {
        sum += readSensorDataFunc();
        HAL_Delay(10); // Delay
    }
    filter->X_hat = sum / 3;
}


