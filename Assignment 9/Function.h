#ifndef FUNCTION_H
#define FUNCTION_H

#include <stdio.h>

enum ProcessState {
    NewState,
    ReadyState,
    RunningState,
    WaitingState,
    TerminatedState,
    KilledState
};

typedef struct {
    char processName[64];
    int pid;
    int cpuBurst;
    int ioStart;
    int ioDuration;
    int remainingCpu;
    int remainingIo;
    int executedCpu;
    int arrivalTime;
    int completionTime;
    int turnaroundTime;
    int waitingTime;
    enum ProcessState state;
    int killedAtTick;
} Process;

typedef struct {
    int pid;
    int killTick;
} KillEvent;

Process *parseInput(int *outProcessCount, KillEvent **outKillList, int *outKillCount);
void runFcfsScheduler(Process *processList, int processCount, KillEvent *killList, int killCount);
void printSummary(Process *processList, int processCount);

#endif
