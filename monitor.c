#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>

#define CMD_FILE "monitor_cmd.txt"
#define BUFFER_SIZE 250

int pipe_fds[2]; // 0 = read, 1 = write

void handle_sigusr1(int signum)
{
    char code = 'C'; // Command
    write(pipe_fds[1], &code, 1);
}

void handle_sigusr2(int signum)
{
    char code = 'X'; // Exit
    write(pipe_fds[1], &code, 1);
}

void process_command()
{
    FILE *fp = fopen(CMD_FILE, "r");
    if (!fp)
    {
        perror("Monitor: nu se poate deschide cmd.txt");
        return;
    }

    char command[BUFFER_SIZE];
    fgets(command, BUFFER_SIZE, fp);
    fclose(fp);

    char *cmd = strtok(command, " \n");
    if (!cmd)
        return;

    if (strcmp(cmd, "list_hunts") == 0)
    {
        system("ls hunts");
    }
    else if (strcmp(cmd, "list_treasures") == 0)
    {
        char *hunt_id = strtok(NULL, " \n");
        if (hunt_id)
        {
            char cmdline[BUFFER_SIZE];
            snprintf(cmdline, sizeof(cmdline), "./treasure_manager --list %s", hunt_id);
            system(cmdline);
        }
    }
    else if (strcmp(cmd, "view_treasure") == 0)
    {
        char *hunt_id = strtok(NULL, " \n");
        char *treasure_id = strtok(NULL, " \n");
        if (hunt_id && treasure_id)
        {
            char cmdline[BUFFER_SIZE];
            snprintf(cmdline, sizeof(cmdline), "./treasure_manager --view %s %s", hunt_id, treasure_id);
            system(cmdline);
        }
    }
    else
    {
        printf("Monitor: comanda necunoscuta.\n");
    }
}

int main()
{
    if (pipe(pipe_fds) == -1)
    {
        perror("Eroare la pipe");
        exit(1);
    }

    struct sigaction sa1, sa2;
    sa1.sa_handler = handle_sigusr1;
    sigemptyset(&sa1.sa_mask);
    sa1.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa1, NULL);

    sa2.sa_handler = handle_sigusr2;
    sigemptyset(&sa2.sa_mask);
    sa2.sa_flags = SA_RESTART;
    sigaction(SIGUSR2, &sa2, NULL);

    printf("Monitor: pornit.\n");

    char signal_code;
    while (1)
    {
        if (read(pipe_fds[0], &signal_code, 1) > 0)
        {
            if (signal_code == 'C')
            {
                process_command();
            }
            else if (signal_code == 'X')
            {
                break;
            }
        }
    }
    printf("Monitor: oprit.\n");
    return 0;
}