#include "Function.h"
#include <stdlib.h>
#include <string.h>

static int findProcessIndexByPid(Process *processList, int processCount, int searchPid)
{
    for (int index = 0; index < processCount; ++index) {
        if (processList[index].pid == searchPid) return index;
    }
    return -1;
}

Process *parseInput(int *outProcessCount, KillEvent **outKillList, int *outKillCount)
{
    Process *processList = NULL;
    int processCapacity = 0;
    int processCount = 0;
    KillEvent *killList = NULL;
    int killCapacity = 0;
    int killCount = 0;
    char inputLine[256];
    while (fgets(inputLine, sizeof(inputLine), stdin) != NULL) {
        char firstToken[16];
        int scannedCount = sscanf(inputLine, " %15s", firstToken);
        if (scannedCount <= 0) continue;
        if (strcmp(firstToken, "KILL") == 0) {
            int parsedPid = 0;
            int parsedTick = 0;
            int result = sscanf(inputLine, " KILL %d %d", &parsedPid, &parsedTick);
            if (result == 2) {
                if (killCount >= killCapacity) {
                    killCapacity = killCapacity == 0 ? 4 : killCapacity * 2;
                    killList = realloc(killList, sizeof(KillEvent) * killCapacity);
                }
                killList[killCount].pid = parsedPid;
                killList[killCount].killTick = parsedTick;
                ++killCount;
            }
        } else {
            char nameBuffer[64];
            int parsedPid = 0;
            int parsedCpu = 0;
            int parsedIoStart = -1;
            int parsedIoDuration = 0;
            int tokenCount = sscanf(inputLine, " %63s %d %d %d %d", nameBuffer, &parsedPid, &parsedCpu, &parsedIoStart, &parsedIoDuration);
            if (tokenCount >= 3) {
                if (processCount >= processCapacity) {
                    processCapacity = processCapacity == 0 ? 4 : processCapacity * 2;
                    processList = realloc(processList, sizeof(Process) * processCapacity);
                }
                strncpy(processList[processCount].processName, nameBuffer, sizeof(processList[processCount].processName) - 1);
                processList[processCount].processName[sizeof(processList[processCount].processName) - 1] = '\0';
                processList[processCount].pid = parsedPid;
                processList[processCount].cpuBurst = parsedCpu;
                processList[processCount].ioStart = parsedIoStart;
                processList[processCount].ioDuration = parsedIoDuration;
                processList[processCount].remainingCpu = parsedCpu;
                processList[processCount].remainingIo = parsedIoDuration;
                processList[processCount].executedCpu = 0;
                processList[processCount].arrivalTime = 0;
                processList[processCount].completionTime = -1;
                processList[processCount].turnaroundTime = -1;
                processList[processCount].waitingTime = 0;
                processList[processCount].state = ReadyState;
                processList[processCount].killedAtTick = -1;
                ++processCount;
            }
        }
    }
    *outProcessCount = processCount;
    *outKillList = killList;
    *outKillCount = killCount;
    return processList;
}

static int isAllDone(Process *processList, int processCount)
{
    for (int index = 0; index < processCount; ++index) {
        if (processList[index].state != TerminatedState && processList[index].state != KilledState) return 0;
    }
    return 1;
}

void runFcfsScheduler(Process *processList, int processCount, KillEvent *killList, int killCount)
{
    int currentTick = 0;
    int runningIndex = -1;
    while (!isAllDone(processList, processCount)) {
        for (int index = 0; index < killCount; ++index) {
            if (killList[index].killTick == currentTick) {
                int foundIndex = findProcessIndexByPid(processList, processCount, killList[index].pid);
                if (foundIndex >= 0) {
                    if (processList[foundIndex].state == RunningState) {
                        processList[foundIndex].state = KilledState;
                        processList[foundIndex].killedAtTick = currentTick;
                        processList[foundIndex].completionTime = currentTick;
                        processList[foundIndex].remainingCpu = 0;
                    } else if (processList[foundIndex].state == ReadyState || processList[foundIndex].state == WaitingState || processList[foundIndex].state == NewState) {
                        processList[foundIndex].state = KilledState;
                        processList[foundIndex].killedAtTick = currentTick;
                        processList[foundIndex].completionTime = currentTick;
                        processList[foundIndex].remainingCpu = 0;
                    }
                }
            }
        }
        for (int index = 0; index < processCount; ++index) {
            if (processList[index].state == ReadyState) {
                if (processList[index].remainingCpu > 0) {
                    if (runningIndex == -1) {
                        runningIndex = index;
                        processList[runningIndex].state = RunningState;
                    }
                }
            }
        }
        if (runningIndex >= 0) {
            Process *currentProcess = &processList[runningIndex];
            if (currentProcess->remainingCpu > 0) {
                if (currentProcess->ioStart >= 0 && currentProcess->executedCpu == currentProcess->ioStart && currentProcess->remainingIo > 0) {
                    currentProcess->state = WaitingState;
                } else {
                    currentProcess->state = RunningState;
                    for (int index = 0; index < processCount; ++index) {
                        if (index != runningIndex) {
                            if (processList[index].state == ReadyState) processList[index].waitingTime += 1;
                        }
                    }
                    currentProcess->remainingCpu -= 1;
                    currentProcess->executedCpu += 1;
                    if (currentProcess->remainingCpu == 0) {
                        currentProcess->state = TerminatedState;
                        currentProcess->completionTime = currentTick + 1;
                        currentProcess->turnaroundTime = currentProcess->completionTime - currentProcess->arrivalTime;
                        runningIndex = -1;
                    }
                }
            } else {
                currentProcess->state = TerminatedState;
                currentProcess->completionTime = currentTick;
                currentProcess->turnaroundTime = currentProcess->completionTime - currentProcess->arrivalTime;
                runningIndex = -1;
            }
        }
        for (int index = 0; index < processCount; ++index) {
            if (processList[index].state == WaitingState) {
                if (processList[index].remainingIo > 0) {
                    processList[index].remainingIo -= 1;
                    if (processList[index].remainingIo == 0) {
                        processList[index].state = ReadyState;
                    }
                } else {
                    processList[index].state = ReadyState;
                }
            }
        }
        if (runningIndex == -1) {
            for (int index = 0; index < processCount; ++index) {
                if (processList[index].state == ReadyState && processList[index].remainingCpu > 0) {
                    runningIndex = index;
                    processList[runningIndex].state = RunningState;
                    break;
                }
            }
        }
        currentTick += 1;
    }
    for (int index = 0; index < processCount; ++index) {
        if (processList[index].state == KilledState) {
            if (processList[index].completionTime < 0) processList[index].completionTime = processList[index].killedAtTick;
            processList[index].turnaroundTime = -1;
        } else {
            if (processList[index].completionTime >= 0) processList[index].turnaroundTime = processList[index].completionTime - processList[index].arrivalTime;
        }
    }
}

void printSummary(Process *processList, int processCount)
{
    printf("PID Name CPU IO Turnaround Waiting Status\n");
    for (int index = 0; index < processCount; ++index) {
        Process *entry = &processList[index];
        const char *statusLabel = "OK";
        if (entry->state == KilledState) statusLabel = "KILLED";
        if (entry->ioDuration <= 0) printf("%d %s %d %d %d %d %s\n", entry->pid, entry->processName, entry->cpuBurst, 0, entry->turnaroundTime >= 0 ? entry->turnaroundTime : -1, entry->waitingTime, statusLabel);
        else printf("%d %s %d %d %d %d %s\n", entry->pid, entry->processName, entry->cpuBurst, entry->ioDuration, entry->turnaroundTime >= 0 ? entry->turnaroundTime : -1, entry->waitingTime, statusLabel);
    }
}
