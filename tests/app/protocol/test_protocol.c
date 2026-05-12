/**
 * @file test_protocol.c
 * @brief UART command parsing edge cases
 */

#include "unity.h"
#include <stdint.h>

#include "app_protocol.h"
#include "app_pwm.h"
#include "test_common.h"

#define UART_SOF 0xAAU
#define CRC8_POLY 0x07U

static uint8_t crc8(const uint8_t* data, size_t n)
{
    uint8_t crc = 0;
    for (size_t i = 0; i < n; ++i)
    {
        crc ^= data[i];
        for (int b = 0; b < 8; ++b)
            crc = (crc & 0x80U) ? (uint8_t)((crc << 1) ^ CRC8_POLY) : (uint8_t)(crc << 1);
    }
    return crc;
}

static void send_packet(uart_parser_t* parser, uint8_t cmd, uint8_t p_hi, uint8_t p_lo)
{
    uint8_t buf[4] = {UART_SOF, cmd, p_hi, p_lo};
    uint8_t crc = crc8(buf, 4);
    protocol_dispatcher_check(parser, UART_SOF);
    protocol_dispatcher_check(parser, cmd);
    protocol_dispatcher_check(parser, p_hi);
    protocol_dispatcher_check(parser, p_lo);

    if (protocol_dispatcher_check(parser, crc)) protocol_dispatcher(parser);
}

static uint8_t pack_ampl_phi(uint8_t timer_id, bool phase, pwm_channel_id_t ch)
{
    return (uint8_t)(((timer_id & 0x0Fu) << 4) | ((phase & 1u) << 3u) | (ch & 0x01u));
}
void test_drops_phase_inverted_request(void)
{
    test_common_reset_all();
    uart_parser_t parser;
    protocol_dispatcher_init();

    send_packet(&parser, CMD_SET_AMPL, pack_ampl_phi(TIMER_1, true, PWM_CH1), 200);

    /* Channel must remain in-phase (no pending update either) since the
     * packet should be silently dropped before pwm_set_amplitude was invoked. */
    pwm_channel_t* c = &PWM_GROUPS[TIMER_1].pwm_channels[PWM_CH1];
    TEST_ASSERT_FALSE(c->phase_inverted);
    TEST_ASSERT_FALSE(c->pending_phase_inverted);
    TEST_ASSERT_FALSE(c->update_pending);
}

void test_accepts_phase_inverted_request(void)
{
    test_common_reset_all();
    uart_parser_t parser = {0};
    protocol_dispatcher_init();
    send_packet(&parser, CMD_SET_FREQ, TIMER_1, SINE_FREQ_3KHZ);
    TEST_ASSERT_EQUAL_INT(SINE_FREQ_3KHZ, pwm_get_frequency_id(TIMER_1));

    send_packet(&parser, CMD_SET_AMPL, pack_ampl_phi(TIMER_1, true, PWM_CH2), 255);

    pwm_channel_t* c = &PWM_GROUPS[TIMER_1].pwm_channels[PWM_CH2];

    TEST_ASSERT_TRUE_MESSAGE(c->pending_phase_inverted, " [ERROR]]: c->pending_phase_inverted");
    TEST_ASSERT_TRUE_MESSAGE(c->update_pending, " [ERROR]]: c->update_pending");
    TEST_ASSERT_EQUAL_UINT32(PWM_SCALE_MAX, c->pending_scale);
}

void test_set_mode_rejects_out_of_range(void)
{
    test_common_reset_all();
    uart_parser_t parser;
    protocol_dispatcher_init();

    send_packet(&parser, CMD_SET_FREQ, TIMER_COUNT, SINE_FREQ_3KHZ);
    send_packet(&parser, CMD_SET_FREQ, TIMER_1, SINE_FREQ_3KHZ);
    TEST_ASSERT_EQUAL_INT(SINE_FREQ_3KHZ, pwm_get_frequency_id(TIMER_1));
}

void test_set_ampl_in_phase_is_always_accepted(void)
{
    test_common_reset_all();
    uart_parser_t parser = {0};
    protocol_dispatcher_init();

    send_packet(&parser, CMD_SET_AMPL, pack_ampl_phi(TIMER_1, false, PWM_CH1), 128);

    pwm_channel_t* c = &PWM_GROUPS[TIMER_1].pwm_channels[PWM_CH1];
    TEST_ASSERT_TRUE(c->update_pending);
    TEST_ASSERT_FALSE(c->pending_phase_inverted);
}

void test_crc_sof_collision_stream_loss(void)
{
    test_common_reset_all();
    uart_parser_t parser = {0};
    protocol_dispatcher_init();

    uint8_t stream[] = {UART_SOF, CMD_SET_AMPL, pack_ampl_phi(TIMER_1, false, PWM_CH1), 100,

                        // CRC missing packet
                        UART_SOF, CMD_SET_AMPL, pack_ampl_phi(TIMER_1, false, PWM_CH1), 200, 0};

    stream[8] = crc8(&stream[4], 4);
    for (size_t i = 0; i < sizeof(stream); i++)
        if (protocol_dispatcher_check(&parser, stream[i])) protocol_dispatcher(&parser);

    pwm_channel_t* c = &PWM_GROUPS[TIMER_1].pwm_channels[PWM_CH1];

    TEST_ASSERT_TRUE(c->update_pending);
}

void test_pack_pwm_status_bitfield(void)
{
    test_common_reset_all();
    pwm_set_frequency(TIMER_1, SINE_FREQ_10KHZ);
    pwm_start(TIMER_1);

    uint8_t hi = 0;
    uint8_t lo = 0;
    protocol_pack_pwm_status(TIMER_1, &hi, &lo);

    TEST_ASSERT_EQUAL_UINT8((uint8_t)TIMER_1, hi);
    /* b1..0=freq(10K=2), b2=CH1 active, b3=CH2 active*/
    uint8_t expected = (uint8_t)((2u & 3u) | (1u << 2) | (1u << 3));
    TEST_ASSERT_EQUAL_UINT8(expected, lo);
}

void test_pack_pwm_status_invalid_timer(void)
{
    uint8_t hi = 0xFF;
    uint8_t lo = 0xFF;
    protocol_pack_pwm_status(TIMER_COUNT, &hi, &lo);
    TEST_ASSERT_EQUAL_UINT8(0, hi);
    TEST_ASSERT_EQUAL_UINT8(0, lo);
}

void test_pack_pwm_status_stopped_timer(void)
{
    test_common_reset_all();
    pwm_set_frequency(TIMER_1, SINE_FREQ_2K5HZ);
    uint8_t hi = 0;
    uint8_t lo = 0;
    protocol_pack_pwm_status(TIMER_1, &hi, &lo);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)TIMER_1, hi);
    TEST_ASSERT_EQUAL_UINT8(0u, lo);
}

void test_protocol(void)
{
    RUN_TEST(test_drops_phase_inverted_request);
    RUN_TEST(test_accepts_phase_inverted_request);
    RUN_TEST(test_set_mode_rejects_out_of_range);
    RUN_TEST(test_set_ampl_in_phase_is_always_accepted);
    RUN_TEST(test_crc_sof_collision_stream_loss);
    RUN_TEST(test_pack_pwm_status_bitfield);
    RUN_TEST(test_pack_pwm_status_invalid_timer);
    RUN_TEST(test_pack_pwm_status_stopped_timer);
}
