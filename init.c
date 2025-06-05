#include "Clock.h"
#include "msp.h"
#include <stdio.h>
#include "init.h"

#define SPEED 1500

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
