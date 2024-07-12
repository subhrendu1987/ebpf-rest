#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

void timer_handler(int signum) {
    printf("Timer expired\n");
}

int main() {
    struct sigevent sev;
    struct itimerspec its;
    struct sigaction sa;
    timer_t timerid;
    int status;

    // Setup signal handler
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = timer_handler;
    sigaction(SIGRTMIN, &sa, NULL);

    // Setup timer
    memset(&sev, 0, sizeof(sev));
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGRTMIN;

    // Create timer
    status = timer_create(CLOCK_REALTIME, &sev, &timerid);
    if (status == -1) {
        perror("timer_create");
        exit(EXIT_FAILURE);
    }

    // Set timer to expire after 1 second and then every 1 second
    its.it_value.tv_sec = 1;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 1;
    its.it_interval.tv_nsec = 0;

    status = timer_settime(timerid, 0, &its, NULL);
    if (status == -1) {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    }

    // Sleep to allow timer to expire
    for (;;) {
        pause(); // Wait for signals
    }

    return 0;
}
