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
    int last_floor; //Will never be -1
    int current_floor;
    int last_direction; //Will never be 0
    int current_direction; //0 = stopp, 1 = opp, -1 = ned

    //Door state
    bool door_open; 
    bool obstruction;

    //Emergency stopreturn elevator;
    bool stop;

    //Door timer
    clock_t start_time;

};

bool check_prioritized_exceptions(int current_floor, struct Active_orders orders, int current_direction){
    bool exception = true;
    if (current_direction == -1) { //Check lower exceptions
        switch (current_floor){
            case 2: {
                if (orders.TWO_UP || orders.ONE_UP){
                    exception = false;
                }
            }
            case 1: {
                if (orders.ONE_UP){
                    exception = false;
                }
            }
        }
    } else if (current_direction == 1){ //Check higher exceptions
        switch (current_floor){
            case 1: {
                if (orders.THREE_DOWN || orders.FOUR_DOWN){
                    exception = false;
                }
            }
            case 2: {
                if (orders.FOUR_DOWN){
                    exception = false;
                }
            }
        }
    }
    return exception;
}

bool check_no_more_orders(int current_floor, struct Active_orders orders, int current_direction){
    bool exception = false;
    if (current_direction == -1){
        switch (current_floor){
            case 2: {
                if (
                    !orders.THREE_DOWN &&
                    !orders.TWO_DOWN &&
                    !orders.PANEL_TWO &&
                    !orders.PANEL_ONE
                ) {exception = true;}
            }
            case 1: {
                if (
                    !orders.TWO_DOWN &&
                    !orders.PANEL_ONE
                ) {exception = true;}
            }
        }
    } else if (current_direction == 1){
        switch (current_floor){
            case 1: {
                if (
                    !orders.THREE_UP &&
                    !orders.TWO_UP &&
                    !orders.PANEL_FOUR &&
                    !orders.PANEL_THREE
                ) {exception = true;}
            }
            case 2: {
                if (
                    !orders.THREE_UP &&
                    !orders.PANEL_FOUR
                ) {exception = true;}
            }
        }
    }
    return exception;
}

bool check_order_at_floor(int current_floor, struct Active_orders orders){
    bool order_at_floor = false;
    switch (current_floor){
        case 0:{
            if(orders.ONE_UP || orders.PANEL_ONE) {order_at_floor = true;}
        }
        case 1:{
            if(orders.TWO_UP || orders.TWO_DOWN || orders.PANEL_TWO) {order_at_floor = true;}
        }
        case 2:{
            if(orders.THREE_UP || orders.THREE_DOWN || orders.PANEL_THREE) {order_at_floor = true;}
        }
        case 3:{
            if(orders.FOUR_DOWN || orders.PANEL_FOUR) {order_at_floor = true;}
        }
    }
    return order_at_floor;
}

bool check_higher_orders(int current_floor, struct Active_orders orders){
    bool higher_order = false;
    switch (current_floor){
        case 0:{
            if( check_order_at_floor(1,orders)||
                check_order_at_floor(2,orders)||
                check_order_at_floor(3,orders)
            ) {higher_order = true;}
            break;
        }
        case 1:{
            if( check_order_at_floor(2,orders)||
                check_order_at_floor(3,orders)
            ) {higher_order = true;}
            break;
        }
        case 2:{
            if(check_order_at_floor(3,orders)) {higher_order = true;}
            break;
        }
    }
    if(higher_order == true) {
        printf("true\n");
    } else {
        printf("false");
    }
    return higher_order;
}

bool check_lower_orders(int current_floor, struct Active_orders orders) {
    bool lower_order = false;

    switch (current_floor) {
        case 3:
            if (check_order_at_floor(2, orders) ||
                check_order_at_floor(1, orders) ||
                check_order_at_floor(0, orders)) {
                lower_order = true;
            }
            break;

        case 2:
            if (check_order_at_floor(1, orders) ||
                check_order_at_floor(0, orders)) {
                lower_order = true;
            }
            break;

        case 1:
            if (check_order_at_floor(0, orders)) {
                lower_order = true;
            }
            break;
    }
    return lower_order;
}

void turn_off_all_lights_at_floor(int floor){
    elevio_buttonLamp(floor, BUTTON_HALL_UP, 0);
    elevio_buttonLamp(floor, BUTTON_HALL_DOWN, 0);
    elevio_buttonLamp(floor, BUTTON_CAB, 0);
}

struct Elevator remove_orders_from_floor(struct Elevator elevator, int floor){
    printf("Remove orders");
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
    elevator.door_open = false;
    elevator.obstruction = false;

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
                elevator.last_direction = 1;
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
            elevator.last_direction = 1;
            if( elevator.current_floor != -1 &&
                check_prioritized_exceptions(elevator.current_floor, elevator.orders, elevator.current_direction) && 
                check_no_more_orders(elevator.current_floor, elevator.orders, elevator.current_direction)){
                printf("Exception");
                elevator.last_direction = 1;
                elevator = remove_orders_from_floor(elevator, elevator.current_floor);
                elevator.current_direction = 0;
                elevator.state = LOAD_ON_AND_OFF;
            }
            else if ((elevator.orders.TWO_UP == true || elevator.orders.PANEL_TWO == true) && elevator.current_floor == 1){
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
            elevator.last_direction = -1;
            if( elevator.current_floor != -1 &&
                check_prioritized_exceptions(elevator.current_floor, elevator.orders, elevator.current_direction) && 
                check_no_more_orders(elevator.current_floor, elevator.orders, elevator.current_direction)){
                elevator.last_direction = 1;
                elevator = remove_orders_from_floor(elevator, elevator.current_floor);
                elevator.current_direction = 0;
                elevator.state = LOAD_ON_AND_OFF;
            } 
            else if ((elevator.orders.PANEL_ONE == true || elevator.orders.ONE_UP == true) && elevator.current_floor == 0){
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
        
        case LOAD_ON_AND_OFF:{
            printf("Loanonandoff\n");
            if (!elevator.door_open){
                elevator.door_open = true;
                elevator.start_time = clock();
                printf("start klokke \n");
            }
            if (elevator.obstruction){
                elevator.door_open = true;
                elevator.start_time = clock();
                printf("Obstruction \n");
            }
            if (elevator.door_open){
                printf("Start time: %.2f, Current time: %.2f\n",
                    (double)elevator.start_time*100 / CLOCKS_PER_SEC,
                    (double)clock()*100 / CLOCKS_PER_SEC);
             
                if ((((float)clock() - elevator.start_time)*100/CLOCKS_PER_SEC) > 3){
//                    printf("3 sekunder har gått \n");
//                    printf("Last direction %d", elevator.last_direction);

                    elevator.door_open = false;

                    if(elevator.last_direction == 1){
                        if(check_higher_orders(elevator.current_floor, elevator.orders)){
                            elevator.state = MOVING_UP;
                            elevator.current_direction = 1;
                            remove_orders_from_floor(elevator, elevator.current_floor);

                        } else if(check_lower_orders(elevator.current_floor, elevator.orders)){
                            elevator.state = MOVING_DOWN;
                            elevator.current_direction = -1;
                            remove_orders_from_floor(elevator, elevator.current_floor);

                        } else if(check_order_at_floor(elevator.current_floor, elevator.orders)){
                            elevator.door_open = true;

                        } else {
                            elevator.state = WAIT_FOR_ORDER;
                            remove_orders_from_floor(elevator, elevator.current_floor);
                        }
                    }
                    else if(elevator.last_direction == -1){
                        if(check_lower_orders(elevator.current_floor, elevator.orders)){
                            elevator.state = MOVING_DOWN;
                            remove_orders_from_floor(elevator, elevator.current_floor);
                            elevator.current_direction = -1;

                        } else if(check_higher_orders(elevator.current_floor, elevator.orders)){
                            elevator.state = MOVING_UP;
                            elevator.current_direction = 1;
                            remove_orders_from_floor(elevator, elevator.current_floor);

                        } else if(check_order_at_floor(elevator.current_floor, elevator.orders)){
                            elevator.door_open = true;
                        } else {
                            elevator.state = WAIT_FOR_ORDER;
                            remove_orders_from_floor(elevator, elevator.current_floor);
                        }
                    }
                }
            }
        break;
        }

        case EMERGENCY_STOP:{
            printf("emergency stop");
            elevator.current_direction = 0;
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
            elevator.door_open = false;
            if (elevio_stopButton() == false){



                elevator.state = WAIT_FOR_ORDER;
            }
            break;
        }

        default: {
            break;
        }
    }
    return elevator;
}


struct Elevator read_input(struct Elevator elevator){
    //Read floor data
    elevator.current_floor = elevio_floorSensor(); //Updates real time floor data including -1
    if (elevio_floorSensor() != -1) {
        elevator.last_floor = elevio_floorSensor(); //Updates last_floor if floor is defined
    } 

    //Check for new orders
    if ((elevio_stopButton() == false) && (elevator.state != INITIALIZING)){
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
    } else if ((elevio_stopButton() == true) && (elevator.state != INITIALIZING)) {
        elevator.state = EMERGENCY_STOP;
    }

    //Read door realted
    elevator.obstruction = elevio_obstruction();
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

    //Update floor indicator
    elevio_floorIndicator(elevator.last_floor);

    //Door communication
    elevio_doorOpenLamp(elevator.door_open);
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


    //    if(elevio_stopButton() && !(elevator.state == INITIALIZING)){
    //        elevio_motorDirection(DIRN_STOP);
    //        running = 0;
    //    }
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