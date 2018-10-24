#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>


#include "lib.h"
#include "morselib.h"


static int sigCnt = 0;
static struct timespec prev, new;
static const char *logfile = "morse.log";
static int logfd;


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


int initLog(void)
{
    if ((logfd = open(logfile, WRITE_FLAGS, FILE_PERMS)) == -1) {
        perror("Logfile init");
        return EXIT_FAILURE;
    }

    dprintf(logfd, "\n---Log starting----\n");
    return EXIT_SUCCESS;
}

int log_out(char *fmt, ...)
{
    int res;
    va_list args;

    va_start(args, fmt);
    res = vdprintf(logfd, fmt, args);
    va_end(args);

    return res;
}

int log_stop(void)
{
    return close(logfd);
}
