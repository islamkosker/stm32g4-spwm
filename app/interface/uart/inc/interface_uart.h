/**
 * @file interface_uart.h
 * @brief
 * @author islamkosker (https://github.com/islamkosker)
 * @date 2026-04-28 22:10
 * @version 1.0.0
 * @copyright Copyright (c) 2026 İslam Köşker. 
 */

#ifndef INTERFACE_UART_H
#define INTERFACE_UART_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    UART_TX_IDLE,
    UART_TX_BUSY,

} uart_tx_state_t;

#define UART_TX_QUEUE_SIZE 4

bool interface_uart_send(const uint8_t* data, size_t len);
void interface_uart_poll_tx(void);

#endif