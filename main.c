#include "msp.h"
#include "Clock.h"
#include "init.h"
#include <stdio.h>

void main(void)
{
    Clock_Init48MHz();
    int sensor1, sensor2, sensor3, sensor4, sensor5, sensor6, sensor7, sensor8;
    systick_init();
    sensor_init();
    motor_init();
    timer_A3_capture_init();
    TA3_N_IRQHandler();
    while (1)
    {
        //IR센서 키기
        P5->OUT |= 0x08;
        P9->OUT |= 0x04;
        P7->DIR = 0xFF;
        P7->OUT = 0xFF;
        Clock_Delay1us(10);
        P7->DIR = 0x00;
        Clock_Delay1us(1000);

        sensor1 = P7->IN & 0x80;
        sensor2 = P7->IN & 0x40;
        sensor3 = P7->IN & 0x20;
        sensor4 = P7->IN & 0x10;
        sensor5 = P7->IN & 0x08;
        sensor6 = P7->IN & 0x04;
        sensor7 = P7->IN & 0x02;
        sensor8 = P7->IN & 0x01;

        //가운데 -> 직진
        if(sensor4 && sensor5){
            front();
            systick_wait1s();
        }
        else{
            move(0, 0);
            systick_wait1s();
        }

        //IR센서 끄기
        P5->OUT &= ~0x08;
        P9->OUT &= ~0x04;
        Clock_Delay1ms(10);
    }
}
