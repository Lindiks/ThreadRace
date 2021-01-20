#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <curses.h>

#include "defs.h"

int main(){
  EntityType *dorc;
  int prob;

//Initalzie nCurses library
  initNcurses();

//Seed random number generator
  srand( (unsigned)time(NULL) );

//Dynamically allocate the race global variable as a RaceInfoType structure
  race = malloc(sizeof(RaceInfoType));
  initRace();

  initScreen();

// Initalize the semaphore
  if(sem_init(&race->mutex, 0, 1)<0){ //semaphore unlocked
    printf("semaphore initialization error \n");
    exit(1);
  }

//Create runners threads
  for (int i=0; i < race->numRunners; i++){
    pthread_create(&(race->runners[i]->ent.thr), NULL, goRunner, race->runners[i]);
  }


//Race loop! loop until a winner is delcared or both runners are dead
  while(strcmp(race->winner, "")==0){
    if(race->runners[0]->dead == 1 && race->runners[1]->dead == 1){
      break;
    }
    prob = randm(10);
//Create dorcs with a 30% probability
    if(prob <= 2){ //30%
      initDorc(&dorc);
//Create dorcs thread
      pthread_create(&(race->dorcs[race->numDorcs-1]->thr), NULL, goDorc, race->dorcs[race->numDorcs-1]);
    }
    usleep(250000);

  }//end Race loop

//wait for threads to finish
  pthread_join(race->runners[0]->ent.thr, NULL);
  pthread_join(race->runners[1]->ent.thr, NULL);

//Cancel all dorc threads
  for(int i =0; i<race->numDorcs; i++){
    pthread_cancel(race->dorcs[i]->thr);
  }

  announceWinner();

  initScreen();
  cleanup();
  cleanupNcurses(31);
return 0;
}
