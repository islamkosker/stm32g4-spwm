/**
 * @file interface_uart.c
 * @brief
 * @author islamkosker (https://github.com/islamkosker)
 * @date 2026-05-05 19:17
 * @version 1.0.0
 * @copyright Copyright (c) 2026 İslam Köşker.
 */

#include "interface_uart.h"
#include "app_protocol.h"
#include "stm32g4xx_hal.h"
#include <string.h>

extern UART_HandleTypeDef hlpuart1;
#define INTERFACE_UART hlpuart1

typedef struct
{
    uint8_t data[UART_PACKET_SIZE];
} uart_tx_slot_t;

#define UART_TX_TIMEOUT_MS 50U
#define UART_RX_DMA_BUFFER_LENGTH 64U

// Queue
static uart_tx_slot_t tx_queue[UART_TX_QUEUE_SIZE];
static volatile uint8_t tx_queue_head = 0;
static volatile uint8_t tx_queue_tail = 0;
static volatile uint8_t tx_queue_count = 0;
// TX state & timeout
static volatile uart_tx_state_t tx_state = UART_TX_IDLE;
static uint32_t tx_start_tick = 0;
// RX
uint8_t uart_rx_dma_buffer[UART_RX_DMA_BUFFER_LENGTH];

static HAL_StatusTypeDef uart_rx_dma_start(void)
{

    if (INTERFACE_UART.RxState != HAL_UART_STATE_READY) (void)HAL_UART_AbortReceive(&INTERFACE_UART);

    HAL_StatusTypeDef status =
        HAL_UARTEx_ReceiveToIdle_DMA(&INTERFACE_UART, uart_rx_dma_buffer, UART_RX_DMA_BUFFER_LENGTH);

    if (status != HAL_OK)
    {
        (void)HAL_UART_AbortReceive(&INTERFACE_UART);
        status =
            HAL_UARTEx_ReceiveToIdle_DMA(&INTERFACE_UART, uart_rx_dma_buffer, UART_RX_DMA_BUFFER_LENGTH);
    }
    return status;
}

bool interface_uart_send(const uint8_t* data, size_t len)
{
    if (!data || !len || len > UART_PACKET_SIZE) return false;

    __disable_irq();
    bool full = (tx_queue_count >= UART_TX_QUEUE_SIZE);
    if (!full)
    {
        memcpy(tx_queue[tx_queue_tail].data, data, len);
        tx_queue_tail = (tx_queue_tail + 1) & (UART_TX_QUEUE_SIZE - 1); // power of 2 masking
        tx_queue_count++;
    }
    __enable_irq();

    return !full;
}

void interface_uart_poll_tx(void)
{
    if (tx_state == UART_TX_BUSY)
    {
        if ((HAL_GetTick() - tx_start_tick) >= UART_TX_TIMEOUT_MS)
        {
            HAL_UART_AbortTransmit(&INTERFACE_UART);
            tx_state = UART_TX_IDLE;
        }
        return;
    }

    __disable_irq();
    uint8_t count = tx_queue_count;
    __enable_irq();

    if (!count) return;

    tx_state = UART_TX_BUSY;
    tx_start_tick = HAL_GetTick();

    HAL_StatusTypeDef state =
        HAL_UART_Transmit_DMA(&INTERFACE_UART, tx_queue[tx_queue_head].data, UART_PACKET_SIZE);

    if (state != HAL_OK)
    {
        tx_state = UART_TX_IDLE;
    }
}

/* #############    HAL override    #############*/

__attribute__((used, externally_visible, noinline)) // disable LTO
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t Size)
{
    HAL_UART_RxEventTypeTypeDef event;

    if (huart->Instance != INTERFACE_UART.Instance) return;

    event = HAL_UARTEx_GetRxEventType(huart);
    
    if (Size > 0U) protocol_dispatcher_feed(uart_rx_dma_buffer, Size);

    if (event == HAL_UART_RXEVENT_HT) return;

    (void)uart_rx_dma_start();
}

__attribute__((used, externally_visible, noinline)) // disable LTO
void HAL_UART_ErrorCallback(UART_HandleTypeDef* huart)
{
    if (huart->Instance != INTERFACE_UART.Instance) return;

    __HAL_UART_CLEAR_OREFLAG(&INTERFACE_UART);
    __HAL_UART_CLEAR_NEFLAG(&INTERFACE_UART);
    __HAL_UART_CLEAR_FEFLAG(&INTERFACE_UART);
    __HAL_UART_CLEAR_PEFLAG(&INTERFACE_UART);

    (void)HAL_UART_AbortReceive(huart);
    (void)uart_rx_dma_start();
}

__attribute__((used, externally_visible, noinline)) // disable LTO
void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
    if (huart->Instance != INTERFACE_UART.Instance) return;

    tx_queue_head = (tx_queue_head + 1) & (UART_TX_QUEUE_SIZE - 1);
    tx_queue_count--;
    tx_state = UART_TX_IDLE;
}