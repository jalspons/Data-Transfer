#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>


#include "lib.h"
#include "morselib.h"


static pid_t pid, ppid;
static int file = -1;
static int error = 0;

static void sig_handler(int sig)
{
    if (sig == SIGINT) {
        log_err("Interrupted execution");
        exit(EXIT_FAILURE);
    }
}

static void handle_termination();

int main(int argc, char *argv[])
{
    char buf[MAXBUF];
    size_t nread;
    pid = getpid();
    ppid = getppid();

    atexit(handle_termination);
    
    if(open_log() == -1) {
        fprintf(stderr, "Open log failed\n");
        exit(EXIT_FAILURE);
    }

    if((file = open(argv[1], READ_FLAGS)) == -1) {
        log_err("open");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, sig_handler);

    while((nread = read(file, buf, sizeof(buf))) > 0) { 
        for (int i = 0; i < nread; i++) {
            if (isspace(buf[i])) {
               delay_sending(LONGPAUSE);
            } else {
               int code = encode_morse(buf[i]);
               fprintf(stdout,"Child[%ld] sent '%c': %d\n", 
                       (long)pid, buf[i], code);
               
               for (int j = morsecode_len(code)-1; j >= 0; j--) {
                   int signal = (code >> j & 1) ? LONG : SHORT;
                   
                   if (send_signal(ppid, signal) == -1) {
                       log_err("Send_signal");
                       exit(EXIT_FAILURE);
                   }
               }
               delay_sending(SHORTPAUSE);
           }
       }
    }
    
    if (nread < 0) {
        log_err("Read");
        error -= 1;
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Child[%ld] sent %d signals\n", 
            (long)pid, get_sig_count());

    exit(EXIT_SUCCESS);
}

static void handle_termination()
{
    fprintf(stdout, "[%ld] Terminating process\n", (long)pid);
    
    if (errno)
        send_signal(ppid, ERROR);

    if (file > 0)
        close(file);
    
    log_close();
}
