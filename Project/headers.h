#include <signal.h>
#include <stdio.h> //if you don't use scanf/printf change this include
#include <stdlib.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

typedef short bool;
#define true 1
#define false 1

#define SHKEY 300

// Process data structure
struct processData {
  unsigned int id;
  unsigned int arrivalTime;
  unsigned int runTime;
  unsigned int priority;
};

enum schedulingAlgorithm { SJF = 1, PHPF, RR };

// Process Data Loader
struct processData *load(char *inpFileName, int *count_processes) {
  *count_processes = 0;
  struct processData *p_arr_process;

  // Opening File and checking if its been successfully opened
  FILE *p_file = fopen(inpFileName, "r");
  if (p_file == NULL) {
    perror("ERROR HAS OCCURED IN INPUT FILE OPENINING");
    fclose(p_file);
    return NULL;
  }

  // Buffer needed to store content of line (This is needed even we dont need
  // the content while counting the number of processes)
  char buffer[256];

  // skipping past comments line
  fgets(buffer, sizeof(buffer), p_file);

  // itterating through the file to find number of processes
  while (fgets(buffer, sizeof(buffer), p_file) != NULL) {
    *count_processes += 1;
  }

  // once counting is complete we close the file to now read the file data
  fclose(p_file);

  // Dyanmic allocation of array for processes
  p_arr_process = (struct processData *)malloc(*count_processes *
                                               sizeof(struct processData));

  // Reopening input file
  p_file = fopen(inpFileName, "r");
  if (p_file == NULL) {
    perror("ERROR HAS OCCURED IN INPUT FILE OPENINING _2_");
    fclose(p_file);
    return NULL;
  }

  // skipping past comments line
  fgets(buffer, sizeof(buffer), p_file);

  // itterating through file to get data of each
  for (int i = 0; i < *count_processes; i++) {
    if (fscanf(p_file, "%d\t%d\t%d\t%d\t", &p_arr_process[i].id,
               &p_arr_process[i].arrivalTime, &p_arr_process[i].runTime,
               &p_arr_process[i].priority) != 4) {
      printf("Issue reading from file");
    };
  }

  // Close File
  fclose(p_file);

  return p_arr_process;
}

// Inter Process Communication
// Structure for Process message buffer, if any additions are need beyond the
// process data
struct processMsgBuff {
  // long mtype;
  processData process;
};

#pragma region "Clock Stuff"
///==============================
// don't mess with this variable//
int *shmaddr; //
//===============================

int getClk() { return *shmaddr; }

/*
 * All process call this function at the beginning to establish communication
 * between them and the clock module. Again, remember that the clock is only
 * emulation!
 */

void initClk() {
  int shmid = shmget(SHKEY, 4, 0444);
  while ((int)shmid == -1) {
    // Make sure that the clock exists
    printf("Wait! The clock not initialized yet!\n");
    sleep(1);
    shmid = shmget(SHKEY, 4, 0444);
  }
  shmaddr = (int *)shmat(shmid, (void *)0, 0);
}

/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of
 * simulation. It terminates the whole system and releases resources.
 */

void destroyClk(bool terminateAll) {
  shmdt(shmaddr);
  if (terminateAll) {
    killpg(getpgrp(), SIGINT);
  }
}

#pragma endregion
