/*
 * board.h
 *
 *  Created on: 31 aug 2023
 *      Author: klaslofstedt
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include "driver/uart.h"

#define THING_BUTTON_GPIO 23
#define THING_LED_GPIO 22

#define DEPLOY_UART_TXD 1
#define DEPLOY_UART_RXD 3
#define DEPLOY_UART_RTS (UART_PIN_NO_CHANGE)
#define DEPLOY_UART_CTS (UART_PIN_NO_CHANGE)

#define DEPLOY_UART_PORT_NUM UART_NUM_0
#define DEPLOY_UART_BAUD_RATE 115200
#define DEPLOY_UART_BUFFER_SIZE 512

#endif /* _BOARD_H_ */
