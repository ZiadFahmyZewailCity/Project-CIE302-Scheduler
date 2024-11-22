#include "headers.h"
#include <stdio.h>

#define RUN_TIME atoi(argv[1])
#define PROCESS_ID atoi(argv[2])


/* Modify this file as needed*/
int remainingtime;


int main(int agrc, char * argv[])
{

    initClk();
    unsigned int startTime = getClk();

    int x = getClk();
    remainingtime = RUN_TIME;
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
