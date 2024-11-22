#include "headers.h"
#include <stdio.h>

#define RUN_TIME argv[3]


/* Modify this file as needed*/
int remainingtime;


int main(int agrc, char * argv[])
{

    initClk();
    unsigned int startTime = getClk();

    int x = getClk();
    remainingtime = atoi(RUN_TIME);
    // it needs to get the remaining time from somewhere
    while (remainingtime > 0)
    {
        if (getClk() != x){
            remainingtime--;
            x = getClk();
            printf("%d\n", remainingtime);
        }
    }
    destroyClk(false);

    

    return 0;
}
