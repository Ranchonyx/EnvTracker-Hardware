/*
 * kalman.h
 *
 *  Created on: May 27, 2024
 *      Author: erikl
 */
#ifndef KALMAN_H
#define KALMAN_H

typedef struct {
    float Q; // Process noise covariance
    float R; // Measurement noise covariance
    float P; // Estimate error covariance
    float K; // Kalman gain
    float X_hat; // Estimated value
} KalmanFilter;

void KalmanFilter_Init(KalmanFilter* filter, float Q, float R, float initial_P, float initial_X_hat);
float KalmanFilter_Update(KalmanFilter* filter, float measurement);
void KalmanFilter_PreInitialize(KalmanFilter* filter, float (*readSensorDataFunc)(void));

#endif /* KALMAN_H */

