#include "msp.h"
#include "Clock.h"
#include "init.h"
#include <stdio.h>

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
    int directNumber;     
    Node* node;            
    int visited;          
} Direction;

typedef struct Node {
    int x;
    int y;
    int id;                  
    int direction_count;      
    Direction directions[4]; 
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
        //IR���� Ű��
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

        //��� -> ����
        if(sensor4 && sensor5){
            front();
            systick_wait1s();
        }
        else{
            move(0, 0);
            systick_wait1s();
        }

        //IR���� ����
        P5->OUT &= ~0x08;
        P9->OUT &= ~0x04;
        Clock_Delay1ms(10);
    }
}
