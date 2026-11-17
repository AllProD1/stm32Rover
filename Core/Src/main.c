/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
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

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

// LED VARRIBLES
//No longer needed, due to being defined in motor?
//uint8_t left_turn_active = 0; // for turn signals, 0 = off, 1 = blinking
//uint8_t right_turn_active = 0;
uint8_t horn = 0;

uint8_t stop_distance = 60; // in cm ok


#define TRIG_PIN GPIO_PIN_15
#define TRIG_PORT GPIOB
#define ECHO_PIN GPIO_PIN_14
#define ECHO_PORT GPIOB
uint32_t pMillis;
uint32_t Value1 = 0;
uint32_t Value2 = 0;
uint16_t Distance  = 0;  // cm
char buffer[6];
uint32_t servo_millis=0;
uint32_t servo_state=0; //State 2 is left or right, state 1 is forward.
// These time variables are used to determine how long it takes the rover to
// get to the end of the hallway, then run for that amount of time on the way back.
uint32_t startTime = 0;
uint32_t stopTime = 0;
uint32_t diffTime = 0;

// enum for the robot movement state machines
typedef enum {
    STATE_DRIVING_FORWARD,
	STATE_PAUSING_To_TURN,
	STATE_STOP_TURNING,
	STATE_DRIVING_BACK,
	STATE_PURELY_PAUSING
} RobotState;

RobotState robot_state = STATE_DRIVING_FORWARD; // start the state machine in driving forwards
uint32_t   motor_millis = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// LED CODE WE HAD ISSUES WITH THIS DURING TESTING THE BRAKE LIGHTS
// COMMENTING IT OUT FIXED THE ISSUES SHOULD CONSIDER REMOVING IT SOON

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
//	if (htim->Instance == TIM4) {
//	if (left_turn_active) {
//	HAL_GPIO_TogglePin(L_TAILLIGHT_GPIO_Port, L_TAILLIGHT_Pin);
//	}
//	else {
//	// Make sure the light is forced off when not turning
//	// HAL_GPIO_WritePin(L_TAILLIGHT_GPIO_Port, L_TAILLIGHT_Pin, GPIO_PIN_RESET);
//		HAL_GPIO_WritePin(L_TAILLIGHT_GPIO_Port, L_TAILLIGHT_Pin, GPIO_PIN_RESET);
//	}
//	if (right_turn_active) {
//	HAL_GPIO_TogglePin(R_TAILLIGHT_GPIO_Port, R_TAILLIGHT_Pin);
//	}
//	else {
//	// HAL_GPIO_WritePin(R_TAILLIGHT_GPIO_Port, R_TAILLIGHT_Pin, GPIO_PIN_RESET);
//		HAL_GPIO_WritePin(R_TAILLIGHT_GPIO_Port, R_TAILLIGHT_Pin, GPIO_PIN_RESET);
//	}
//
//	// horn
//	if (horn) {
//	HAL_GPIO_TogglePin(HORN_GPIO_Port, HORN_Pin);
//	} else {
//	// HAL_GPIO_WritePin(HORN_GPIO_Port, HORN_Pin, GPIO_PIN_RESET);
//		HAL_GPIO_WritePin(HORN_GPIO_Port, HORN_Pin, GPIO_PIN_RESET);
//				}
//	}
//}





// READ ULTRA PINGER
float readaltra(void){

	HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);  // pull the TRIG pin HIGH
	__HAL_TIM_SET_COUNTER(&htim12, 0);
	while (__HAL_TIM_GET_COUNTER (&htim12) < 10);  // wait for 10 us
	HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);  // pull the TRIG pin low

	pMillis = HAL_GetTick(); // used this to avoid infinite while loop  (for timeout)
	// wait for the echo pin to go high
	while (!(HAL_GPIO_ReadPin (ECHO_PORT, ECHO_PIN)) && pMillis + 10 >  HAL_GetTick());
	Value1 = __HAL_TIM_GET_COUNTER (&htim12);

	pMillis = HAL_GetTick(); // used this to avoid infinite while loop (for timeout)
	// wait for the echo pin to go low
	while ((HAL_GPIO_ReadPin (ECHO_PORT, ECHO_PIN)) && pMillis + 50 > HAL_GetTick());
	Value2 = __HAL_TIM_GET_COUNTER (&htim12);

	Distance = (Value2-Value1)* 0.034/2;

	snprintf(buffer, sizeof(buffer), "%d", Distance);
	char out[16];
	snprintf(out, sizeof(out), " %s\r\n", buffer);
//	HAL_Delay(50);
//	print("%s", out);

//	HD44780_Clear();
//	HD44780_SetCursor(0,0);
//	HD44780_PrintStr(buffer);
//	HAL_Delay(50);
//	__enable_irq();
	return Distance;
}



// SERVO CODE TO PING ON WAY DOWN

void moveServo_right_forward(void){


	//Non-blocking version of the code
	if (servo_millis == 0){
		servo_millis = HAL_GetTick();
		servo_state = 1;
	}
	else if (HAL_GetTick() - servo_millis >= 800  && servo_state == 1){
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 500);   // ~0°
		servo_millis = HAL_GetTick(); //Reset Back to 0
		servo_state = 2;
	}
	else if (HAL_GetTick() - servo_millis >= 1400 && servo_state == 2){
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 1500);   // ~90°
		servo_millis = HAL_GetTick();
		servo_state = 1;
	}

//	if(servo_millis- HAL_GetTick())

}


// SERVO CODE TO PING ON WAY BACK


void moveServo_left_forward(void){
	if (servo_millis == 0){
			servo_millis = HAL_GetTick();
			servo_state = 1;
	}
else if (HAL_GetTick() - servo_millis >= 800 && servo_state == 1){
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 2500);   // ~180°
		servo_millis = HAL_GetTick(); //Reset Back to 0
		servo_state = 2;
	}
	else if (HAL_GetTick() - servo_millis >= 1400 && servo_state == 2){
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 1500);   // ~90°
		servo_millis = HAL_GetTick(); //Reset Back to 0
		servo_state = 1;
	}
}



// BATTERY MONITORING FUNCTION THAT KILLS ROBOT IF LOW POWER

void battery_voltage_shutdown(void){
	// bat mon shut down stuff
			float voltage = getBatteryVoltage();


			if (voltage < 6.0f && voltage > 1.0f) {
			HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
			HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
			int millivolts = (int)(voltage * 1000.0f + 0.5f);
						int volts = millivolts / 1000;
						int frac = millivolts % 1000;
						print(" %d.%03d\r\n", volts, frac);

			// Pls Turn off lights.
			while (1==1){
			//Die XP
			}
			}

}

void TurnSignals(void) { //Credit goes to grok.
    static uint32_t t_turn = 0;
    static uint8_t  blink_on = 0;

    // Same 250 ms cadence as HeartBeat
    if (HAL_GetTick() >= t_turn) {
        blink_on = !blink_on;
        t_turn += 250;
    }

    // Brake lights take priority — solid on, no blinking
    // (BREAKLIGHTS() already forces both pins; we just don't override them here
    //  if you call this only when not braking. Or explicitly:)
    if (left_turn_active && !right_turn_active) {
        HAL_GPIO_WritePin(L_TAILLIGHT_GPIO_Port, L_TAILLIGHT_Pin,
                          blink_on ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(R_TAILLIGHT_GPIO_Port, R_TAILLIGHT_Pin, GPIO_PIN_RESET);
    }
    else if (right_turn_active && !left_turn_active) {
        HAL_GPIO_WritePin(R_TAILLIGHT_GPIO_Port, R_TAILLIGHT_Pin,
                          blink_on ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(L_TAILLIGHT_GPIO_Port, L_TAILLIGHT_Pin, GPIO_PIN_RESET);
    }
    else if (!left_turn_active && !right_turn_active) {
        // neither turning — leave pins alone so BREAKLIGHTS() can own them
        // (or force off if you prefer)
    }
}




// START UP COMMAND TO SHOW ALL LIGHTS AND THE HORN SYSTEM WORK PER REQUIREMENTS

void bist(void){
	 //  more led stuff
	  HEADLIGHTS();
	  for (uint32_t start_tick = HAL_GetTick(); (HAL_GetTick() - start_tick) < 2000;) {

	  }
	  HEADLIGHTS();

	 // horn = 1;
	  HORN(GPIO_PIN_SET);
	  for (uint32_t start_tick = HAL_GetTick(); (HAL_GetTick() - start_tick) < 1000;) {

	  }
	 // horn = 0;
	  HORN(GPIO_PIN_RESET);

	  left_turn_active = 1;
	  for (uint32_t start_tick = HAL_GetTick(); (HAL_GetTick() - start_tick) < 2000;) {

	  }
	  left_turn_active = 0;

	  right_turn_active = 1;
	  for (uint32_t start_tick = HAL_GetTick(); (HAL_GetTick() - start_tick) < 2000;) {

	  }
	  right_turn_active = 0;



}




/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM12_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_TIM8_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */

  // bat mon stuff
  HAL_TIM_Base_Start(&htim8);
  HAL_ADC_Start_IT(&hadc1);

  // starts timers for the motors
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);


  // Light and horn timer
  HAL_TIM_Base_Start_IT(&htim4);



  // sets timer for pinger or servo
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

  HAL_TIM_Base_Start(&htim12);
  HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);  // pull the TRIG pin low
//    HD44780_Init(2);
//    HD44780_Clear();
//    HD44780_SetCursor(0,0);
//    HD44780_PrintStr("HC-SR04");
    HAL_Delay(250);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  print("\033[H\033[2J");
  print("Rover v0.1 %s %s\r\n\r\n", __DATE__, __TIME__);
  battery_voltage_shutdown();

  // boot up delay
//  HAL_Delay(500);


  // startup for leds and horn
//  bist();


  startTime = HAL_GetTick(); // start the "timer" for how long it takes to go down the hall (one way)
  set_default_motor_speed();
  while (1)
  {

 // tasks that are always running

		HeartBeat();
		TurnSignals();   // same 250 ms rhythm
		Command_Proc();
		battery_voltage_shutdown();


		// logic to control movement for the project phase



		// This should stop the rover on the way back. Subtracting 7200 for the time it takes to turn around.
		//TODO: This actually just breaks the code since stopTime is currently. BEN, THIS IS WHY YOU HAD TO ADD THE STATE/CHECK IF IT WAS DRIVING BACK.
		if (HAL_GetTick() - stopTime - 7200 >= diffTime && robot_state == STATE_DRIVING_BACK) {
			while(1){
				stop();
			}
		}


		// THIS SWITCH BELOW IS HUGE should help us going forward
		// Lachlan you should be able to set up your own cases that will correct it left vs. right
		// I also think the timer here may help us remember where we started I am not sure though

		// switch loop to control the conditions for driving the robot
		 switch (robot_state)
		  {
		 	 // default drive forward that stops when 2ft from object sets break lights
		    case STATE_DRIVING_FORWARD:
		        if (readaltra() <= 60.0 && servo_state == 1 && HAL_GetTick() - servo_millis >= 300){
		        	print(" I'm Braking!\r\n");
					BREAKLIGHTS(GPIO_PIN_SET);
					stop();
					motor_millis = HAL_GetTick();
					robot_state = STATE_PAUSING_To_TURN;
		        } else {
		            BREAKLIGHTS(GPIO_PIN_RESET);
		            forward();
		            moveServo_right_forward(); // servo control on the way down the hall

		            if (servo_state == 2 && (HAL_GetTick() - servo_millis) >= 200) {
		                float d = readaltra();
		                const float target   = 105.0f;
		                const float deadband = 6.0f;
		                const float kp       = 0.020f;   // tune this

		                if (d > 15.0f && d < 250.0f) {
		                    float error = d - target;   // + = too far, − = too close

		                    if (error > deadband || error < -deadband) {
		                        // correction grows with how far off you are
		                        float adj = kp * error;          // can be positive or negative
		                        // limit how hard you can kick in one reading
		                        if (adj >  1.5f) adj =  1.5f;
		                        if (adj < -1.5f) adj = -1.5f;

		                        // too far (error > 0) → speed left, slow right
		                        // too close (error < 0) → slow left, speed right
		                        left_percentage  += adj;
		                        right_percentage -= adj * 0.7f;
		                    }

		                    // clamps
		                    if (left_percentage  < 30.0f) left_percentage  = 30.0f;
		                    if (left_percentage  > 85.0f) left_percentage  = 85.0f;
		                    if (right_percentage < 30.0f) right_percentage = 30.0f;
		                    if (right_percentage > 85.0f) right_percentage = 85.0f;
		                }
		            }
		                const float left_default  = 55.55f;   // match set_default_motor_speed()
						const float right_default = 50.0f;
						left_percentage  += (left_default  - left_percentage)  * 0.02f;
						right_percentage += (right_default - right_percentage) * 0.02f;
						if (servo_state == 1 && (HAL_GetTick() - servo_millis) >= 300) {
							float front = readaltra();
							if (front > 60.0f && front < 100.0f) {
								float scale = (front - 60.0f) / 60.0f;  // 1.0 at 120 cm → 0 at 60 cm
								if (scale < 0.4f) scale = 0.4f;
								left_percentage  = 55.55f * scale;
								right_percentage = 50.0f  * scale;
							}
						}
		        }

		        break;

		     // state to wait 3 seconds to make sure it is wall and not random obstacle person or bad data
		     // will go back to driving forward if it is not a wall otherwise to moves on to turn around
		    case STATE_PAUSING_To_TURN:
		        if (readaltra() > 60.0 && servo_state == 1){
		            robot_state = STATE_DRIVING_FORWARD;           // path cleared
		        } else if (HAL_GetTick() - motor_millis >= 3000){
		        		stopTime = HAL_GetTick(); // stop the "timer" for how long it takes to go down the hall one way
		        		diffTime = stopTime - startTime - 3000; // subtract startTime from stopTime to determine how long it took, subtract 3000 for time waiting.
		            turn_around();
		            motor_millis = HAL_GetTick();
		            robot_state = STATE_STOP_TURNING;
		        }

		        break;


		     // this is the turning around case that counts how long it turns around before moving forward again
		    case STATE_STOP_TURNING:
		        if (HAL_GetTick() - motor_millis >= 7600){
		            stop();
		            left_turn_active = 0;          // stop blinking
					right_turn_active = 0;
		            BREAKLIGHTS(GPIO_PIN_RESET);
		            robot_state = STATE_DRIVING_BACK;
		        }
		        break;

		    case STATE_DRIVING_BACK:
		    	if (readaltra() <= 60.0 && servo_state == 1 && HAL_GetTick() - servo_millis >= 300){
		    		BREAKLIGHTS(GPIO_PIN_SET);
		    		stop();
		    		motor_millis = HAL_GetTick();
		    		robot_state = STATE_PURELY_PAUSING;
		        } else {
		        	BREAKLIGHTS(GPIO_PIN_RESET);
		    	    forward();
		    	    moveServo_left_forward(); // servo control on the way back the hall
		    	    if (servo_state == 2 && (HAL_GetTick() - servo_millis) >= 200) {
						float d = readaltra();
						const float target   = 105.0f;
						const float deadband = 6.0f;
						const float kp       = 0.016f;   // tune this

						if (d > 15.0f && d < 250.0f) {
							float error = d - target;   // + = too far, − = too close

							if (error > deadband || error < -deadband) {
								// correction grows with how far off you are
								float adj = kp * error;          // can be positive or negative
								// limit how hard you can kick in one reading
								if (adj >  1.5f) adj =  1.5f;
								if (adj < -1.5f) adj = -1.5f;

								// too far (error < 0)
								// too close (error > 0)
								left_percentage  -= adj;
								right_percentage += adj * 0.7f;
							}

							// clamps
							if (left_percentage  < 30.0f) left_percentage  = 30.0f;
							if (left_percentage  > 85.0f) left_percentage  = 85.0f;
							if (right_percentage < 30.0f) right_percentage = 30.0f;
							if (right_percentage > 85.0f) right_percentage = 85.0f;
						}
					}
						const float left_default  = 55.55f;   // match set_default_motor_speed()
						const float right_default = 50.0f;
						left_percentage  += (left_default  - left_percentage)  * 0.02f;
						right_percentage += (right_default - right_percentage) * 0.02f;
		    	}
		    	break;

		    case STATE_PURELY_PAUSING:

			if (readaltra() > 60.0 && servo_state == 1){
				robot_state = STATE_DRIVING_BACK;           // path cleared
			}
			break;
		    }



    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
