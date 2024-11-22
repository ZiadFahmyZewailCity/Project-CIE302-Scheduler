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

//This is the code related to the output file still needs to be integrated with the rest of the code



//WARNING THIS IS A GLOBAL VARIABLE UNTIL A WAY FOR THE SIGNAL AND HANDLER AND OUTPUT FUNTION TO 
//SEE THIS FILE VARIABLE
FILE* p_out;

//WE SHOULD PROBABLY PUT ALL SIGNALS HANDLERS IN A HEADER FILE
void signalHandler_outputfunctionEXIT(int signal)
{
    fclose(p_out);
    exit(0);
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


//This acts similar to a server, waits for an update to the process to occur
//if it does writes the required details in a file
void output()
{
    //WARNING THE SIGNAL WHICH SHOULD BE MAPPED TO OUT SIGNAL HANDLER SHOULD BE A CUSTOM ONE SIGINT
    //IS A PLACEHOLDER FOR TESTING
    signal(SIGUSR1,signalHandler_outputfunctionEXIT);
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

    p_out = fopen("check.txt", "w");
    if (p_out == NULL)
    {
        perror("ERROR HAS OCCURRED IN OUTPUT FILE OPENING");
        return;
    }

    struct message p;  // Assuming the PCB structure is defined somewhere else

    // Print file header
    fprintf(p_out, "# At \ttime x \tprocess y \tstate arr w \ttotal z \tremain y \twait k\n");
 
    while (1)
    {
        // Receive message
        rec_PUP = msgrcv(msqPUP_id, &p, sizeof(p.sent_PCB), 0,0);
        if (rec_PUP == -1)
        {
            printf("Failed to receive\n");
        }
        else
        {
            // REMOVE WHEN DONE
            printf("Message received\n");
            //Current time should be found from the clock
            int wait_time = (p.sent_PCB.currentTime - p.sent_PCB.arrivalTime) - p.sent_PCB.totalTimeRun;
            int remain_time = p.sent_PCB.totalTime - p.sent_PCB.totalTimeRun;


            
            // Print process information to the file
            
            fprintf(p_out, "At \ttime %d \tprocess %d \tstate arr %d \ttotal %d \tremain %d \twait %d\n",
                   p.sent_PCB.currentTime, p.sent_PCB.processsID, p.sent_PCB.arrivalTime, p.sent_PCB.totalTime, remain_time, wait_time);
        }
    }

    fclose(p_out);
}

//Version of output function that doesnt work like a sever

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
    int wait_time = (inpPCB.currentTime - inpPCB..arrivalTime) - inpPCB.totalTimeRun;
    int remain_time = inpPCB.totalTime - inpPCB.totalTimeRun;

    fprintf(p_out, "At \ttime %d \tprocess %d \tstate arr %d \ttotal %d \tremain %d \twait %d\n"
    ,inpPCB.currentTime, inpPCB.processsID, inpPCB.arrivalTime, inpPCB.totalTime, remain_time, wait_time);

    fclose(p_out);
}
