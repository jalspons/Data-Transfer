#ifndef LIB_H_
#define LIB_H_

#define MAXBUF 4096
#define ERRBUF 81

#define FILE_PERMS S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define WRITE_FLAGS O_WRONLY | O_APPEND
#define READ_FLAGS O_RDONLY

/* COMMUNICATION CODES */
#define LONG 3
#define SHORT 1
#define ERROR -1

/* Communication delay: char = SHORTPAUSE, space = LONGPAUSE */
#define LONGPAUSE 70000000
#define SHORTPAUSE 30000000
#define SENDDELAY 10000


/* Start "program" clock
 * RETURN: 0 for success, -1 for failure (with errno set)*/
int init_time(void);

/* Calculate time differences from the last time time was updated
 * RETURN: value of time difference in nano seconds*/
long get_time_diff();

/* RETURN: How many sigals have been sent by using sendSignal */
int get_sig_count();


/* Send a signal with "value" to "pid". Uses SIGRTMIN signal as standard.
 * RETURN: 0 on success, -1 on failure (with errno set) */
int send_signal(pid_t pid, int value);


int delay_sending(long val);



int open_log(void);

int init_log(void);

int get_logfd(void);

/* Generic error handling 
 *
 * On success returns a number of bytes written to log
 * on failure returns -1 */
int log_err(char *msg);

int log_close(void);

int terminate_child(pid_t childpid);

#endif

