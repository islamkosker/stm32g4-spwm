/**
 * @file app_protocol.h
 * @brief
 * @author islamkosker (https://github.com/islamkosker)
 * @date 2026-04-28 21:44
 * @version 1.0.0
 * @copyright Copyright (c) 2026 İslam Köşker.
 */

#ifndef APP_PROTOCOL_H
#define APP_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "app_pwm.h"


#define UART_SOF 0xAAU
#define UART_PACKET_SIZE 5

typedef enum
{
    CMD_SET_FREQ = 0x01,
    CMD_TIMER_START,
    CMD_TIMER_STOP,
    CMD_SET_AMPL,
    CMD_GET_PWM_STATUS,
} uart_cmd_t;

#define CMD_GET_PWM_STATUS_ALL_TIMERS 0xFFU

typedef enum
{
    PARSE_WAIT_SOF = 0,
    PARSE_CMD,
    PARSE_P_HI, // payload high
    PARSE_P_LO, // payload low
    PARSE_CRC
} parse_state_t;

typedef struct
{
    parse_state_t state;
    uint8_t cmd;
    uint8_t p_hi;
    uint8_t p_lo;
    uint8_t crc;
} uart_parser_t;

bool protocol_dispatcher_check(uart_parser_t* parser, uint8_t byte);
void protocol_dispatcher(uart_parser_t* parser);
void protocol_dispatcher_feed(const uint8_t* data, uint16_t len);
void protocol_dispatcher_init(void);

void protocol_pack_pwm_status(timer_id_t t, uint8_t* p_hi, uint8_t* p_lo);

#endif // APP_PROTOCOL_H