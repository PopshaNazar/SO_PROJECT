#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define STRING_SIZE 200
#define CMD_FILE "cmd.txt"

void write_command(const char *command)
{
    int fd = open(CMD_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("Eroare la scrierea cmd.txt");
        return;
    }
    write(fd, command, strlen(command));
    close(fd);
}

void run_monitor_and_print()
{
    int fd[2];
    pipe(fd);

    pid_t pid = fork();
    if (pid == 0)
    {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        execl("./monitor", "monitor", NULL);
        perror("Eroare exec monitor");
        exit(1);
    }
    else
    {
        close(fd[1]);
        char buf[256];
        ssize_t n;
        while ((n = read(fd[0], buf, sizeof(buf) - 1)) > 0)
        {
            buf[n] = '\0';
            printf("%s", buf);
        }
        close(fd[0]);
        wait(NULL);
    }
}

int main()
{
    char input[STRING_SIZE];

    while (1)
    {
        printf("\n");
        fflush(stdout);

        if (!fgets(input, STRING_SIZE, stdin))
            continue;

        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "exit") == 0)
            break;

        if (
            strncmp(input, "list_hunts", 10) == 0 ||
            strncmp(input, "list_treasures", 14) == 0 ||
            strncmp(input, "view_treasure", 13) == 0)
        {
            write_command(input);
            run_monitor_and_print();
        }
        else
        {
            printf("Comanda necunoscuta.\n");
        }

        printf("\n");
    }

    return 0;
}
