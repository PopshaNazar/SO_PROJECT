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
        perror("Eroare: nu se poate scrie în cmd.txt");
        return;
    }
    dprintf(fd, "%s\n", command);
    close(fd);
    kill(monitor_pid, SIGUSR1);
}

void start_monitor()
{
    if (monitor_running)
    {
        printf("Monitorul este deja pornit (PID: %d)\n", monitor_pid);
        return;
    }
    monitor_pid = fork();
    if (monitor_pid == -1)
    {
        perror("Eroare la fork");
        exit(1);
    }
    if (monitor_pid == 0)
    {
        execl("./monitor", "monitor", NULL);
        perror("Eroare: nu se poate porni monitorul");
        exit(1);
    }
    monitor_running = 1;
    printf("Monitorul a fost pornit (PID: %d).\n", monitor_pid);
}

void stop_monitor()
{
    if (!monitor_running)
    {
        printf("Monitorul nu este pornit.\n");
        return;
    }
    printf("Se trimite semnal de oprire catre monitor (PID: %d)...\n", monitor_pid);
    kill(monitor_pid, SIGUSR2);
    int status;
    waitpid(monitor_pid, &status, 0);
    printf("Monitorul a fost oprit.\n");
    monitor_running = 0;
}

int main()
{
    char input[STRING_SIZE];

    while (1)
    {
        printf("\n");
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
        else if (strncmp(input, "list_hunts", 10) == 0)
        {
            if (monitor_running)
            {
                write_command("list_hunts");
            }
            else
            {
                printf("Monitorul nu este pornit.\n");
            }
        }
        else if (strncmp(input, "list_treasures ", 15) == 0)
        {
            if (monitor_running)
            {
                write_command(input);
            }
            else
            {
                printf("Monitorul nu este pornit.\n");
            }
        }
        else if (strncmp(input, "view_treasure ", 14) == 0)
        {
            if (monitor_running)
            {
                write_command(input);
            }
            else
            {
                printf("Monitorul nu este pornit.\n");
            }
        }
        else if (strcmp(input, "exit") == 0)
        {
            if (monitor_running)
            {
                printf("Monitorul este inca activ.\n");
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
    }

    return 0;
}
