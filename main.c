#include "msp.h"
#include "Clock.h"

#define RED (1 << 0)
#define GREEN (1 << 1)
#define BLUE (1 << 2)

void Led_Init(void)
{
    P2->SEL0 &= ~0x07;
    P2->SEL1 &= ~0x07;
    P2->DIR |= 0x07;
    P2->OUT &= ~0x07;
}

void Motor_Init(void)
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

    PWM_Init34(15000, 0, 0);
}

void System_Initialize(void)
{
    Clock_Init48MHz();
    Motor_Init();
}

void PWM_Init34(uint16_t period, uint16_t duty3, uint16_t duty4) {
    P2 ->DIR |= 0xC0;
    P2 ->SEL0 |= 0xC0;
    P2 ->SEL1 &= ~0xC0;

    TIMER_A0->CCTL[0] = 0x800;
    TIMER_A0->CCR[0] = period;

    TIMER_A0->EX0 = 0x0000;

    TIMER_A0->CCTL[3] = 0x0040;
    TIMER_A0->CCR[3] = duty3;
    TIMER_A0->CCTL[4] = 0x0040;
    TIMER_A0->CCR[4] = duty4;

    TIMER_A0->CTL = 0x02F0;
}

void Left_Forward() {
    P5->OUT &= ~0x10;
}

void Left_Backward() {
    P5->OUT |= 0x10;
}

void Right_Forward() {
    P5->OUT &= ~0x20;
}

void Right_Backward() {
    P5->OUT |= 0x20;
}

void PWM_Duty3(uint16_t duty3) {
    TIMER_A0->CCR[3] = duty3;
}

void PWM_Duty4(uint16_t duty4) {
    TIMER_A0->CCR[4] = duty4;
}

void Move(uint16_t leftDuty, uint16_t rightDuty) {
    P3->OUT |= 0xC0;
    PWM_Duty3(rightDuty);
    PWM_Duty4(leftDuty);
}

void moveForward(int leftDuty, int rightDuty, int delay) {
    Left_Forward();
    Right_Forward();
    Move(leftDuty, rightDuty);
    Clock_Delay1us(delay);
}

void moveRight(int leftDuty, int rightDuty, int delay) {
    Left_Forward();
    Right_Backward();
    Move(leftDuty, rightDuty);
    Clock_Delay1us(delay);
}

void moveLeft(int leftDuty, int rightDuty, int delay) {
    Left_Backward();
    Right_Forward();
    Move(leftDuty, rightDuty);
    Clock_Delay1us(delay);
}

void loadSensor() {
   P7->DIR = 0xFF;
   P7->OUT = 0xFF;
   Clock_Delay1us(1000);
   P7->DIR = 0x00;
   Clock_Delay1us(1000);
}

void TurnOn_Led(int color)
{
    P2->OUT &= ~0x07;
    P2->OUT |= color;
}

void TurnOff_Led(void)
{
    P2->OUT &= ~0x07;
}

//LSRB 0000
int direction(int duty) {
    loadSensor();
    int res = 0;
    if (P7->IN & 0b01100000)
        res += 0b1000;//L

    if (P7->IN & 0b00000110)
        res += 0b0010;//R

    if ((P7->IN & 0b00011000) == 0b00011000)
        res += 0b0100;//S

    if (P7->IN == 0)
        res += 0b0001;//B

    return res;
}

int main(void)

{
    Led_Init();
    System_Initialize();
    uint16_t duty = 1700;
    int left = 0;
    int right = 0;
    int b = 0;
    int turnCount = 0;
    int rotCnt = 0;
    int index = 0;
    int loopCount = -1;
    int preCount = -1;
    int branch = 0;
    char lst[1000];
    int i;
    for (i = 0; i<1000; i++) {
        lst[i] = 'N';
    }

    //phase 1
    while (1)
    {
        loopCount++;

        //endPoint - end phase 1 and proceed to phase 2
        loadSensor();

//        if(branch == 2){
//            Move(0, 0);
//            Clock_Delay1us(10000);
//        }

        int way = direction(duty);


        if(branch == 2){
            turnCount = 0;
            TurnOff_Led();
            TurnOn_Led(GREEN);
            while(1){

                moveLeft(duty, duty, 5000);
                if(turnCount > 180){
                    branch++;
                    break;
                }
                turnCount++;
            }

            turnCount = 0;
            while(1){
                moveForward(duty, duty, 5000);
                if(turnCount > 250){
                    break;
                }
                turnCount++;

            }

        }
        //go right
        else if (way & 0b0010) {
            loadSensor();
            right = 1;
            left = 0;
            moveForward(duty, duty, 5000);
            turnCount = 0;
            while (1) {
                moveRight(duty, duty, 1000);
                loadSensor();
                if ((P7->IN & 0b00011000) == 0b00011000) {
                    moveRight(duty, duty, 5000);
                    break;
                }
                turnCount++;
            }

            if (turnCount>130) {
                lst[index] = 'r';
                index++;
                branch++;
            }
        }
        //go straight
        else if (way & 0b0100) {
            if (loopCount-preCount>50) {
                loadSensor();
                if ((P7->IN & 0b01111000) == 0b01111000||(P7->IN & 0b00011110) == 0b00011110) {
                    lst[index] = 's';
                    index++;
                    preCount = loopCount;
                }
            }
            moveForward(duty, duty, 5000);
        }
        //go left
        else if (way & 0b1000) {
            left = 1;
            right = 0;
            moveForward(duty, duty, 5000);
            turnCount = 0;
            while (1) {
                moveLeft(duty, duty, 1000);
                loadSensor();
                if ((P7->IN & 0b00011000) == 0b00011000) {
                    moveLeft(duty, duty, 5000);
                    break;
                }
                turnCount++;
            }
            if (turnCount>130) {
                lst[index] = 'l';
                index++;
                preCount = loopCount;
            }
        }
        else if ((left==1) && (way & 0b0001)) {
            moveForward(duty, duty, 5000);
            turnCount = 0;
            while (1) {
                moveLeft(duty, duty, 1000);
                loadSensor();
                if ((P7->IN & 0b00011000) == 0b00011000) {
                    moveLeft(duty, duty, 3200);
                    break;
                }
                turnCount++;
            }
            if (turnCount>800) {
                lst[index] = 'b';
                index++;
                b++;
                TurnOff_Led();
                TurnOn_Led(GREEN);
            }
            else if (turnCount>40 && loopCount-preCount>50) {
                lst[index] = 'l';
                index++;
                TurnOff_Led();
                TurnOn_Led(RED);
                preCount = loopCount;
            }
        }
        else if ((right==1) && (way & 0b0001)) {
            moveForward(duty, duty, 5000);
            turnCount = 0;
            while (1) {
                moveRight(duty, duty, 1000);
                loadSensor();
                if ((P7->IN & 0b00011000) == 0b00011000) {
                    moveRight(duty, duty, 3200);
                    break;
                }
                turnCount++;
            }
            if (turnCount>800) {
                lst[index] = 'b';
                index++;
                b++;
                TurnOff_Led();
                TurnOn_Led(GREEN);
            }
            else if (turnCount>40 && loopCount-preCount>50) {
                lst[index] = 'r';
                index++;
                TurnOff_Led();
                TurnOn_Led(BLUE);
                preCount = loopCount;
            }

        }
        //else follow trace
    }
}
