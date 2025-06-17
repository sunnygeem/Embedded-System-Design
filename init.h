#include <stdio.h>

uint16_t first_left;
uint16_t first_right;

uint16_t period_left;
uint16_t period_right;

uint32_t left_count;

void TA3_0_IRQHandler();
void systick_init();
void sensor_init();
void pwm_init34(uint16_t period, uint16_t duty3, uint16_t duty4);
void motor_init(void);
void timer_A3_capture_init();
void systick_wait1ms();
void systick_wait1s();
uint32_t get_left_rpm();
void left_forward();
void left_backward();
void right_forward();
void right_backward();
void move_front(uint16_t leftDuty, uint16_t rightDuty, int delay);
void move_right(uint16_t leftDuty, uint16_t rightDuty, int delay);
void move_left(uint16_t leftDuty, uint16_t rightDuty, int delay);
void move(uint16_t leftDuty, uint16_t rightDuty);

