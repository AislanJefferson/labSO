#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void handlerUSR1(int signo, siginfo_t *si, void *data) {
    (void) signo;
    (void) data;
    switch (si->si_signo) {
        case SIGUSR1:
            printf("Value %d received from pid %lu\n", 
(si->si_value).sival_int,
                   (unsigned long) si->si_pid);
            exit(0);
            break;
    }
}


int main(void) {

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);

    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handlerUSR1;

    if (sigaction(SIGUSR1, &sa, 0) == -1) {
        fprintf(stderr, "%s: %s\n", "sigaction", strerror(errno));
    }
    printf("Pid %lu waiting for SIGUSR1\n", (unsigned long) getpid());
    for (;;) {
        sleep(10);
    }
    return 0;
}
