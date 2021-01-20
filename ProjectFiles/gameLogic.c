#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <curses.h>

#include "defs.h"

//PURPOSE: Used by the runner thread
//INPUT: runner - pointer
void *goRunner(void* runner){ //parameter will be of runnerType
  int newpos;
  int toMove;
  int prob;
//typecast the paramenter to be a RunnerType pointer
  RunnerType* r = runner;

//loop until runner reaches the top (row 2) or runner dies
  while((r->ent.currPos > END_POS) && r->health > 0){
//Compute runners next position
    prob = randm(10);
    if(prob <=2){ //30%
      newpos = randm(3) + 1; //DOWN
      toMove = r->ent.currPos + newpos;
      if((r->ent.currPos+newpos) > START_POS){ //maybe refine this so they can only go so many steps (i.e. they're 2 steps from the line but wanna take 3, they can then take 2)

        toMove = r->ent.currPos;
        newpos = 0; //so they don't lose any health points for remaining in the same position
      }
    }else{
      newpos  = randm(4) + 1 ; //UP
      toMove = r->ent.currPos - newpos;
    }

//Check for collisions
  collide(&r, toMove);

//decrement health & check if runner is dead
  if((r->health - newpos) > 0){
    r->health = r->health - newpos;
  }else{
    reportDead(&r);
    break;
  }

//MOVE the Runner
  moveRunner(&r, toMove);

 usleep(250000);
}//END LOOP

//Display Health
//Lock semaphore
  if(sem_wait(&race->mutex) < 0){
    printf("semaphore wait error\n");
    exit(1);
  }
  displayHealth();
//Check/update the winner
  if((strcmp(race->winner, "") == 0) && (r->dead == 0)){
    strcpy(race->winner, r->name);
  }

//unlock semaphore
  if(sem_post(&race->mutex) < 0) {
      printf("semaphore post error\n");
      exit(1);
  }
}

//PURPOSE: USed by the dorc thread
//INPUT: dorc - pointer
void *goDorc(void* dorc){
  int newRow;
  int newCol;
//type cast the parameter to be an EntityType pointer
  EntityType* d = dorc;

//print dorcs inital spawn position
  moveDorc(&d, d->currPos, d->path);

//loop until dorc reaches bottom of the mountain (row 35)
  while(d->currPos < START_POS){

//Compute dorcs next position
    newRow = randm(5) + 1;
    newRow =  newRow + d->currPos;
    newCol = computeCol(d->path);

    moveDorc(&d, newRow, newCol);

    usleep(700000);
}//end loop

//Lock semaphore
  if(sem_wait(&race->mutex) < 0){
    printf("semaphore wait error\n");
    exit(1);
  }
//update screen one last time to display blank spaces in dorcs final position
  scrPrt(" ", d->currPos , d->path);

//Unlock semaphore
  if(sem_post(&race->mutex) < 0) {
    printf("semaphore post error\n");
    exit(1);
  }
}


//PURPOSE: Determines if a collision occurs or not, and updates accordingly
//INPUT: r - double pointer to runner, pos - position r wishes to move to
void collide(RunnerType** r, int pos){
  int collision;  //0 for false, 1 for true
  char cc[MAX_STR];
  char msgg[MAX_STR];


//"collision between %s and dorc"
  sprintf(cc, "%s",(*r)->name);
  strcpy(msgg, " collided");

  strcat(cc, msgg);
//Lock the semaphore
  if(sem_wait(&race->mutex) < 0){
    printf("semaphore wait error\n");
    exit(1);
  }
//iterate over all dorcs:
  for(int i = 0; i<race->numDorcs; i++){
//check for collisions
    if((pos == race->dorcs[i]->currPos) && ((*r)->ent.path == race->dorcs[i]->path)){
      collision = 1;
      break;
    }
  }
//unlock the semaphore
  if(sem_post(&race->mutex) < 0) {
    printf("semaphore post error\n");
    exit(1);
  }

  if(collision == 1){
    (*r)->health= (*r)->health - 3;
    statusUpdate(cc);
  }
}

//PURPOSE: Move the runner by updating its position
//INPUT: r - double pointer to runner, newRow - position the runner will move to
void moveRunner(RunnerType** r, int newRow){
//Lock the semaphore
  if(sem_wait(&race->mutex) < 0){
    printf("semaphore wait error\n");
    exit(1);
  }
//update screen at runners previous position to blank spaces
  scrPrt(" ", (*r)->ent.currPos , (*r)->ent.path);

//Ensures the runner does not place beyond the finish line
  if(newRow < END_POS){
    scrPrt((*r)->ent.avatar, END_POS, (*r)->ent.path);
    (*r)->ent.currPos = END_POS;
    //lock Semaphore
    if(sem_post(&race->mutex) < 0) {
      printf("semaphore post error\n");
      exit(1);
    }
    return;
  }
//Update the screen at runners NEW position to show their avatar
  scrPrt((*r)->ent.avatar, newRow, (*r)->ent.path);
  //set runners current position to the new position
  (*r)->ent.currPos = newRow;
//display runners health
  displayHealth();
//unlock the semaphore
  if(sem_post(&race->mutex) < 0) {
    printf("semaphore post error\n");
    exit(1);
  }
}


//PURPOSE: Move the dorc by updating its position
//INPUT: d - double pointer to dorc, nRow - row the dorc will move to, nCol - column the dorc will move to
void moveDorc(EntityType** d, int nRow, int nCol){
  //checks if the space is already occupied by a runner (so )
  if(nCol == PATH_1 || nCol == PATH_2){
    for(int i = 0; i<race->numRunners; i++){
      if((race->runners[i]->ent.path == nCol) && (race->runners[i]->ent.currPos == nRow)){
      //  printf("%d Space occupied by: %s ", nRow, race->runners[i]->name);
        return;
      }
    }
  }
//Lock the semaphore
  if(sem_wait(&race->mutex) < 0){
    printf("semaphore wait error\n");
    exit(1);
  }
//update screen at dorcs previous position to " " blank spaces
  scrPrt(" ", (*d)->currPos , (*d)->path);


//Ensures dorcs don't go "into the ground"
  if(nRow > START_POS){
    scrPrt((*d)->avatar, START_POS, nCol);
    (*d)->currPos = START_POS;
//Unlock semaphore
    if(sem_post(&race->mutex) < 0) {
        printf("semaphore post error\n");
        exit(1);
    }
    return;
  }
//Update the screen at dorcs NEW position to show their avatar
  scrPrt((*d)->avatar, nRow, nCol);

  (*d)->currPos = nRow;
  (*d)->path = nCol;

//lock semaphore
  if(sem_post(&race->mutex) < 0) {
      printf("semaphore post error\n");
      exit(1);
  }
}

//PURPOSE: Updates all necessary attributes to declare the runner dead
//INPUT: r - double pointer to runner
void reportDead(RunnerType** r){
  char c[MAX_STR];
  char msg[MAX_STR];
  sprintf(c, "%s",(*r)->name);
  strcpy(msg, " has died");
  strcat(c, msg);
//a dead hero's avatar is permanetly changed to "+" to mark their grave
  strcpy((*r)->ent.avatar, "+");
//dead flag is set to true (1)
  (*r)->dead = 1; //C_TRUE;
  (*r)->health = 0;
//update the status bar to say the runner has died
  statusUpdate(c);
  moveRunner(r, (*r)->ent.currPos); //Update to display grave
}

//PURPOSE: Display the runners health
void displayHealth(){
  char c[MAX_STR];

  sprintf(c ,"%2d", race->runners[0]->health);
  scrPrt(c, HEALTH_ROW+2, HEALTH_COL+8);

  sprintf(c, "%2d",race->runners[1]->health);
  scrPrt(c, HEALTH_ROW+2, HEALTH_COL+12);

   // for (int i = 0; i<race->numRunners; i++){
  //    sprintf(c ,"%2d", race->runners[0]->health);
  //    scrPrt(c, HEALTH_ROW+2, HEALTH_COL+8);
  // }
}


//PURPOSE: Update the status bar
//INPUT: msg - the message to be displayed
void statusUpdate(char* msg){
  char c[MAX_STR];
//Lock the semaphore
  if(sem_wait(&race->mutex) < 0){
    printf("semaphore wait error\n");
    exit(1);
  }

  strcpy(c,"STATUS:  ");
  strcat(c, msg);

  race->statusRow = race->statusRow+1;

  scrPrt(c, race->statusRow, STATUS_COL);

//unlock the semaphore
  if(sem_post(&race->mutex) < 0) {
    printf("semaphore post error\n");
    exit(1);
  }
}

//PURPOSE: Announce the winner of the race
void announceWinner(){
  char c[MAX_BUF];
  //Lock the semaphore
  // if(sem_wait(&race->mutex) < 0){
  //     printf("semaphore wait error\n");
  //     exit(1);
  //   }

  if(strcmp(race->winner, "")==0){
    strcpy(c,"OUTCOME:  All runners are dead");
  }else{
    strcpy(c,"WINNER:  ");
    strcat(c, race->winner);
  }
  race->statusRow = race->statusRow+1;

  scrPrt(c, race->statusRow, STATUS_COL);

  //unlock the semaphore
  // if(sem_post(&race->mutex) < 0) {
  //   printf("semaphore post error\n");
  //   exit(1);
  // }
}
