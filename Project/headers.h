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


struct PCB
{
    int currentTime;
    int processsID;
    int status;
    int arrivalTime;
    int remainingTime;
    int totalTimeRunl;

};

//This acts similar to a server, waits for an update to the process to occur
//if it does writes the required details in a file
void output()
{
    key_t queue_PUP_ID;
    int msqPUP_id, rec_PUP;

    // Get unique ID for process_update queue
    queue_PUP_ID = ftok("N", 'P'); 

    // Getting the down queue
    msqPUP_id = msgget(queue_PUP_ID, 0666 | IPC_CREAT);

    if (msqPUP_id == -1)
    {
        printf("Error message during creation of process_update queue\n");
        return;
    }

    // Remove when fully tested
    printf("Output initialized\n");

    // Create file to write output
    FILE* p_out = fopen("outputfile.txt", "w");
    if (p_out == NULL)
    {
        perror("ERROR HAS OCCURRED IN OUTPUT FILE OPENING");
        return;
    }

    struct PCB p;  // Assuming the PCB structure is defined somewhere else

    // Print file header
    fprintf(p_out, "# At \ttime x \tprocess y \tstate arr w \ttotal z \tremain y \twait k\n");
    fflush(p_out);

    while (1)
    {
        // Receive message
        rec_PUP = msgrcv(msqPUP_id, &p, sizeof(p), 0, !IPC_NOWAIT);
        if (rec_PUP == -1)
        {
            printf("Failed to receive\n");
        }
        else
        {
            // REMOVE WHEN DONE
            printf("Message received\n");

            // Assuming the PCB structure contains the following fields:
            // - int ID: Process ID
            // - int Arrival: Arrival time
            // - int TotalTime: Total time the process requires to run
            // - int TotalTimeRun: Time the process has already run
            // - int CurrentTime: Current system time when the message is processed

            int wait_time = (p.CurrentTime - p.Arrival) - p.TotalTimeRun;
            int remain_time = p.TotalTime - p.TotalTimeRun;

            // Print process information to the file
            fprintf(p_out, "At \ttime %d \tprocess %d \tstate arr %d \ttotal %d \tremain %d \twait %d\n",
                    p.CurrentTime, p.ID, p.Arrival, p.TotalTime, remain_time, wait_time);
            fflush(p_out);
        }
    }

    fclose(p_out);
}
