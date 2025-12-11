#include "Function.h"
#include <stdlib.h>

int main(void)
{
    Process *processList = NULL;
    KillEvent *killList = NULL;
    int processCount = 0;
    int killCount = 0;
    processList = parseInput(&processCount, &killList, &killCount);
    runFcfsScheduler(processList, processCount, killList, killCount);
    printSummary(processList, processCount);
    free(processList);
    free(killList);
    return 0;
}
