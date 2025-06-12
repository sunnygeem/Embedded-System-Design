#include "Clock.h"
#include "msp.h"
#include <stdio.h>
#include "init.h"

#define SPEED 1500

// git
int direct = 0;
int phase = 0;
int ncount = 0;

typedef struct {
    int x;
    int y;
} Coordinate;

Coordinate direct_offset[4] = {
    {0, 1},   // north
    {-1, 0},  // west
    {0, -1},  // south
    {1, 0},   // east
};

typedef struct Node Node;

typedef struct Direction {
    int directNumber;       // 방향 넘버
    Node* node;             // 해당 방향의 노드
    int visited;           // 해당 방향의 탐색 여부 (0: 탐색 안함, 1: 탐색 완료)
} Direction;

typedef struct Node {
    int x;
    int y;
    int id;                     // 노드 ID
    int direction_count;        // 연결된 방향의 수
    Direction directions[4];  // 방향 배열
} Node;

Node* now_node;

#define MAX_NODES 50
Node node_pool[MAX_NODES];
int node_pool_index = 0;


Node* create_node() {
    if (node_pool_index >= MAX_NODES) {
        printf("Error: Maximum number of nodes reached\n");
        return NULL;
    }
    Node* node = &node_pool[node_pool_index];
    initialize_node(node);
    return node;
}


void initialize_node(Node* node) {
    node->x = 0;
    node->y = 0;
    node->id = node_pool_index++;
    node->direction_count = 0;
    int i;
    for (i = 0; i < 4; i++) {
        node->directions[i].directNumber = i;
        node->directions[i].node = NULL;
        node->directions[i].visited = 0;
    }
}

void add_direction(Node* src, int direction_index, Node* dest, int reverse_direction) {
    if (src->direction_count < 4) {
        src->directions[direction_index].node = dest;
        src->directions[direction_index].visited = 1;
        src->direction_count++;
    }

    if (dest->direction_count < 4) {
        dest->directions[reverse_direction].node = src;
        dest->directions[reverse_direction].visited = 1;
        dest->direction_count++;
    }
}

Node* find_or_create_node(int direction_index, Node* current) {
    if (current->directions[direction_index].visited == 0) {
        Node* new_node;
        if(!check_next(current->x + direct_offset[direction_index].x,
                current->y + direct_offset[direction_index].y))
        {
            new_node = create_node();
        }
        else{
            new_node = &node_pool[0];
        }
        int reverse_direction = (direction_index + 2) % 4;
        new_node->x = current->x + direct_offset[direction_index].x;
        new_node->y = current->y + direct_offset[direction_index].y;
        add_direction(current, direction_index, new_node, reverse_direction);

        return new_node;
    }
}

Node* Get_Dest(){
    int x = now_node->x,y = now_node->y,i;
    Node* cur;
    while(1)
    {
        x += direct_offset[direct].x;
        y += direct_offset[direct].y;
        for(i=0;i<node_pool_index;i++){
            cur = &node_pool[i];
            printf("curx: %d, cury : %d, x : %d, y : %d\n",cur->x,cur->y,x,y);
            if(x == cur->x && y == cur->y){
                return cur;
            }
        }
    }
}

void systick_init()
{
    SysTick->LOAD = 0x00FFFFFF;
    SysTick->CTRL = 0x00000005;
}

void sensor_init()
{
    P5->SEL0 &= ~0x08;
    P5->SEL1 &= ~0x08;
    P5->DIR |= 0x08;
    P5->OUT &= ~0x08;

    P9->SEL0 &= ~0x04;
    P9->SEL1 &= ~0x04;
    P9->DIR |= 0x04;
    P9->OUT &= ~0x04;

    P7->SEL0 &= ~0xFF;
    P7->SEL1 &= ~0xFF;
    P7->DIR &= ~0xFF;
}

void pwm_init34(uint16_t period, uint16_t duty3, uint16_t duty4)
{
    // CCR0 period
    TIMER_A0->CCR[0] = period;

    //divide by 1
    TIMER_A0->EX0 = 0x0000;

    // toggle/reset
    TIMER_A0->CCTL[3] = 0x0040;
    TIMER_A0->CCR[3] = duty3;
    TIMER_A0->CCTL[4] = 0x0040;
    TIMER_A0->CCR[4] = duty4;

    // 0x200 -> SMCLK
    // 0b1100 0000 -> input divider /8
    // 0b0011 0000 -> up/down mode
    TIMER_A0->CTL = 0x2F0;

    // set alternative
    P2->DIR |= 0xC0;
    P2->SEL0 |= 0xC0;
    P2->SEL1 &= ~0xC0;
}

void motor_init(void)
{
    P3->SEL0 &= ~0xC0;
    P3->SEL1 &= ~0xC0;
    P3->DIR |= 0xC0;
    P3->OUT &= ~0xC0;

    P5->SEL0 &= ~0x30;
    P5->SEL1 &= ~0x30;
    P5->DIR |= 0x30;
    P5->OUT &= ~0x30;

    P2->SEL0 &= ~0xC0;
    P2->SEL1 &= ~0xC0;
    P2->DIR |= 0xC0;
    P2->OUT &= ~0xC0;

    pwm_init34(7500, 0, 0);
}

void timer_A3_capture_init()
{
    P10->SEL0 |= 0x30;
    P10->SEL1 &= ~0x30;
    P10->DIR &= ~0x30;

    TIMER_A3->CTL &= ~0x0030;
    TIMER_A3->CTL = 0x0200;

    TIMER_A3->CCTL[0] = 0x4910;
    TIMER_A3->CCTL[1] = 0x4910;
    TIMER_A3->EX0 &= ~0x0007;

    NVIC->IP[3] = (NVIC->IP[3] & 0x0000FFFF) | 0x40400000;
    NVIC->ISER[0] = 0x0000C000;
    TIMER_A3->CTL |= 0x0024;
}

void systick_wait1ms()
{
    SysTick->LOAD = 48000;
    SysTick->VAL = 0;
    while ((SysTick->CTRL & 0x00010000) == 0)
    {
    };
}

void systick_wait1s()
{
    int count = 1000;
    int i;
    for (i = 0; i < count; i++)
    {
        systick_wait1ms();
    }
}

void left_forward()
{
    P5->OUT &= ~0x10;
}

void left_backward()
{
    P5->OUT |= 0x10;
}

void right_forward()
{
    P5->OUT &= ~0x20;
}

void right_backward()
{
    P5->OUT |= 0x20;
}

uint32_t get_left_rpm()
{
    return 2000000 / period_left;
}

void move(uint16_t leftDuty, uint16_t rightDuty)
{
    P3->OUT |= 0xC0;
    P2->OUT |= 0xC0;
    TIMER_A0->CCR[3] = leftDuty;
    TIMER_A0->CCR[4] = rightDuty;
}

void TA3_0_IRQHandler()
{
    TIMER_A3->CCTL[0] &= ~0x0001;
    period_right = TIMER_A3->CCR[0] - first_right;
    first_right = TIMER_A3->CCR[0];
}

void TA3_N_IRQHandler()
{
    TIMER_A3->CCTL[1] &= ~0x0001;
    left_count++;
}

void front(){
    left_forward();
    right_forward();
    move(SPEED, SPEED);
}

void left(){
    left_backward();
    right_forward();
    move(SPEED,SPEED);
}

void left90()
{
    while (1)
    {
        if (left_count > 180)
        {
            left_count = 0;
            break;
        }
        else
        {
            left();
        }
    }
    while (1)
    {
        if (left_count > 10)
        {
            left_count = 0;
            break;
        }
        else
        {
            front();
        }
    }

}

void right(){
    left_forward();
    right_backward();
    move(SPEED, SPEED);
}

void right90()
{
    while (1)
    {
        if (left_count > 180)
        {
            left_count = 0;
            break;
        }
        else
        {
            right();
        }
    }
    while (1)
    {
        if (left_count > 10)
        {
            left_count = 0;
            break;
        }
        else
        {
            front();
        }
    }

}
