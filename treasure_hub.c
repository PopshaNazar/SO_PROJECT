#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

#define STRING_SIZE 200
#define CMD_FILE "cmd.txt"

pid_t monitor_pid = -1;
int monitor_running = 0;

void write_command(const char *command)
{
    int fd = open(CMD_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("erroare: nu se poate scrie în cmd.txt");
        return;
    }
    write(fd, command, strlen(command));
    close(fd);
}

void handle_sigchld(int sig)
{
    wait(NULL);
    monitor_running = 0;
    printf("Monitorul s-a inchis.\n");
}

int main()
{
    signal(SIGCHLD, handle_sigchld);

    char input[STRING_SIZE];

    while (1)
    {
        printf("[Comanda]> ");
        fflush(stdout);

        if (!fgets(input, STRING_SIZE, stdin))
        {
            printf("erroare la citirea comenzii.\n");
            continue;
        }

        input[strcspn(input, "\n")] = 0; // elimină newline

        if (strcmp(input, "start_monitor") == 0)
        {
            if (monitor_running)
            {
                printf("Monitorul ruleaza deja.\n");
            }
            else
            {
                monitor_pid = fork();
                if (monitor_pid == 0)
                {
                    execl("./monitor", "monitor", NULL);
                    perror("erroare la pornirea monitorului");
                    exit(1);
                }
                else
                {
                    monitor_running = 1;
                    printf("Monitor pornit cu PID %d.\n", monitor_pid);
                }
            }
        }
        else if (strcmp(input, "stop_monitor") == 0)
        {
            if (!monitor_running)
            {
                printf("Monitorul nu ruleaza.\n");
            }
            else
            {
                kill(monitor_pid, SIGUSR2);
            }
        }
        else if (strncmp(input, "list_hunts", 10) == 0 ||
                 strncmp(input, "list_treasures", 14) == 0 ||
                 strncmp(input, "view_treasure", 13) == 0)
        {
            if (!monitor_running)
            {
                printf("Monitorul nu este activ.\n");
            }
            else
            {
                write_command(input);
                kill(monitor_pid, SIGUSR1);
            }
        }
        else if (strcmp(input, "exit") == 0)
        {
            if (monitor_running)
            {
                printf("Monitorul inca ruleaza.\n");
            }
            else
            {
                break;
            }
        }
        else
        {
            printf("Comanda necunoscuta.\n");
        }

        printf("\n");
    }

    return 0;
}
