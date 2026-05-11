/**
 * @file app_protocol.c
 * @brief
 * @author islamkosker (https://github.com/islamkosker)
 * @date 2026-05-05 20:14
 * @version 1.0.0
 * @copyright Copyright (c) 2026 İslam Köşker.
 */

#include "app_protocol.h"
#include "app_pwm.h"
#include "interface_uart.h"
#include <string.h>

#define UINT8_BIT 8
#define CRC8_MSB_MASK (1u << (UINT8_BIT - 1))
#define CRC8_POLY 0x07

#define PKT_SOF_IDX 0
#define PKT_CMD_IDX 1
#define PKT_PHI_IDX 2
#define PKT_PLO_IDX 3
#define PKT_CRC_IDX 4

#define PKT_PHI_TIMER_SHIFT 4u
#define PKT_PHI_TIMER_MASK 0xF0u
#define PKT_PHI_PHASE_SHIFT 3u
#define PKT_PHI_PHASE_MASK 0x08u
#define PKT_PHI_CH_MASK 0x01u

#define PKT_PHI_PACK(timer, phase, ch)                                                                  \
    (uint8_t)(((timer) << PKT_PHI_TIMER_SHIFT) | ((phase) << PKT_PHI_PHASE_SHIFT) |                     \
              ((ch) & PKT_PHI_CH_MASK))

#define PKT_PHI_GET_TIMER(phi) (((phi) & PKT_PHI_TIMER_MASK) >> PKT_PHI_TIMER_SHIFT)
#define PKT_PHI_GET_PHASE(phi) (((phi) & PKT_PHI_PHASE_MASK) >> PKT_PHI_PHASE_SHIFT)
#define PKT_PHI_GET_CH(phi) ((phi) & PKT_PHI_CH_MASK)

static uart_parser_t g_parser;

static uint8_t crc_u8(uint8_t crc, uint8_t data)
{
    crc ^= data;
    for (size_t bit = 0; bit < UINT8_BIT; bit++)
        crc = (uint8_t)(crc & CRC8_MSB_MASK) ? (crc << 1) ^ CRC8_POLY : (crc << 1);

    return crc;
}

static uint8_t crc_block(uint8_t crc, const uint8_t* data, size_t n)
{
    for (size_t i = 0; i < n; i++)
        crc = crc_u8(crc, data[i]);
    return crc;
}

static void protocol_pack_pwm_status(timer_id_t t, uint8_t* p_hi, uint8_t* p_lo)
{
    if (t >= TIMER_COUNT || p_hi == NULL || p_lo == NULL)
    {
        if (p_hi) *p_hi = 0;
        if (p_lo) *p_lo = 0;
        return;
    }

    pwm_group_t* p = &PWM_GROUPS[t];
    *p_hi = (uint8_t)t;
    uint8_t lo = (uint8_t)(p->freq_id & 3u);
    if (p->pwm_channels[PWM_CH1].active) lo |= (1u << 2);
    if (p->pwm_channels[PWM_CH2].active) lo |= (1u << 3);
    *p_lo = lo;
}

static bool check_packet_wait_sof(uart_parser_t* parser, uint8_t byte)
{
    if (byte != UART_SOF) return false;

    parser->crc = crc_u8(0x00, byte);
    parser->state = PARSE_CMD;
    return false;
}

static bool check_packet_cmd(uart_parser_t* parser, uint8_t byte)
{
    parser->cmd = byte;
    parser->crc = crc_u8(parser->crc, byte);
    parser->state = PARSE_P_HI;
    return false;
}

static bool check_packet_payload_high(uart_parser_t* parser, uint8_t byte)
{
    parser->p_hi = byte;
    parser->crc = crc_u8(parser->crc, byte);
    parser->state = PARSE_P_LO;
    return false;
}

static bool check_packet_payload_low(uart_parser_t* parser, uint8_t byte)
{
    parser->p_lo = byte;
    parser->crc = crc_u8(parser->crc, byte);
    parser->state = PARSE_CRC;
    return false;
}

static bool check_packet_crc(uart_parser_t* parser, uint8_t byte)
{
    if (byte == parser->crc)
    {
        parser->state = PARSE_WAIT_SOF;
        return true;
    }
    else if (byte == UART_SOF)
    {
        parser->crc = crc_u8(0x00, byte);
        parser->state = PARSE_CMD;
        return false;
    }

    parser->state = PARSE_WAIT_SOF;
    return false;
}

static void protocol_dispatch_get_pwm_status(uart_parser_t* parser)
{
    if (parser->p_hi == CMD_GET_PWM_STATUS_ALL_TIMERS)
    {
        uint8_t buf[UART_PACKET_SIZE * TIMER_COUNT];
        for (int t = 0; t < TIMER_COUNT; ++t)
        {
            uint8_t* frame = &buf[UART_PACKET_SIZE * t];
            protocol_pack_pwm_status((timer_id_t)t, &frame[PKT_PHI_IDX], &frame[PKT_PLO_IDX]);
            frame[PKT_SOF_IDX] = UART_SOF;
            frame[PKT_CMD_IDX] = CMD_GET_PWM_STATUS;
            frame[PKT_CRC_IDX] = crc_block(0, frame, 4);
        }
        interface_uart_send(buf, sizeof(buf));
        return;
    }

    if (parser->p_hi >= TIMER_COUNT) return;

    uint8_t buf[UART_PACKET_SIZE];
    protocol_pack_pwm_status((timer_id_t)parser->p_hi, &buf[PKT_PHI_IDX], &buf[PKT_PLO_IDX]);
    buf[PKT_SOF_IDX] = UART_SOF;
    buf[PKT_CMD_IDX] = CMD_GET_PWM_STATUS;
    buf[PKT_CRC_IDX] = crc_block(0, buf, 4);
    interface_uart_send(buf, sizeof(buf));
}

/* DISPATCHERS */
static void protocol_dispatch_stop_timer(uart_parser_t* parser)
{
    timer_id_t timer = (timer_id_t)parser->p_hi;
    if (timer >= TIMER_COUNT) return;
    pwm_stop(timer);
}

static void protocol_dispatch_start_timer(uart_parser_t* parser)
{
    timer_id_t timer = (timer_id_t)parser->p_hi;
    if (timer >= TIMER_COUNT) return;
    pwm_start(timer);
}

static void protocol_dispatch_set_freq(uart_parser_t* parser)
{
    timer_id_t timer = (timer_id_t)parser->p_hi;
    freq_config_id_t freq = (freq_config_id_t)parser->p_lo;

    if (timer >= TIMER_COUNT || freq >= SINE_FREQ_COUNT) return;

    pwm_set_frequency(timer, freq);
}

static void protocol_dispatch_set_ampl(uart_parser_t* parser)
{
    timer_id_t timer = (timer_id_t)PKT_PHI_GET_TIMER(parser->p_hi);
    bool phase_inverted = PKT_PHI_GET_PHASE(parser->p_hi) != 0u;
    pwm_channel_id_t ch = (pwm_channel_id_t)PKT_PHI_GET_CH(parser->p_hi);
    uint32_t scale = ((uint32_t)parser->p_lo * PWM_SCALE_MAX) / 255U;

    if (timer >= TIMER_COUNT || ch >= DEFAULT_PWM_CHANNEL_COUNT) return;
    pwm_set_amplitude(timer, ch, phase_inverted, scale);
}

bool protocol_dispatcher_check(uart_parser_t* parser, uint8_t byte)
{
    switch (parser->state)
    {
    case PARSE_WAIT_SOF: return check_packet_wait_sof(parser, byte);
    case PARSE_CMD: return check_packet_cmd(parser, byte);
    case PARSE_P_HI: return check_packet_payload_high(parser, byte);
    case PARSE_P_LO: return check_packet_payload_low(parser, byte);
    case PARSE_CRC: return check_packet_crc(parser, byte);
    default: return false;
    }
}

void protocol_dispatcher(uart_parser_t* parser)
{
    switch (parser->cmd)
    {
    case CMD_SET_FREQ: protocol_dispatch_set_freq(parser); break;
    case CMD_TIMER_START: protocol_dispatch_start_timer(parser); break;
    case CMD_TIMER_STOP: protocol_dispatch_stop_timer(parser); break;
    case CMD_SET_AMPL: protocol_dispatch_set_ampl(parser); break;
    case CMD_GET_PWM_STATUS: protocol_dispatch_get_pwm_status(parser); break;
    }
}
void protocol_dispatcher_feed(const uint8_t* data, uint16_t len)
{
    if (!data || !len) return;

    for (uint16_t i = 0U; i < len; i++)
        if (protocol_dispatcher_check(&g_parser, data[i])) protocol_dispatcher(&g_parser);
}

void protocol_dispatcher_init(void)
{
    memset(&g_parser, 0, sizeof(g_parser));
    g_parser.state = PARSE_WAIT_SOF;
}