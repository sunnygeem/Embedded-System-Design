#include "msp.h"
#include "Clock.h"
#include "init.h"

#define LEFT  'l'
#define RIGHT 'r'
#define STRAIGHT 's'
#define BACK 'b'

#define leftDir 0
#define rightDir 1
#define frontDir 2
#define backDir 3

char path[1000];     // memorized path in phase1
char optimized[1000]; // optimzed path throught phase2. will used for phase3
int path_index = 0;
int optimized_index = 0;

int sensor[8];

void loadSensor() {
    P7->DIR = 0xFF;
    P7->OUT = 0xFF;
    Clock_Delay1us(1000);
    P7->DIR = 0x00;
    Clock_Delay1us(1000);

    for (int i = 0; i < 8; i++) {
        sensor[i] = (P7->IN >> i) & 1;
    }
}

int direction() {
    loadSensor();
    if (sensor[5] && sensor[6]) return leftDir;
    if (sensor[1] && sensor[2]) return rightDir;
    if (sensor[3] && sensor[4]) return frontDir;
    for (int i = 0; i < 8; i++) if (sensor[i]) return -1;
    return backDir;
}

void memorize_phase(int duty) {
    int loopCount = -1, preCount = -1, turnCount = 0;
    int left = 0, right = 0, b = 0;

    while (1) {
        loopCount++;
        if (b > 23) {
            move(0, 0);
            break;
        }

        int dir = direction();

        if (dir == leftDir) {
            left = 1; right = 0;
            move_front(duty, duty, 5);
            turnCount = 0;
            while (1) {
                move_left(duty, duty, 1);
                loadSensor();
                if (sensor[3] && sensor[4]) break;
                turnCount++;
            }
            if (turnCount > 40 && loopCount - preCount > 50) {
                path[path_index++] = LEFT;
                preCount = loopCount;
            }
        } else if (dir == frontDir) {
            if (loopCount - preCount > 50 && ((sensor[1] && sensor[2] && sensor[3] && sensor[4]) || (sensor[3] && sensor[4] && sensor[5] && sensor[6]))) {
                path[path_index++] = STRAIGHT;
                preCount = loopCount;
            }
            move_front(duty, duty, 5);
        } else if (dir == rightDir) {
            right = 1; left = 0;
            move_front(duty, duty, 5);
            turnCount = 0;
            while (1) {
                move_right(duty, duty, 1);
                loadSensor();
                if (sensor[3] && sensor[4]) break;
                turnCount++;
            }
            if (turnCount > 40 && loopCount - preCount > 50) {
                path[path_index++] = RIGHT;
                preCount = loopCount;
            }
        } else if ((left || right) && dir == backDir) {
            move_front(duty, duty, 5);
            turnCount = 0;
            if (left) while (1) {
                move_left(duty, duty, 1);
                loadSensor();
                if (sensor[3] && sensor[4]) break;
                turnCount++;
            } else while (1) {
                move_right(duty, duty, 1);
                loadSensor();
                if (sensor[3] && sensor[4]) break;
                turnCount++;
            }
            if (turnCount > 800) {
                path[path_index++] = BACK;
                b++;
            } else if (turnCount > 40 && loopCount - preCount > 50) {
                path[path_index++] = (left ? LEFT : RIGHT);
                preCount = loopCount;
            }
        }
    }
}

void optimize_path() {
    int i;
    for (i = 0; i < path_index - 2; i++) {
        if (path[i] == LEFT && path[i + 1] == BACK && path[i + 2] == RIGHT) {
            optimized[optimized_index++] = BACK;
            i += 2;
        }
        else if (path[i] == LEFT && path[i + 1] == BACK && path[i + 2] == STRAIGHT) {
            optimized[optimized_index++] = RIGHT;
            i += 2;
        }
        else if (path[i] == RIGHT && path[i + 1] == BACK && path[i + 2] == LEFT) {
            optimized[optimized_index++] = BACK;
            i += 2;
        }
        else if (path[i] == STRAIGHT && path[i + 1] == BACK && path[i + 2] == LEFT) {
            optimized[optimized_index++] = RIGHT;
            i += 2;
        }
        else if (path[i] == STRAIGHT && path[i + 1] == BACK && path[i + 2] == STRAIGHT) {
            optimized[optimized_index++] = BACK;
            i += 2;
        }
        else if (path[i] == LEFT && path[i + 1] == BACK && path[i + 2] == LEFT) {
            optimized[optimized_index++] = STRAIGHT;
            i += 2;
        }
        else {
            optimized[optimized_index++] = path[i];
        }
    }
    
    while (i < path_index) {
        optimized[optimized_index++] = path[i++];
    }
}

void follow_path(const char* route, int length, int duty) {
    int loopCount = -1, preCount = -1, index = 0;
    while (index < length) {
        loopCount++;
        if (loopCount - preCount > 50) {
            loadSensor();
            if ((sensor[1] && sensor[2] && sensor[3] && sensor[4]) || (sensor[3] && sensor[4] && sensor[5] && sensor[6])) {
                char dir = route[index++];
                if (dir == LEFT) {
                    move_front(duty, duty, 5);
                    while (1) {
                        move_left(duty, duty, 10);
                        loadSensor()

                        if (sensor[3] && sensor[4]) break;
                    }
                } else if (dir == STRAIGHT) {
                    move_front(duty, duty, 5);
                } else if (dir == RIGHT) {
                    move_front(duty, duty, 5);
                    while (1) {
                        move_right(duty, duty, 10);
                        loadSensor();

                        if (sensor[3] && sensor[4]) break; }
                } else if (dir == BACK) {
                    while (1) {
                        move_left(duty, duty, 1); 
                        loadSensor();

                        if (sensor[3] && sensor[4]) break; }
                }
                preCount = loopCount;
            }
        }
    }
}

int main(void) {
    motor_init();
    Clock_Init48MHz();
    uint16_t duty = 1500;

    // phase 1
    memorize_phase(duty);    
    // phase 2
    follow_path(path, path_index, duty); // follow memorized path
    optimize_path();          // optimize route for phase3
    Clock_Delay1ms(10000);
    // phase 3
    follow_path(optimized, optimized_index, duty); // fast escape

    return 0;
}
