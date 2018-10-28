#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/select.h>
#include <sys/types.h>

#include "lib.h"
#include "morselib.h"

static int val = 0, sigCnt = 0;
static int pipefd[2], file;
static const struct timespec onesec = {1, 0};
static pid_t childpid, ppid;
static void handle_termination(void);

static void sig_handler(int sig, siginfo_t *si, void *ucontext)
{
    if (sig == SIGINT) {
        log_err("Interrupted");
        exit(EXIT_FAILURE);
    }
    
    long timeDiff = get_time_diff();
    int code = si->si_value.sival_int;

    if (code == ERROR) {
        log_err("Child");
        exit(EXIT_FAILURE);
    }

    if (timeDiff > SHORTPAUSE) {
        write(pipefd[1], &val, 1);
        val = 0;
        
        if (timeDiff > LONGPAUSE) {
            write(pipefd[1], &val, 1);
        }
    }

    if (code == LONG)
        val = (val << 1) | 1;
    else
        val <<= 1;

    sigCnt++;
}


int main(int argc, char *argv[]) 
{
    if(argc < 3 || argc > 4) {
        printf("Usage: %s file1 file2\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if(init_log() == -1) {
        log_err("Init log");
        exit(EXIT_FAILURE);
    }

    if(init_time() == -1) {
        log_err("Init Time");
        exit(EXIT_FAILURE);
    }

    ppid = getpid();

    if ((childpid = fork()) == -1) {
        log_err("Fork");
        exit(EXIT_FAILURE);
    }
   
    /* Child moving to other program or stopping execution */
    else if (childpid == 0) {
        char *params[] = {"./ChildProgram", argv[1], NULL};
        
        execv("./ChildProgram", params);
        
        _exit(EXIT_FAILURE);
    }
    else { 
        fd_set watchset, inset;
        char c;
        struct sigaction sa;
        int ready;

        
        pipefd[0] = 0; 
        pipefd[1] = 0;
        file = 0;
        atexit(handle_termination);

        /* Configgure Signal handling */
        memset(&sa, '\0', sizeof(sa));
        sa.sa_sigaction = sig_handler;
        sa.sa_flags = SA_SIGINFO;
        sigemptyset(&sa.sa_mask);

        if(sigaction(SIGRTMIN, &sa, NULL) == -1) {
            terminate_child(childpid);
            log_err("Sigaction");
            exit(EXIT_FAILURE);
        }

        if(sigaction(SIGINT, &sa, NULL) == -1) {
            terminate_child(childpid);
            log_err("Sigaction");
            exit(EXIT_FAILURE);
        }

        if ((file = creat(argv[2], FILE_PERMS))== -1) {
            terminate_child(childpid);
            log_err("open");
            exit(EXIT_FAILURE);
        }

        /* Init Pipe */
        if (pipe(pipefd) == -1) {
            terminate_child(childpid);
            log_err("Pipe");
            exit(EXIT_FAILURE);
        }

        /* Add pipe read-end to fd_set */
        FD_ZERO(&watchset);
        FD_SET(pipefd[0], &watchset);
       
        /* Loop for receiving signals and writing to a file */
        while(FD_ISSET(pipefd[0], &watchset)) {
            inset = watchset;
            
            /* Wait for signals or data in pipe. Otherwise timeout */
            if ((ready = pselect(pipefd[0]+1, &inset, 
                            NULL, NULL, &onesec, NULL)) < 1) 
            {
                /* Error handling*/
               if(!ready) {
                   FD_CLR(pipefd[0], &watchset);
               } else if (errno == EINTR) {
                   continue;
               } else {
                   log_err("Pselect");
                   exit(EXIT_FAILURE);
               }
            }

            /* If pipe is ready, read data and write to file */
            if (FD_ISSET(pipefd[0], &inset)) {
                if(read(pipefd[0], &c, 1) < 0) {
                   log_err("Read");
                   exit(EXIT_FAILURE);
                }
                
                c = decode_morse(c);
                fprintf(stdout,"Parent[%ld] caught '%c'\n", 
                        (long)ppid, c);   
                
                if(write(file, &c, 1) == -1) {
                    log_err("write");
                    exit(EXIT_FAILURE);
                }
            }
        }
       
        /* Catch the last char */
        c = decode_morse(val);
        write(file,&c, 1);
        fprintf(stdout,"Parent[%ld] caught '%c'\n", 
                (long)ppid, c);   

        /* Send total amount of signals count */
        fprintf(stdout,"Parent[%ld] caught %d signals\n", 
                (long)ppid, sigCnt);   
     
        exit(EXIT_SUCCESS);
    }
}

static void handle_termination(void)
{
    terminate_child(childpid);
    
    fprintf(stderr, "[%ld] Terminating process\n", (long)ppid);
  
    if (pipefd[0])
        close(pipefd[0]);
    if (pipefd[1])
        close(pipefd[1]);
    if (file)
        close(file);
   
    log_close();
}
