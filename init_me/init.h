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
void left_forward();
void left_backward();
void right_forward();
void right_backward();
void front();
void right();
void left();
uint32_t get_left_rpm();
void move(uint16_t leftDuty, uint16_t rightDuty);

struct Coordinate;
struct Node;
struct Direction;

Node* create_node();
void initialize_node(Node* node);
Node* Get_Dest();
int check_next(int x, int y);
void add_direction(Node* node, int direction_index, Node* new_node, int reverse_direction);