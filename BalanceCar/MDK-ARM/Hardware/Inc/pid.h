#ifndef __PID_H
#define __PID_H



void motorControl(void);
int Turn(float Gyro);
int Velocity(int Encoder_Left,int Encoder_Right);
int Balance(float Angle,float Gyro);
#endif
