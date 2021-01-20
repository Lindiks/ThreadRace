#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <curses.h>

#include "defs.h"


//PURPOSE: To initalize the default state of the screen
void initScreen(){
  scrPrt("------------------------",1,0);

  scrPrt("Health:  T   H ", HEALTH_ROW, HEALTH_COL);
  //  displayHealth();

  for (int i =0; i<race->numRunners; i++){
    scrPrt(race->runners[i]->ent.avatar, race->runners[i]->ent.currPos, race->runners[i]->ent.path);
  }

  scrPrt("------------------------",START_POS+1,0);
}


//PURPOSE: To initalize the starting state of the race
void initRace(){
  RunnerType *r;

//initalize the status row to row 4
  race->statusRow = STATUS_ROW;
//initalize the winner to an empty string
  strcpy((race)->winner, "");

//Initalize runners
  initRunner("Timmy", PATH_1, &r);
  initRunner("Harold", PATH_2, &r);
}



//PURPOSE: Initialize runner
//INPUT: rName - runners name, colNum - column this runner will be in, r - double pointer to runner
void initRunner(char *rName, int colNum, RunnerType **r){

  *r = malloc(sizeof(RunnerType));
  char aSymbol;
  aSymbol = rName[0];

  //initalize the runner
  strcpy((*r)->name, rName);
  strcpy((*r)->ent.avatar, &aSymbol);
  (*r)->health = 50;
  (*r)->ent.path = colNum;
  (*r)->dead = 0;
  (*r)->ent.currPos = START_POS;

  addRunner(*r);
}

//PURPOSE: Adds runner to the RaceInfoType 'runner' array
//INPUT: r - pointer to runner
void addRunner(RunnerType *r){
  race->runners[race->numRunners] = r;
  race->numRunners++;
}

//PURPOSE: Initalize dorc
//INPUT: d - double pointer to dorc
void initDorc(EntityType **d){

  *d = malloc(sizeof(EntityType));

  strcpy((*d)->avatar, "d");

  (*d)->currPos= END_POS;
  (*d)->path = computeCol((*d)->path);

  addDorc(*d);

}

//PURPOSE: Adds dorc to the RaceInfoType 'dorc' array
//INPUT: d - pointer to dorc
void addDorc(EntityType *d){
  race->dorcs[race->numDorcs] = d;
  race->numDorcs++;
}


//PURPOSE: Compute which column the dorc will be placed in
//INPUT: curCol - the current column the dorc occupies
//RETURN: the new Column
int computeCol(int curCol){
  int prob;
  int midCol = (PATH_1 + PATH_2)/2;
  //with equal probability, the dorc may be placed either in the same column as:
  // the tortoise, or in the same column as the hare,
  // or in the column exactly half-way between the two
  prob = randm(3);

//NOTE: Columns 11 & 13 are NOT valid
//1: 2 columns to the LEFT of the current column
  if(prob == 0){
    curCol = curCol - 2;   //outcome: 10 or 12

    if(curCol < PATH_1){
      curCol = PATH_1;
    }
  }

//2: 2 columns to the RIGHT of the current column
  if(prob == 1){
    curCol = curCol + 2; //outcome: 12, 14

    if(PATH_2 < curCol){
      curCol = PATH_2;
    }
  }
//3: same column
  if(prob == 2){
    curCol = curCol;
  }
//4: invalid column: reset to the closest of the 3 given options (Tortise, hare, middle)
  if(!(curCol==PATH_1) || !(curCol==PATH_2) || !(curCol==midCol)){
    if(prob == 0){
      curCol = PATH_1;
    }
    if(prob == 1){
      curCol = PATH_2;
    }
    if(prob == 2){
      curCol = midCol;
    }
  }
  return curCol;
}


//PURPOSE: Free and deallocate memory
void cleanup(){
  for(int i =0; i<race->numRunners; i++){
    free(race->runners[i]);
  }
  for(int i =0; i<race->numDorcs; i++){
    free(race->dorcs[i]);
  }
  return;
}
