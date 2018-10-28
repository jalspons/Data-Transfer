#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>


#include "lib.h"
#include "morselib.h"



/* TIME FUNCTIONS */

static struct timespec prev, new;

int init_time(void)
{
    return clock_gettime(CLOCK_MONOTONIC, &prev);
}

long get_time_diff()
{
    if(clock_gettime(CLOCK_MONOTONIC, &new) == -1)
        log_err("clock_gettime");
   
    long prevtime = (long)prev.tv_sec * 1000000000L + prev.tv_nsec;
    long newtime = (long)new.tv_sec * 1000000000L + new.tv_nsec;
    long diff = newtime - prevtime;
    
    prev = new;

    return diff;
}




/* SIGNAL HELPFUNCTIONS*/
static int sigCnt = 0;

int get_sig_count()
{
    return sigCnt;
}

int send_signal(pid_t pid, int value)
{
    int res, retries = 3;

    union sigval sv;
    sv.sival_int = value;
    
    while ((res = sigqueue(pid, SIGRTMIN, sv)) == -1 && retries > 0) {
        log_err("sigqueue");
        delay_sending(SENDDELAY);
        retries--;
    }
    
    if (retries == 0) {
        fprintf(stderr, "Sending signal failed\n");
        return EXIT_FAILURE;
    }
    
    sigCnt++;
    return res;
}

int delay_sending(long val)
{
    struct timespec t = {0, val};

    return clock_nanosleep(CLOCK_MONOTONIC, 0, &t, NULL); 
}



/* LOGGING FUNCTIONS */

static const char *logfile = "morse.log";
static int logfd = -1;

int open_log(void)
{
    if ((logfd = open(logfile, WRITE_FLAGS, FILE_PERMS)) == -1)
        fprintf(stderr, "Logfile init failed\n");
    
    return logfd;
}


int init_log(void)
{
    open_log();
    
    dprintf(logfd, "\n---Log starting----\n");
    
    return logfd;
}

int log_err(char *msg)
{
    /* Add timestamp */
    struct tm result;
    time_t t = time(NULL);
    struct tm *tm_info = localtime_r(&t, &result);
    char timeBuffer[25];

    strftime(timeBuffer, sizeof(timeBuffer), "%F %T -", tm_info);

    /* Form error message */
    char errMsg[ERRBUF];

    snprintf(errMsg, ERRBUF, "[%ld] %s Error %s: %s\n", 
             (long)getpid(), timeBuffer, msg, strerror(errno));

    return dprintf(logfd, errMsg);
}

int log_close(void)
{
    int res = 0;

    if (logfd > 0)
        res = close(logfd);
    
    return res;
}



/* CHILD TERMINATION */

int terminate_child(pid_t childpid)
{
    int status;
    kill(childpid, SIGINT);

    pid_t wpid = waitpid(-1, &status, 0);
    
    if (wpid == -1) {
        log_err("waitpid");
        return EXIT_FAILURE;
    }

   if (WIFEXITED(status) || WIFSIGNALED(status) || WIFSTOPPED(status))
       return EXIT_SUCCESS;
   else
       return EXIT_FAILURE;
}

