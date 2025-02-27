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
    MOWING_UP,
    ROUTINE_STOP_DOWN,
    ROUTINE_STOP_UP,
    EMERGENCY_STOP,
    INITIALIZING,
    WAIT_FOR_ORDER
};

struct Elevator{
    //Tilstand
    enum State state;

    //Orders:
    //struct Activated_orders;

    //Elevator state
    int last_floor;
    int current_floor;
    int direction; //0 = stopp, 1 = opp, -1 = ned

    //Door state
    bool door_open; 
    bool obstruction;

    //Emergency stopreturn elevator;
    bool stop;
};

struct Elevator Initialize () {
    struct Elevator elevator;
    elevator.state = INITIALIZING;
    //elevator.door_open = false;

    //Sjekk startetasje, gå opp til etasje om ugyldig
    //elevator.floor = elevio_floorSensor();
    return elevator;
}

void Elevator_logic(struct Elevator elevator){
    switch(elevator.state) {
        case INITIALIZING:
            //Heislogikk
            if (elevator.current_floor == -1){
                elevator.direction = 1;
            } else {
                elevator.direction == 0;
                elevator.state == WAIT_FOR_ORDER;
            }
            //Dørlogikk
            
            break;

        default:
          // code block
    } 
};

struct Elevator read_input(){

    return elevator
}

int main(){
    elevio_init();
    int running = 1;
    Elevator elevator = Initialize();

    while(running){
        //elevator = getInput(elevator); //Update elevator state based on inputs
        //next_move = elevator_logic(elevator); //Find out what the elevator should do next
        //execute(next_move); //Get the physical elevator to execute the desired order


        if(elevio_stopButton()){
            elevio_motorDirection(DIRN_STOP);
            running = 0;
        }
    }



    /*
    printf("=== Example Program ===\n");
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


        for(int f = 0; f < N_FLOORS; f++){
            for(int b = 0; b < N_BUTTONS; b++){
                int btnPressed = elevio_callButton(f, b);
                elevio_buttonLamp(f, b, btnPressed);
            }
        }        if(elevio_stopButton(){
            elevio_motorDirection(DIRN_STOP);
            break;
        }/Door state

        if(elevio_obstruction()){
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
