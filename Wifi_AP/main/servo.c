#include "servo.h"
#include "esp_log.h"

// Tag for logging
static const char *TAG = "SERVO";

int current_angle = 0;
mcpwm_cmpr_handle_t comparator;

// Convert angle to pulse width
uint32_t angle_to_pulsewidth(int angle) {
    return (angle - SERVO_MIN_DEGREE) * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) /
           (SERVO_MAX_DEGREE - SERVO_MIN_DEGREE) + SERVO_MIN_PULSEWIDTH_US;
}

// Set the servo to a specific angle
void set_servo_angle(int angle) {
    if (angle < SERVO_MIN_DEGREE) angle = SERVO_MIN_DEGREE;
    if (angle > SERVO_MAX_DEGREE) angle = SERVO_MAX_DEGREE;
    ESP_LOGI(TAG, "Setting angle to %d", angle);
    mcpwm_comparator_set_compare_value(comparator, angle_to_pulsewidth(angle));
    current_angle = angle;
}

// Initialize the servo
void init_servo() {
    mcpwm_timer_handle_t timer;
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000,
        .period_ticks = 20000,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };
    mcpwm_new_timer(&timer_config, &timer);

    mcpwm_oper_handle_t operator;
    mcpwm_operator_config_t operator_config = {.group_id = 0};
    mcpwm_new_operator(&operator_config, &operator);

    mcpwm_operator_connect_timer(operator, timer);

    mcpwm_comparator_config_t comparator_config = {.flags.update_cmp_on_tez = true};
    mcpwm_new_comparator(operator, &comparator_config, &comparator);

    mcpwm_gen_handle_t generator;
    mcpwm_generator_config_t generator_config = {.gen_gpio_num = SERVO_GPIO};
    mcpwm_new_generator(operator, &generator_config, &generator);

    mcpwm_comparator_set_compare_value(comparator, angle_to_pulsewidth(0));
    mcpwm_generator_set_action_on_timer_event(generator, MCPWM_GEN_TIMER_EVENT_ACTION(
        MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH));
    mcpwm_generator_set_action_on_compare_event(generator, MCPWM_GEN_COMPARE_EVENT_ACTION(
        MCPWM_TIMER_DIRECTION_UP, comparator, MCPWM_GEN_ACTION_LOW));

    mcpwm_timer_enable(timer);
    mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP);
}
