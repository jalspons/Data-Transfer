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


static int file, sigpipe;
static int val = 0, sigCnt = 0;
static const struct timespec onesec = {1, 0};

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

    pid_t childpid, parentpid;
    parentpid = getpid();

    if(initLog() == -1) {
        errExit("Init log");
    }

    if(initTime() == -1) {
        errExit("Init time");
    }

    if ((childpid = fork()) == -1) {
        errExit("Fork");
    }
   
    else if (childpid == 0) {
        char buf[MAXBUF];
        size_t nread;

        childpid = getpid();

        if ((file = open(argv[1], READ_FLAGS)) == -1) {
            sendSignal(parentpid, FILENOTFOUND);
            errExit("Child open");
        }
        
        while((nread = read(file, buf, sizeof(buf))) > 0) {
            for (int i = 0; i < nread; i++) {
               if (isspace(buf[i])) {
                   delaySending(LONGPAUSE);
               } else {
                   int code = encodeMorse(buf[i]);
                   log_out("Child[%ld] sent '%c': %d\n", 
                           (long)childpid, buf[i], code);
                   
                   for (int j = codeShifts(code)-1; j >= 0; j--) {
                       int signal = (code >> j & 1) ? LONG : SHORT;
                       
                       if (sendSignal(parentpid, signal) == -1) {
                           close(file);
                           errExit("Child sendSignal");
                       }
                   }
                   delaySending(SHORTPAUSE);
               }
           }
        }
        if (nread < 0) {
            errExit("Child read");
        }
       
        log_out("Child[%ld] sent %d signals\n", 
                (long)childpid, getSigCount());
      
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
      
        if ((file = creat(argv[2], FILE_PERMS))== -1) {
           errExit("open");
        }

        /* Init Pipe */
        pipe(pipefd);
        sigpipe = pipefd[1];
        readFd = pipefd[0];

        /* Add pipe read-end to fd_set */
        FD_ZERO(&watchset);
        FD_SET(readFd, &watchset);
       
        /* Loop for receiving signals and writing to a file */
        while(FD_ISSET(readFd, &watchset)) {
            inset = watchset;
            
            /* Wait for signals or data in pipe. Otherwise timeout */
            if ((ready = pselect(readFd+1, &inset, 
                            NULL, NULL, &onesec, NULL)) < 1) 
            {
                /* Error handling*/
               if(!ready) {
                   FD_CLR(readFd, &watchset);
               } else if (errno == EINTR) {
                   continue;
               } else {
                   perror("Pselect");
                   continue;
               }
            }

            /* If data is available in pipe, */
            /* read data and write to file */
            if (FD_ISSET(readFd, &inset)) {
                if(read(readFd, &c, 1) < 0) {
                   close(readFd);
                   perror("read");
                }
                
                c = decodeMorse(c);
                log_out("Parent[%ld] caught '%c'\n", 
                        (long)parentpid, c);   
                write(file, &c, 1);
            }
        }
       
        /* Catch the last char */
        c = decodeMorse(val);
        write(file,&c, 1);
        log_out("Parent[%ld] caught '%c'\n", 
                (long)parentpid, c);   

        /* Send total amount of signals count */
        log_out("Parent[%ld] caught %d signals\n", 
                (long)parentpid, sigCnt);   
      
        /* close pipe */
        close(readFd);
        close(sigpipe);

    }
    
    close(file);
    
    exit(EXIT_SUCCESS);
}
