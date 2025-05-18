#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CMD_FILE "cmd.txt"
#define BUFFER_SIZE 250

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
            char full_command[BUFFER_SIZE];
            snprintf(full_command, sizeof(full_command), "./treasure_manager --list %s", hunt_id);
            system(full_command);
        }
    }
    else if (strcmp(cmd, "view_treasure") == 0)
    {
        char *hunt_id = strtok(NULL, " \n");
        char *treasure_id = strtok(NULL, " \n");
        if (hunt_id && treasure_id)
        {
            char full_command[BUFFER_SIZE];
            snprintf(full_command, sizeof(full_command), "./treasure_manager --view %s %s", hunt_id, treasure_id);
            system(full_command);
        }
    }
}

int main()
{
    process_command();
    return 0;
}