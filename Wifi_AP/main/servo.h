#ifndef SERVO_H
#define SERVO_H

#include "driver/mcpwm_prelude.h"

// Servo configuration
#define SERVO_MIN_PULSEWIDTH_US 500
#define SERVO_MAX_PULSEWIDTH_US 2500
#define SERVO_MIN_DEGREE -90
#define SERVO_MAX_DEGREE 90
#define SERVO_GPIO 4

// Function declarations
void init_servo(void);
void set_servo_angle(int angle);
uint32_t angle_to_pulsewidth(int angle);
extern int current_angle;

#endif // SERVO_H
