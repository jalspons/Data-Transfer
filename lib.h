#ifndef LIB_H_
#define LIB_H_

#define MAXBUF 4096

#define FILE_PERMS S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define WRITE_FLAGS O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK
#define READ_FLAGS O_RDONLY

/* COMMUNICATION CODES */
#define LONG 3
#define SHORT 1
#define END -3
#define ERROR -1
#define FILENOTFOUND -2

/* Communication delay: char = SHORTPAUSE, space = LONGPAUSE */
#define LONGPAUSE 70000000
#define SHORTPAUSE 30000000


/* Start "program" clock
 * RETURN: 0 for success, -1 for failure (with errno set)*/
int initTime(void);

/* Calculate time differences from the last time time was updated
 * RETURN: value of time difference in nano seconds*/
long getTimeDiff();

/* RETURN: How many sigals have been sent by using sendSignal */
int getSigCount();

/* Generic error handling */
void errExit(char *msg);

/* Send a signal with "value" to "pid". Uses SIGRTMIN signal as standard.
 * RETURN: 0 on success, -1 on failure (with errno set) */
int sendSignal(pid_t pid, int value);


int delaySending(long val);

int initLog(void);

int log_out(char *fmt, ...);

#endif

