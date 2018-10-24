#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>


#include "lib.h"
#include "morselib.h"


static int sigCnt = 0;
static struct timespec prev, new;

/* S = 0.003 seconds, L = 0.007 seconds */

int initTime(void)
{
    return clock_gettime(CLOCK_MONOTONIC, &prev);
}

long getTimeDiff()
{
    if(clock_gettime(CLOCK_MONOTONIC, &new) == -1) {
        errExit("clock_gettime");
    }
   
    long prevTime = (long)prev.tv_sec * 1000000000L + prev.tv_nsec;
    long newTime = (long)new.tv_sec * 1000000000L + new.tv_nsec;
    long diff = newTime - prevTime;
    
    prev = new;

    return diff;
}

struct timespec getTimeInSec(int sec)
{
    struct timespec t = {sec, 0};

    return t;
}


int getSigCount()
{
    return sigCnt;
}

void errExit(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int sendSignal(pid_t pid, int value)
{
    int resValue, err;

    union sigval sv;
    sv.sival_int = value;
    
    while ((resValue = sigqueue(pid, SIGRTMIN, sv)) == -1) {
        err = errno;

        if (err == EAGAIN) {
            printf("EAGAIN");
            return resValue;
        }
        else {
            return resValue;
        }
    }
    
    sigCnt++;
    return resValue;
}

int delaySending(long val)
{
    struct timespec t = {0, val};

    return clock_nanosleep(CLOCK_MONOTONIC, 0, &t, NULL); 
}

