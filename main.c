#include "msp.h"
#include "Clock.h"
#include "init.h"

#define leftDir 0
#define rightDir 1
#define frontDir 2
#define backDir 3

int sensor1 = 0;
int sensor2 = 0;
int sensor3 = 0;
int sensor4 = 0;
int sensor5 = 0;
int sensor6 = 0;
int sensor7 = 0;
int sensor8 = 0;

void loadSensor() {
   P7->DIR = 0xFF;
   P7->OUT = 0xFF;
   Clock_Delay1us(1000);
   P7->DIR = 0x00;
   Clock_Delay1us(1000);

   sensor1 = P7->IN & 0x01;
   sensor2 = P7->IN & 0x02;
   sensor3 = P7->IN & 0x04;
   sensor4 = P7->IN & 0x08;
   sensor5 = P7->IN & 0x10;
   sensor6 = P7->IN & 0x20;
   sensor7 = P7->IN & 0x40;
   sensor8 = P7->IN & 0x80;
}

int direction(int duty) {
    loadSensor();
    int res = 0;
    if (sensor6 && sensor7)
        res = leftDir;
    if (sensor2 && sensor3)
        res = rightDir;
    if (sensor4 && sensor5)
        res = frontDir;
    if (!sensor1 && !sensor2 && !sensor3 && !sensor4 && !sensor5 && !sensor6 && !sensor7 && !sensor8)
        res = backDir;
    return res;
}

int main(void)
{
    motor_init();
    Clock_Init48MHz();

    uint16_t duty = 1500;
    int left = 0;
    int right = 0;

    int b = 0;
    int turnCount = 0;
    int index = 0;
    int loopCount = -1;
    int preCount = -1;
    char lst[1000];
    int i;
    for (i = 0; i<1000; i++) {
        lst[i] = 'N';
    }

    // phase 1
    while (1)
    {
        loopCount++;

        loadSensor();

        // phase 1 loop end condition
        if (b>23) {
            move(0, 0);
            break;
        }

        int way = direction(duty);

        // direction priority: left -> front -> right
        // go left
        if (way == leftDir) {
            left = 1;
            right = 0;

            move_front(duty, duty, 5);
            turnCount = 0;
            while (1) {
                move_left(duty, duty, 1);
                loadSensor();

                if (sensor4 && sensor5) {
                    move_left(duty, duty, 5);
                    break;
                }
                turnCount++;
            }

            if (turnCount>40 && loopCount-preCount>50) {
                lst[index] = 'l';
                index++;
                preCount = loopCount;
            }
        }

        //go straight
        else if (way == frontDir) {
            if (loopCount-preCount>50) {
                loadSensor();
                if ((sensor2 && sensor3 && sensor4 && sensor5) || (sensor4 && sensor5 && sensor6 && sensor7)) {
                    lst[index] = 's';
                    index++;
                    preCount = loopCount;
                }
            }
            move_front(duty, duty, 5);
        }
        //go right
        else if (way == rightDir) {
            right = 1;
            left = 0;
            move_front(duty, duty, 5);
            turnCount = 0;
            while (1) {
                move_right(duty, duty, 1);
                loadSensor();
                if (sensor4 && sensor5) {
                    move_right(duty, duty, 5);
                    break;
                }
                turnCount++;
            }
            if (turnCount>40  && loopCount-preCount>50) {
                lst[index] = 'r';
                index++;
                preCount = loopCount;
            }
        }
        else if ((left==1) && (way == backDir)) {
            move_front(duty, duty, 5);
            turnCount = 0;
            while (1) {
                move_left(duty, duty, 1);
                loadSensor();
                if (sensor4 && sensor5) {
                    move_left(duty, duty, 3);
                    break;
                }
                turnCount++;
            }
            if (turnCount>800) {
                lst[index] = 'b';
                index++;
                b++;
            }
            else if (turnCount>40 && loopCount-preCount>50) {
                lst[index] = 'l';
                index++;
                preCount = loopCount;
            }
        }
        else if ((right==1) && (way == backDir)) {
            move_front(duty, duty, 5);
            turnCount = 0;
            while (1) {
                move_right(duty, duty, 1);
                loadSensor();
                if (sensor4 && sensor5) {
                    move_right(duty, duty, 3);
                    break;
                }
                turnCount++;
            }
            if (turnCount>800) {
                lst[index] = 'b';
                index++;
                b++;
            }
            else if (turnCount>40 && loopCount-preCount>50) {
                lst[index] = 'r';
                index++;
                preCount = loopCount;
            }

        }
        //else follow trace
    }

    //phase 1.5 : parse route
    /*
    LBR = B
    LBS = R
    RBL = B
    SBL = R
    SBS = B
    LBL = S
    */
    char res[1000];
    for (i = 0; i<998; i++) {
        if (lst[i]=='l'&&lst[i+1]=='b'&&lst[i+2]=='r') {
            res[i] = 'b';
            i = i+3;
        }
        else if (lst[i]=='l'&&lst[i+1]=='b'&&lst[i+2]=='s') {
                    res[i] = 'r';
                    i = i+3;
                }
        else if (lst[i]=='r'&&lst[i+1]=='b'&&lst[i+2]=='l') {
                    res[i] = 'b';
                    i = i+3;
                }
        else if (lst[i]=='s'&&lst[i+1]=='b'&&lst[i+2]=='l') {
                    res[i] = 'r';
                    i = i+3;
                }
        else if (lst[i]=='s'&&lst[i+1]=='b'&&lst[i+2]=='s') {
                    res[i] = 'b';
                    i = i+3;
                }
        else if (lst[i]=='l'&&lst[i+1]=='b'&&lst[i+2]=='l') {
                    res[i] = 's';
                    i = i+3;
                }
        else {
            res[i] = lst[i];
        }
    }
    left = 0;
    right = 0;
    b = 0;
    turnCount = 0;
    index = 0;
    loopCount = -1;
    preCount = -1;

    Clock_Delay1ms(10000);

    //phase 2
    while (1) {
        loopCount++;

        if (loopCount-preCount>50) {
            loadSensor();
            if ((sensor2 && sensor3 && sensor4 && sensor5) || (sensor4 && sensor5 && sensor6 && sensor7)) {
                if (res[index]=='l') {
                    left = 1;
                    right = 0;
                    move_front(duty, duty, 5);
                    while (1) {
                        move_left(duty, duty, 10);
                        loadSensor();
                        if (sensor4 && sensor5) {
                            move_left(duty, duty, 5);
                            break;
                        }
                    }
                }
                else if (res[index]=='s') {
                    move_front(duty, duty, 5);
                }
                else if (res[index]=='r') {
                    right = 1;
                    left = 0;
                    move_front(duty, duty, 5);
                    while (1) {
                        move_right(duty, duty, 10);
                        loadSensor();
                        if (sensor4 && sensor5) {
                            move_right(duty, duty, 5);
                            break;
                        }
                    }
                }
                else {
                    move_front(duty, duty, 5);
                    while (1) {
                        move_left(duty, duty, 1);
                        loadSensor();
                        if (sensor4 && sensor5) {
                            move_left(duty, duty, 3);
                            break;
                        }
                    }
                }
                index++;
                preCount = loopCount;
            }
        }
        int way = direction(duty);

        //go left
        if (way == leftDir) {
            left = 1;
            right = 0;
            move_front(duty, duty, 5);
            while (1) {
                move_left(duty, duty, 1);
                loadSensor();
                if (sensor4 && sensor5) {
                    move_left(duty, duty, 5);
                    break;
                }
            }
        }
        //go straight
        else if (way == frontDir) {
            move_front(duty, duty, 5);
        }
        //go right
        else if (way == rightDir) {
            right = 1;
            left = 0;
            move_front(duty, duty, 5);
            while (1) {
                move_right(duty, duty, 1);
                loadSensor();
                if (sensor4 && sensor5) {
                    move_right(duty, duty, 5);
                    break;
                }
            }
        }
        else if ((left==1) && (way == backDir)) {
            move_front(duty, duty, 5);
            while (1) {
                move_left(duty, duty, 1);
                loadSensor();
                if (sensor4 && sensor5) {
                    move_left(duty, duty, 3);
                    break;
                }
            }
        }
        else if ((right==1) && (way == backDir)) {
            move_front(duty, duty, 5);
            while (1) {
                move_right(duty, duty, 1);
                loadSensor();
                if (sensor4 && sensor5) {
                    move_right(duty, duty, 3);
                    break;
                }
            }
        }
    }
}
