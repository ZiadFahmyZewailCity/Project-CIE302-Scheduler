#include <stdio.h>      //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

typedef short bool;
#define true 1
#define false 1

#define SHKEY 300


///==============================
//don't mess with this variable//
int * shmaddr;                 //
//===============================



int getClk()
{
    return *shmaddr;
}


/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *) shmat(shmid, (void *)0, 0);
}


/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}


//PCB IS A PLACEHOLDER STRUCT 
struct PCB
{
    int currentTime;
    int totalTime;
    int processsID;
    int status;
    int arrivalTime;
    int remainingTime;
    int totalTimeRun;
};

struct message
{
    int messageType;
    struct PCB sent_PCB;
};


//p_out = fopen("check.txt", "w");
//fprintf(p_out, "# At \ttime x \tprocess y \tstate arr w \ttotal z \tremain y \twait k\n");
//fclose(p_out);
void output(struct PCB inpPCB)
{
    p_out = fopen("check.txt", "a");
    if (p_out == NULL)
    {
        perror("ERROR HAS OCCURRED IN OUTPUT FILE OPENING");
        return;
    }
 
        
    int wait_time = (inpPCB.currentTime - inpPCB.arrivalTime) - inpPCB.totalTimeRun;
    int remain_time = inpPCB.totalTime - inpPCB.totalTimeRun;

    //This checks if the update is that the process is terminated to print the extra parameters, 
    //number should be equal to the enum of terminated status
    if(inpPCB.status == 5)
    {
        int turnAround = inpPCB.currentTime-inpPCB.arrivalTime;
        int weightedTurnAround = (inpPCB.currentTime-inpPCB.arrivalTime)/inpPCB.totalTimeRun;

        fprintf(p_out, "At \ttime %d \tprocess %d \tstate arr %d \ttotal %d \tremain %d \twait %d\n" \tTA %d \tWTA %d,
        ,inpPCB.currentTime, inpPCB.processsID, inpPCB.arrivalTime, inpPCB.totalTime, remain_time, wait_time,turnAround,weightedTurnAround);
    }
    else
    {
        fprintf(p_out, "At \ttime %d \tprocess %d \tstate arr %d \ttotal %d \tremain %d \twait %d\n", 
        inpPCB.currentTime, inpPCB.processsID, inpPCB.arrivalTime, inpPCB.totalTime, remain_time, wait_time);

    }

    fclose(p_out);
}
    