
#ifndef __MOTOR_H
#define __MOTOR_H


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

void forward(void);
void backward(void);
void stop(void);
void turn_around(void);
extern uint8_t left_turn_active; // for turn signals, 0 = off, 1 = blinking
extern uint8_t right_turn_active;


#endif /* __MOTOR_H */
