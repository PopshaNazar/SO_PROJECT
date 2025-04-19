#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define STRING_SIZE 200

pid_t monitor_pid = -1;
int monitor_running = 0;

void start_monitor()
{
    if (monitor_running)
    {
        printf("monitorul ruleaza deja (PID: %d)\n", monitor_pid);
        return;
    }
    monitor_pid = fork();
    if (monitor_pid == -1)
    {
        perror("Error fork");
        exit(1);
    }
    if (monitor_pid == 0) // copil
    {
        execl("./monitor", "monitor", NULL);
        perror("Nu am putut porni monitorul");
        exit(1);
    }
    monitor_running = 1;
    printf("Monitorul a fost pornit cu PID %d.\n", monitor_pid);
}

void stop_monitor()
{
    if (!monitor_running)
    {
        printf("monitorul nu este pornit ca sa il inchizi\n");
        return;
    }
    printf("trimit SIGUSR2 catre monitorul PID: %d\n", monitor_pid);
    kill(monitor_pid, SIGUSR2);

    int status;
    waitpid(monitor_pid, &status, 0);

    printf("Monitoru s a inchis\n");
    monitor_running = 0;
}

int main()
{
    char input[STRING_SIZE];

    while (1)
    {
        printf(">>> ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "start_monitor") == 0)
        {
            start_monitor();
        }

        else if (strcmp(input, "stop_monitor") == 0)
        {
            stop_monitor();
        }
        else if (strcmp(input, "exit") == 0)
        {
            if (monitor_running)
            {
                printf("Monitorul inca ruleaza\n");
            }
            else
            {
                break;
            }
        }
        else
        {
            printf("NOT FOUND COMMAND, exit\n");
        }
    }

    return 0;
}