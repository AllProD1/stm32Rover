
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "Command_Process.h"
#include "liquidcrystal_i2c.h"
#include "motor.h"
/* USER CODE END Includes */

// LED VARRIBLES
uint8_t left_turn_active = 0; // for turn signals, 0 = off, 1 = blinking
uint8_t right_turn_active = 0;
double right_percentage = 50.0;
double left_percentage = 55.55;


void forward(void) {
		print("L: %.1f  R: %.1f\r\n", left_percentage, right_percentage);


		// Right, forwards
		double R_Speed = (right_percentage/100.0)*(4000);
		HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
		TIM3->CCR1 = R_Speed;
		HAL_GPIO_WritePin(L298N_IN1_GPIO_Port, L298N_IN1_Pin, 0);
		HAL_GPIO_WritePin(L298N_IN2_GPIO_Port, L298N_IN2_Pin, 1);

		// Left, forwards
		double L_Speed = (left_percentage/100.0)*(4000);
		HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
		TIM3->CCR2 = L_Speed;
		HAL_GPIO_WritePin(L298N_IN3_GPIO_Port, L298N_IN3_Pin, 1);
		HAL_GPIO_WritePin(L298N_IN4_GPIO_Port, L298N_IN4_Pin, 0);

}

// MOVE MOTORS BACKWORDS
void backward(void){

		// Right, backwards
		double R_Speed = (right_percentage/100.0)*(4000);
		HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
		TIM3->CCR1 = R_Speed;
		HAL_GPIO_WritePin(L298N_IN1_GPIO_Port, L298N_IN1_Pin, 1);
		HAL_GPIO_WritePin(L298N_IN2_GPIO_Port, L298N_IN2_Pin, 0);

		// Left, backwards
		double L_Speed = (left_percentage/100.0)*(4000);
		HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
		TIM3->CCR2 = L_Speed;
		HAL_GPIO_WritePin(L298N_IN3_GPIO_Port, L298N_IN3_Pin, 0);
		HAL_GPIO_WritePin(L298N_IN4_GPIO_Port, L298N_IN4_Pin, 1);

}


// STOP MOTORS
void stop(void) {


		// Right, stop
		double R_Speed = (50.0/100.0)*(4000);
		HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
		TIM3->CCR1 = R_Speed;
		HAL_GPIO_WritePin(L298N_IN1_GPIO_Port, L298N_IN1_Pin, 0);
		HAL_GPIO_WritePin(L298N_IN2_GPIO_Port, L298N_IN2_Pin, 0);

		// Left, stop
		double L_Speed = (55.55/100.0)*(4000);
		HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
		TIM3->CCR2 = L_Speed;
		HAL_GPIO_WritePin(L298N_IN3_GPIO_Port, L298N_IN3_Pin, 0);
		HAL_GPIO_WritePin(L298N_IN4_GPIO_Port, L298N_IN4_Pin, 0);

}


//TURN AROUND IN A 180
void turn_around(void){

	// need to turn on left turn signal as that is the turn the robot makes
			left_turn_active = 1;

	// Right, forwards
			double R_Speed = (50.0/100.0)*(4000);
			HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
			TIM3->CCR1 = R_Speed;
			HAL_GPIO_WritePin(L298N_IN1_GPIO_Port, L298N_IN1_Pin, 0);
			HAL_GPIO_WritePin(L298N_IN2_GPIO_Port, L298N_IN2_Pin, 1);
	// Left, backwards
			double L_Speed = (55.55/100.0)*(4000);
			HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
			TIM3->CCR2 = L_Speed;
			HAL_GPIO_WritePin(L298N_IN3_GPIO_Port, L298N_IN3_Pin, 0);
			HAL_GPIO_WritePin(L298N_IN4_GPIO_Port, L298N_IN4_Pin, 1);

}
void set_default_motor_speed(void){
	right_percentage = 50.0;
	left_percentage = 55.55;
}

