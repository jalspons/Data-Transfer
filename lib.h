#ifndef LIB_H_
#define LIB_H_

#define MAXBUF 4096

#define FILE_PERMS S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define WRITE_FLAGS O_NONBLOCK | O_CREAT | O_WRONLY
#define READ_FLAGS O_RDONLY

#define LONG 3
#define SHORT 1
#define END -3
#define ERROR -1
#define FILENOTFOUND -2

#define LONGPAUSE 70000000
#define SHORTPAUSE 30000000

struct timespec getTimeInSec(int sec);

int initTime(void);

long getTimeDiff();

int getSigCount();

void errExit(char *msg);

int sendSignal(pid_t pid, int value);

int sleepFor(long val);

#endif

