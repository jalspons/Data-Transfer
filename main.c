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

#include "lib.h"
#include "morselib.h"


static int file;
static volatile int sigCnt = 0;
static int val = 0;
static int sigpipe;
static const struct timespec timeout = {1, 0}; 

static void sigHandler(int sig, siginfo_t *si, void *ucontext)
{
    long timeDiff = getTimeDiff();
    int code = si->si_value.sival_int;

    if (timeDiff > SHORTPAUSE) {
        write(sigpipe, &val, 1);
        val = 0;
        
        if (timeDiff > LONGPAUSE) {
            write(sigpipe, &val, 1);
        }
    }

    if (code == LONG) {
        val = (val << 1) | 1;
    } else if (code == SHORT) {
        val <<= 1;
    }

    sigCnt++;
}

int main(int argc, char *argv[]) 
{
    if(argc < 3 || argc > 4) {
        printf("Usage: %s file1 file2\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t childPid;

    if(initTime() == -1) {
        errExit("Init time");
    }
    
    if ((childPid = fork()) == -1) {
        errExit("fork");
    }
   
    else if (childPid == 0) {
        pid_t ppid = getppid();
        
        char buf[MAXBUF];
        size_t nread;

        if ((file = open(argv[1], READ_FLAGS)) == -1) {
            sendSignal(ppid, FILENOTFOUND);
            errExit("Child open");
        }
        
        while((nread = read(file, buf, sizeof(buf))) > 0) {
            for (int i = 0; i < nread; i++) {
               if (isspace(buf[i])) {
                   sleepFor(LONGPAUSE);
               } else {
                   int code = encodeMorse(buf[i]);
                   printf("C -  sent '%c': %d\n", buf[i], code);
                   
                   for (int j = getShiftN(code)-1; j >= 0; j--) {
                       int signal = (code >> j & 1) ? LONG : SHORT;
                       
                       if (sendSignal(ppid, signal) == -1) {
                           close(file);
                           errExit("Child sendSignal");
                       }
                   }
                   sleepFor(SHORTPAUSE);
               }
           }
        }
        if (nread < 0) {
            errExit("Child read");
        }

        printf("Child sent %d signals\n", getSigCount());
       
        close(file);
    }
    
    else { 
        int pipefd[2], readFd, ready;
        fd_set watchset, inset;
        char c;
        struct sigaction sa;


        /* Configgure Signal handling */
        memset(&sa, '\0', sizeof(sa));
        sa.sa_sigaction = sigHandler;
        sa.sa_flags = SA_SIGINFO;
        sigemptyset(&sa.sa_mask);

        if(sigaction(SIGRTMIN, &sa, NULL) == -1) {
            errExit("Parent Sigaction");
        }
      
        if ((file = open(argv[2], WRITE_FLAGS, FILE_PERMS)) == -1) {
           errExit("open");
        }

        /* Init Pipe */
        pipe(pipefd);
        sigpipe = pipefd[1];
        readFd = pipefd[0];

        /* Add pipe read-end to fd_set */
        FD_ZERO(&watchset);
        FD_SET(pipefd[0], &watchset);
       
        /* Loop for receiving signals and writing to a file */
        while(FD_ISSET(pipefd[0], &watchset)) {
            inset = watchset;
            
            /* Wait for signals or data in pipe. Otherwise timeout */
            if ((ready = pselect(pipefd[0]+1, &inset, 
                            NULL, NULL, &timeout, NULL)) < 1) 
            {
                /* Error handling*/
               if(ready == 0) {
                   FD_CLR(pipefd[0], &watchset);
                   break;
               } else if (errno == EINTR) {
                   continue;
               } else {
                   perror("Pselect");
                   continue;
               }
            }

            /* If data is available in pipe, */
            /* read data and write to file */
            if (FD_ISSET(pipefd[0], &inset)) {
                if(read(pipefd[0], &c, 1) < 0) {
                   close(pipefd[0]);
                   perror("read");
                }
                
                c = decodeMorse(c);
                printf("P - caught char: '%c'\n", c);
                write(file, &c, 1);
            }
        }
        
        c = decodeMorse(val);
        write(file,&c, 1);
        printf("P - Last char: '%c'\n", c); 

        printf("Parent caught %d signals\n", sigCnt);
       
        close(pipefd[0]);
        close(pipefd[1]);
        close(file);
    }
    
    exit(EXIT_SUCCESS);
}
