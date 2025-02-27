/**
* @file
* @brief Testing
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "driver/elevio.h"
#include <stdbool.h>

/**
* @brief Copy a list of integers from one buffer to another,
* reversing the order of the output in the process.
*
*
* @return 0 on success
* is a
*/


enum State{ 
    MOVING_DOWN,
    MOVING_UP,
    ROUTINE_STOP_DOWN,
    ROUTINE_STOP_UP,
    EMERGENCY_STOP,
    INITIALIZING,
    WAIT_FOR_ORDER,
    LOAD_ON_AND_OFF
};

struct Active_orders {
    bool ONE_UP;
    bool TWO_UP;
    bool TWO_DOWN;
    bool THREE_DOWN;
    bool THREE_UP;
    bool FOUR_DOWN;
    bool PANEL_ONE;
    bool PANEL_TWO;
    bool PANEL_THREE;
    bool PANEL_FOUR;
};


struct Elevator{
    //Tilstand
    enum State state;

    //Orders:
    struct Active_orders orders;

    //Elevator state
    int last_floor; //Will never be
    int current_floor;
    int last_direction; //Will never be 0
    int current_direction; //0 = stopp, 1 = opp, -1 = ned

    //Door state
    bool door_open; 
    bool obstruction;

    //Emergency stopreturn elevator;
    bool stop;
};

void turn_off_all_lights_at_floor(int floor){
    elevio_buttonLamp(floor, BUTTON_HALL_UP, 0);
    elevio_buttonLamp(floor, BUTTON_HALL_DOWN, 0);
    elevio_buttonLamp(floor, BUTTON_CAB, 0);
}

struct Elevator remove_orders_from_floor(struct Elevator elevator, int floor){
    if (floor == 0) {
        elevator.orders.ONE_UP = false;
        elevator.orders.PANEL_ONE = false;
    }
    else if (floor == 1) {
        elevator.orders.TWO_UP = false;
        elevator.orders.TWO_DOWN = false;
        elevator.orders.PANEL_TWO = false;
    }
    else if (floor == 2) {
        elevator.orders.THREE_UP = false;
        elevator.orders.THREE_DOWN = false;
        elevator.orders.PANEL_THREE = false;
    }
    else if (floor == 3) {
        elevator.orders.FOUR_DOWN = false;
        elevator.orders.PANEL_FOUR = false;
    }
    return elevator;
}

struct Elevator Initialize () {
    struct Elevator elevator;

    //Assign default values to orders
    elevator.orders.ONE_UP = false;
    elevator.orders.TWO_UP = false;
    elevator.orders.TWO_DOWN = false;
    elevator.orders.THREE_DOWN = false;
    elevator.orders.THREE_UP = false;
    elevator.orders.FOUR_DOWN = false;
    elevator.orders.PANEL_ONE = false;
    elevator.orders.PANEL_TWO = false;
    elevator.orders.PANEL_THREE = false;
    elevator.orders.PANEL_FOUR = false;
    
    elevator.state = INITIALIZING;

    return elevator;
}

struct Elevator elevator_logic(struct Elevator elevator){
    switch(elevator.state) {
        case INITIALIZING:{
            //Heislogikk
            if (elevator.current_floor == -1){
                elevator.current_direction = 1;
            } else {
                elevator.current_direction = 0;
                elevator.state = WAIT_FOR_ORDER;
            }
            //Dørlogikk
            break;
        }

        case WAIT_FOR_ORDER:{
            //Check what floor is ordered
            int floor_ordered = -1;
            if (elevator.orders.ONE_UP || elevator.orders.PANEL_ONE) {
                floor_ordered = 0;
            }
            else if (elevator.orders.TWO_UP || elevator.orders.TWO_DOWN || elevator.orders.PANEL_TWO) {
                floor_ordered = 1;
            }
            else if (elevator.orders.THREE_UP || elevator.orders.THREE_DOWN || elevator.orders.PANEL_THREE) {
                floor_ordered = 2;
            }
            else if (elevator.orders.FOUR_DOWN || elevator.orders.PANEL_FOUR) {
                floor_ordered = 3;
            }

            //Set new state based on ordered floor
            if (floor_ordered == -1){
                break;
            }
            else if (floor_ordered > elevator.current_floor){
                elevator.current_direction = DIRN_UP;
                elevator.state = MOVING_UP;
            } else if (floor_ordered < elevator.current_floor){
                elevator.current_direction = DIRN_DOWN;
                elevator.state = MOVING_DOWN;
            } 
            else if (floor_ordered == elevator.current_floor){
                elevator.state = LOAD_ON_AND_OFF;
                turn_off_all_lights_at_floor(floor_ordered);
                elevator = remove_orders_from_floor(elevator, floor_ordered);
            }
            break;
        }

        case MOVING_UP:{
            elevator.last_direction = MOVING_UP;
            if ((elevator.orders.TWO_UP == true || elevator.orders.PANEL_TWO == true) && elevator.current_floor == 1){
                elevator.state = LOAD_ON_AND_OFF;    
                elevator.current_direction = DIRN_STOP;
                turn_off_all_lights_at_floor(1);
                elevator = remove_orders_from_floor(elevator, 1);
            }
            else if ((elevator.orders.THREE_UP == true || elevator.orders.PANEL_THREE == true) && elevator.current_floor == 2){
                elevator.state = LOAD_ON_AND_OFF;
                elevator.current_direction = DIRN_STOP;
                turn_off_all_lights_at_floor(2);
                elevator = remove_orders_from_floor(elevator, 2);

            }
            else if ((elevator.orders.PANEL_FOUR == true || elevator.orders.FOUR_DOWN == true) && elevator.current_floor == 3){
                elevator.state = LOAD_ON_AND_OFF;
                elevator.current_direction = DIRN_STOP;
                turn_off_all_lights_at_floor(3);
                elevator = remove_orders_from_floor(elevator, 3);
            } 
            break;
        }

        case MOVING_DOWN:{
            if ((elevator.orders.PANEL_ONE == true || elevator.orders.ONE_UP == true) && elevator.current_floor == 0){
                elevator.state = LOAD_ON_AND_OFF;
                elevator.current_direction = DIRN_STOP;
                turn_off_all_lights_at_floor(0);
                elevator = remove_orders_from_floor(elevator, 0);
            } 
            else if ((elevator.orders.TWO_DOWN == true || elevator.orders.PANEL_TWO == true) && elevator.current_floor == 1){
                elevator.state = LOAD_ON_AND_OFF;                
                elevator.current_direction = DIRN_STOP;
                turn_off_all_lights_at_floor(1);
                elevator = remove_orders_from_floor(elevator, 1);
            }
            else if ((elevator.orders.THREE_DOWN == true || elevator.orders.PANEL_THREE == true) && elevator.current_floor == 2){
                elevator.state = LOAD_ON_AND_OFF;
                elevator.current_direction = DIRN_STOP;
                turn_off_all_lights_at_floor(2);
                elevator = remove_orders_from_floor(elevator, 2);
            }
            break;
        }
        default:
            break;
          // code block
    }
    return elevator;
}

struct Elevator read_input(struct Elevator elevator){
    //Read floor data
    elevator.current_floor = elevio_floorSensor(); //Updates real time floor data including -1
    if (elevio_floorSensor() != -1) {elevator.last_floor = elevio_floorSensor();} //Updates last_floor if floor is defined

    //Check for new orders
    if (elevio_callButton(0, BUTTON_CAB)) {elevator.orders.PANEL_ONE = true;}
    if (elevio_callButton(1, BUTTON_CAB)) {elevator.orders.PANEL_TWO = true;}
    if (elevio_callButton(2, BUTTON_CAB)) {elevator.orders.PANEL_THREE = true;}
    if (elevio_callButton(3, BUTTON_CAB)) {elevator.orders.PANEL_FOUR = true;}
    
    if (elevio_callButton(0, BUTTON_HALL_UP)) {elevator.orders.ONE_UP = true;}
    if (elevio_callButton(1, BUTTON_HALL_DOWN)) {elevator.orders.TWO_DOWN = true;}
    if (elevio_callButton(1, BUTTON_HALL_UP)) {elevator.orders.TWO_UP = true;}
    if (elevio_callButton(2, BUTTON_HALL_DOWN)) {elevator.orders.THREE_DOWN = true;}
    if (elevio_callButton(2, BUTTON_HALL_UP)) {elevator.orders.THREE_UP = true;}
    if (elevio_callButton(3, BUTTON_HALL_DOWN)) {elevator.orders.FOUR_DOWN = true;}
    
    return elevator;
}

void send_actions_to_elev(struct Elevator elevator){
    elevio_motorDirection(elevator.current_direction);

    //Update order lights
    elevio_buttonLamp(0, BUTTON_HALL_UP, elevator.orders.ONE_UP);
    elevio_buttonLamp(1, BUTTON_HALL_DOWN, elevator.orders.TWO_DOWN);
    elevio_buttonLamp(1, BUTTON_HALL_UP, elevator.orders.TWO_UP);
    elevio_buttonLamp(2, BUTTON_HALL_DOWN, elevator.orders.THREE_DOWN);
    elevio_buttonLamp(2, BUTTON_HALL_UP, elevator.orders.THREE_UP);
    elevio_buttonLamp(3, BUTTON_HALL_DOWN, elevator.orders.FOUR_DOWN);

    elevio_buttonLamp(0, BUTTON_CAB, elevator.orders.PANEL_ONE);
    elevio_buttonLamp(1, BUTTON_CAB, elevator.orders.PANEL_TWO);
    elevio_buttonLamp(2, BUTTON_CAB, elevator.orders.PANEL_THREE);
    elevio_buttonLamp(3, BUTTON_CAB, elevator.orders.PANEL_FOUR);
}

int main(){
    elevio_init();
    int running = 1;
    struct Elevator elevator = Initialize();

    while(running){
        elevator = read_input(elevator);
        elevator = elevator_logic(elevator);
        send_actions_to_elev(elevator);
        //elevator = getInput(elevator); //Update elevator state based on inputs
        //next_move = elevator_logic(elevator); //Find out what the elevator should do next
        //execute(next_move); //Get the physical elevator to execute the desired order


        if(elevio_stopButton() && !(elevator.state == INITIALIZING)){
            elevio_motorDirection(DIRN_STOP);
            running = 0;
        }
    }



    /*
    printf("=== Example Program ===\n");    elevio_buttonLamp(1, BUTTON_HALL_DOWN, elevator.orders.TWO_DOWN);
    elevio_buttonLamp(1, BUTTON_HALL_UP, elevator.orders.TWO_UP);
    elevio_buttonLamp(2, BUTTON_HALL_DOWN, elevator.orders.THREE_DOWN);
    elevio_buttonLamp(2, BUTTON_HALL_UP, elevator.orders.THREE_UP);
    elevio_buttonLamp(3, BUTTON_HALL_DOWN, elevator.orders.FOUR_DOWN);

    elevio_buttonLamp(0, BUTTON_CAB, elevator.orders.PANEL_ONE);
    elevio_buttonLamp(1, BUTTON_CAB, elevator.orders.PANEL_TWO);
    elevio_buttonLamp(2, BUTTON_CAB, elevator.orders.PANEL_THREE);
    elevio_buttonLamp(3, BUTTON_CAB, elevator.orders.PANEL_FOUR);
    printf("Press the stop button on the elevator panel to exit\n");

    elevio_motorDirection(DIRN_UP);
    while(1){
        int floor = elevio_floorSensor();
        printf("Tallet er%d\n", floor);

        if(floor == 0){
            elevio_motorDirectio/Door staten(DIRN_UP);
        }        if(elevio_stopButton(){
            elevio_motorDirection(DIRN_STOP);
            break;
        }

        if(floor == N_FLOORS-1){
            elevio_motorDirection(DIRN_DOWN);
        }
MOWING_UP

        for(int f = 0; f < N_FLOORS; f++){
            for(int b = 0; b < N_BUTTONS; b++){
                int btnPressed = elevio_callButton(f, b);
                elevio_buttonLamp(f, b, btnPressed);
            }
        }        if(elevio_stopButton(){
            elevio_motorDirection(DIRN_STOP);
            break;
        }/Door state

        if(elevio_obstruction()){//BUTTON_HALL_UP      = 0,
//BUTTON_HALL_DOWN    = 1,
//BUTTON_CAB          = 2
            elevio_stopLamp(1);
            elevio_buttonLamp(1,BUTTON_CAB
            sprintf(str, "%d", floor);,1);
            elevio_buttonLamp(0,BUTTON_HALL_UP,1);
        } else {
            elevio_stopLamp(0);
            elevio_buttonLamp(1,BUTTON_CAB,0);
            elevio_buttonLamp(0,BUTTON_HALL_UP,0);
        }
                if(elevio_stopButton(){
            elevio_motorDirection(DIRN_STOP);
            break;
        }
        if(elevio_stopButton(){
            elevio_motorDirection(DIRN_STOP);
            break;
        }
           printf("=== Example Program ===\n");

    nanosleep(&(struct timespec){0, 20*1000*1000}, NULL);
    }

    */

    return 0;
}