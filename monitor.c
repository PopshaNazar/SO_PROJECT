#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t received_sigusr2 = 0;

void handle_sigusr2(int signum)
{
    received_sigusr2 = 1;
}

int main()
{
    struct sigaction sa;
    sa.sa_handler = handle_sigusr2;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR2, &sa, NULL);

    printf("[Monitor] Pornit. Aștept semnale...\n");

    while (1)
    {
        pause(); // asteapta semnal

        if (received_sigusr2)
        {
            printf("Semnal primit\n");
            break;
        }
    }

    printf("[Monitor] Închis.\n");
    return 0;
}
