#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>

//define number of runways and planes
#define NUM_RUNS 3
#define PLANES 5

void* planeFlight(void* arg);

//initialize global semaphores
sem_t runways[NUM_RUNS];
sem_t tower;

//structure of plane information
typedef struct {
        int number;
        int runway;
        int land_takeoff;
}Plane;


int main(){

        pthread_t planes[PLANES];

        Plane plane[PLANES];
  
        //initialize runways and tower
        for(int i = 0 ; i < NUM_RUNS ; i ++){
                sem_init(&runways[i], 0, 1);
        }
        sem_init(&tower, 0, 1);

        //Create planes, and assign information
        for(int i = 0 ; i < PLANES ; i++){
                plane[i].number = i + 1;
                plane[i].runway = -1;
                plane[i].land_takeoff = i%2; // if 0, takeoff. If 1, land.
                pthread_create(&planes[i], NULL, planeFlight, &plane[i]);
        }
        //join all the threads
        for(int i = 0 ; i < PLANES ; i++){
                pthread_join(planes[i], NULL);
        }

        //end semaphores
        for(int i = 0 ; i < NUM_RUNS ; i++){
                sem_destroy(&runways[i]);
        }
        sem_destroy(&tower);

        return 0;
}


void* planeFlight(void* arg){

        Plane* Airplane = (Plane*) arg;

        //start waiting for tower
        printf("Airplane %d waiting for tower.\n", Airplane -> number);

        sem_wait(&tower);

        printf("Airplane %d: Waiting on runway.\n", Airplane -> number);

        //finding available runway
        for(int i = 0 ; i<NUM_RUNS ; i++){
                //wait on runway
                if(sem_trywait(&runways[i]) == 0){
                        Airplane -> runway = i + 1;
                        break;
                }
        }
        //If the plane runway is available, assign plane to it and begin either take off or landing based on it's need.
        if(Airplane -> runway != -1){
                printf("Runway available. Plane %d, proceed to runway: %d.\n", Airplane -> number, Airplane -> runway);

                //plane is taking off
                if(Airplane -> land_takeoff == 0){
                        printf("Plane %d is taking off at runway %d.\n", Airplane -> number, Airplane -> runway);
                        sleep(2);
                        printf("Plane %d has taken off.\n", Airplane -> number);
                }else{ //plane is landing
                        printf("Plane %d is landing at runway %d.\n", Airplane -> number, Airplane -> runway);
                        sleep(2);
                        printf("Plane %d has landed.\n", Airplane -> number);
                }
                //free the runway for the next traffic
                sem_post(&runways[Airplane -> runway]);
        }else{
                //error handle unavailable runways
                printf("Plane %d, there are currently no available runways. Releasing tower to other dispatch.\n", Airplane -> number);
        }
        //release the tower from this plane
        sem_post(&tower);
}
