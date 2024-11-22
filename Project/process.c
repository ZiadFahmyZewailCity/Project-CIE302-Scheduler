#include "headers.h"
#include <stdio.h>
#include <sys/types.h>

#define RUN_TIME atoi(argv[1])
#define PROCESS_ID atoi(argv[2])

struct processFinalInfo {
    uint id;
    uint msgType;
    uint startTime;
    uint runTime;
    uint remainingTime;
    uint finishTime;
};

/* Modify this file as needed*/
int remainingTime;
int termMsgid;


void stopProcess(int signum);

struct processFinalInfo processInfo;
int main(int agrc, char * argv[])
{
    key_t termKey = ftok("Terminating_Processes",2);
    termMsgid = msgget(termKey, 0666);
    signal(SIGUSR1, stopProcess); //use SIGUSR1 as a signal for stopping 
    initClk();

    processInfo.msgType = 1;
    processInfo.startTime = getClk();
    processInfo.runTime = RUN_TIME;
    processInfo.id = PROCESS_ID;

    int x = getClk();
    remainingTime = RUN_TIME;
    while (remainingTime > 0)
    {
        if (getClk() != x){
            remainingTime--;
            x = getClk();
            printf("%d\n", remainingTime);
        }
    }
    destroyClk(false);
    
    processInfo.finishTime = x;
    msgsnd(termMsgid, &processInfo, sizeof(struct processFinalInfo), 0);
    return 0;
}

void stopProcess(int signum){
    printf("stopping!");
    msgsnd(termMsgid, &processInfo, sizeof(struct processFinalInfo), 0);
    raise(SIGSTOP);
}
