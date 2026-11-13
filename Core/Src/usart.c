/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"

/* USER CODE BEGIN 0 */
#define RX_IT_BUFFER_SIZE 1
#define TX_IT_BUFFER_SIZE 1
#define U2_RX_BUF_SIZE 160
#define U2_TX_BUF_SIZE 640


#define L_U2_ARGS  5
#define L_U2_CMD  40

#define TRUE  1
#define FALSE 0

uint8_t sTemp[250];  // used in the macro print
uint16_t L_stemp;

uint8_t uart2_rx[U2_RX_BUF_SIZE];
uint8_t uart2_tx[U2_TX_BUF_SIZE];

static volatile uint16_t uart2_rx_in;
static volatile uint16_t uart2_rx_out;
static volatile uint16_t uart2_tx_in;
static volatile uint16_t uart2_tx_out;

uint8_t U2_Cmd_Rdy;

/* USER CODE END 0 */

UART_HandleTypeDef huart2;

/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */
	uart2_rx_in = 0;
	uart2_rx_out = 0;
	uart2_tx_in = 0;
	uart2_tx_out = 0;

	U2_Cmd_Rdy = FALSE;

  HAL_UART_Receive_IT(&huart2, &uart2_rx[uart2_rx_in], RX_IT_BUFFER_SIZE);

  /* USER CODE END USART2_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = UART_TX_Pin|UART_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, UART_TX_Pin|UART_RX_Pin);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {
		if (uart2_tx_in != uart2_tx_out) {
			HAL_UART_Transmit_IT(&huart2, &uart2_tx[uart2_tx_out],
					TX_IT_BUFFER_SIZE);
			if (++uart2_tx_out >= U2_TX_BUF_SIZE) {
				uart2_tx_out = 0;
			}
		}
	}
}

#if 1
void USART2_write_nonblocking(uint8_t *c, uint16_t L_c) {
	__disable_irq();
	for (uint16_t i = 0; i < L_c; i++) {
		uart2_tx[uart2_tx_in] = c[i];
		if (++uart2_tx_in >= U2_TX_BUF_SIZE) {
			uart2_tx_in = 0;
		}
	}
	HAL_UART_Transmit_IT(&huart2, &uart2_tx[uart2_tx_out], TX_IT_BUFFER_SIZE);
	__enable_irq();
}

#else

void USART2_write_nonblocking(uint8_t *c, uint16_t L_c) {
	HAL_UART_Transmit(&huart2, c, L_c,10000);

}
#endif

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {
		U2_Cmd_Rdy = (uart2_rx[uart2_rx_in] == 0x0D);
		if (++uart2_rx_in >= U2_RX_BUF_SIZE) {
			uart2_rx_in = 0;
		}
		HAL_UART_Receive_IT(&huart2, &uart2_rx[uart2_rx_in], RX_IT_BUFFER_SIZE);
	}
}

uint8_t Get_U2_Cmd(uint8_t *cmd) {
	uint16_t i = 0;
	__disable_irq();
	if ((U2_Cmd_Rdy == TRUE) && (uart2_rx_out != uart2_rx_in)) {
		print("i: %d O: %d\r\n", uart2_rx_in, uart2_rx_out);
		while ((uart2_rx[uart2_rx_out] != 0x0D) && (i < U2_CMD_SIZE)) {
			cmd[i++] = uart2_rx[uart2_rx_out++];
			if (uart2_rx_out >= U2_RX_BUF_SIZE) {
				uart2_rx_out = 0;
			}
		}
		cmd[i++] = 0; // terminate with null character
		U2_Cmd_Rdy = FALSE;
		uart2_rx_out++;
		if (uart2_rx_out >= U2_RX_BUF_SIZE) {
			uart2_rx_out = 0;
		}
		print("i: %d O: %d cmd: %s\r\n", uart2_rx_in, uart2_rx_out,cmd);
	}
	__enable_irq();
	return i;
}

/* USER CODE END 1 */

