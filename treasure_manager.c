#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>

#define STRING_SIZE 256
#define LOGGED_HUNT "logged_hunt.log"
#define HUNT_FOLDER "hunts"
#define TREASURE_DATA "treasure.dat"

typedef struct
{
    char id[STRING_SIZE];
    char username[STRING_SIZE];
    float latitude;
    float longitude;
    char clue[STRING_SIZE];
    int value;
} Treasure;

int createHuntDirectory(char *hunt_id)
{
    if (mkdir(HUNT_FOLDER, 0755) == 0)
    {
        printf("este primul hunt, s-a creat fisier '%s'\n", HUNT_FOLDER);
    }
    if ((mkdir(HUNT_FOLDER, 0755) == -1) && errno != EEXIST)
    { // daca nu a reusit && fisieru deja exista
        perror("Error");
        return -1;
    }

    char directory_path[STRING_SIZE];
    snprintf(directory_path, sizeof(directory_path), "%s/%s", HUNT_FOLDER, hunt_id);
    if ((mkdir(directory_path, 0755) == -1) && errno != EEXIST)
    {
        perror("Error");
        return -1;
    }

    return 0;
}

void log_action(char *hunt_id, char *action)
{
    char log_path[STRING_SIZE];
    snprintf(log_path, sizeof(log_path), "%s/%s/%s", HUNT_FOLDER, hunt_id, LOGGED_HUNT);

    int fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0)
    {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return;
    }

    dprintf(fd, "[%ld],%s\n", time(NULL), action);
    close(fd);

    char symlink_name[STRING_SIZE];
    snprintf(symlink_name, sizeof(symlink_name), "logged_hunt-%s", hunt_id);
    unlink(symlink_name);
    symlink(log_path, symlink_name);
}

void addTreasure(char *hunt_id)
{
    if (createHuntDirectory(hunt_id) == -1)
    {
        printf("nu se creeaza directorul");
        return;
    }
    char file_path[STRING_SIZE];
    snprintf(file_path, sizeof(file_path), "%s/%s/%s", HUNT_FOLDER, hunt_id, TREASURE_DATA);
    int fd = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0755);
    if (fd == -1)
    {
        perror("Error");
        return;
    }

    Treasure t;
    printf("Treasure ID: ");
    fgets(t.id, STRING_SIZE, stdin);
    t.id[strcspn(t.id, "\n")] = '\0';
    printf("Username: ");
    fgets(t.username, STRING_SIZE, stdin);
    t.username[strcspn(t.username, "\n")] = '\0';
    printf("Latitude: ");
    scanf("%f", &t.latitude);
    getchar();
    printf("Longitude: ");
    scanf("%f", &t.longitude);
    getchar();
    printf("Clue: ");
    fgets(t.clue, STRING_SIZE, stdin);
    t.clue[strcspn(t.clue, "\n")] = '\0';
    printf("Value: ");
    scanf("%d", &t.value);
    getchar();

    if (write(fd, &t, sizeof(Treasure)) == -1)
    {
        perror("Error");
    }
    else
    {
        char action[STRING_SIZE];
        snprintf(action, sizeof(action), "ADDED %s", t.id);
        log_action(hunt_id, action);
        printf("Treasure added!\n");
    }
    close(fd);
}
void list_treasures(const char *hunt_id)
{
    char treasure_file_path[STRING_SIZE];
    snprintf(treasure_file_path, sizeof(treasure_file_path), "%s/%s/%s", HUNT_FOLDER, hunt_id, TREASURE_DATA);

    struct stat treasure_stats;
    if (stat(treasure_file_path, &treasure_stats) == -1)
    {
        perror("Error getting file info");
        return;
    }

    printf("=== Hunt: %s ===\n", hunt_id);
    printf("File size: %ld bytes\n", treasure_stats.st_size);
    printf("Last modified: %s", ctime(&treasure_stats.st_mtime));

    int treasure_fd = open(treasure_file_path, O_RDONLY);
    if (treasure_fd == -1)
    {
        perror("Error opening treasure file");
        return;
    }

    Treasure treasure;
    while (read(treasure_fd, &treasure, sizeof(Treasure)) > 0)
    {
        printf("\nID: %s\nUser: %s\nCoords: %.2f,%.2f\nValue: %d\nClue: %s\n",
               treasure.id, treasure.username, treasure.latitude,
               treasure.longitude, treasure.value, treasure.clue);
    }

    close(treasure_fd);

    log_action(hunt_id, "LIST");
}

void view_treasure(const char *hunt_id, const char *treasure_id)
{
    char treasure_file_path[STRING_SIZE];
    snprintf(treasure_file_path, sizeof(treasure_file_path), "%s/%s/%s", HUNT_FOLDER, hunt_id, TREASURE_DATA);

    int treasure_fd = open(treasure_file_path, O_RDONLY);
    if (treasure_fd == -1)
    {
        perror("Error opening treasure file");
        return;
    }

    Treasure treasure;
    int treasure_found = 0;

    while (read(treasure_fd, &treasure, sizeof(Treasure)) > 0)
    {
        if (strcmp(treasure.id, treasure_id) == 0)
        {
            printf("\n=== Treasure %s ===\n", treasure.id);
            printf("User: %s\nCoords: %.2f,%.2f\nValue: %d\nClue: %s\n",
                   treasure.username, treasure.latitude, treasure.longitude, treasure.value, treasure.clue);
            treasure_found = 1;
            break;
        }
    }

    close(treasure_fd);

    if (!treasure_found)
    {
        printf("Treasure not found!\n");
    }

    char log_entry[STRING_SIZE];
    snprintf(log_entry, sizeof(log_entry), "VIEW %s", treasure_id);
    log_action(hunt_id, log_entry);
}

void removeTreasure(const char *hunt_id, const char *treasure_id)
{
    char file_path[STRING_SIZE];
    snprintf(file_path, sizeof(file_path), "%s/%s/%s", HUNT_FOLDER, hunt_id, TREASURE_DATA);

    int fd = open(file_path, O_RDWR);
    if (fd == -1)
    {
        perror("Error opening file");
        return;
    }

    // file temporar
    char temp_path[STRING_SIZE];
    snprintf(temp_path, sizeof(temp_path), "%s/%s/temp.dat", HUNT_FOLDER, hunt_id);
    int temp_fd = open(temp_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fd == -1)
    {
        perror("Error creating temp file");
        close(fd);
        return;
    }

    Treasure t;
    int found = 0;
    ssize_t bytes_read;

    while ((bytes_read = read(fd, &t, sizeof(Treasure))) > 0)
    {
        if (strcmp(t.id, treasure_id) == 0)
        {
            found = 1;
        }
        else
        {
            write(temp_fd, &t, sizeof(Treasure));
        }
    }

    close(fd);
    close(temp_fd);

    if (found)
    {
        remove(file_path);
        rename(temp_path, file_path);

        char action[STRING_SIZE];
        snprintf(action, sizeof(action), "REMOVE %s", treasure_id);
        log_action(hunt_id, action);
        printf("Treasure %s removed\n", treasure_id);
    }
    else
    {
        remove(temp_path);
        printf("Treasure not found\n");
    }
}

void removeHunt(const char *hunt_id)
{
    char hunt_path[STRING_SIZE];
    snprintf(hunt_path, sizeof(hunt_path), "%s/%s", HUNT_FOLDER, hunt_id);

    char treasure_file[STRING_SIZE];
    snprintf(treasure_file, sizeof(treasure_file), "%s/%s", hunt_path, TREASURE_DATA);

    char log_file[STRING_SIZE];
    snprintf(log_file, sizeof(log_file), "%s/%s", hunt_path, LOGGED_HUNT);

    char logged_hunt_file[STRING_SIZE];
    snprintf(logged_hunt_file, sizeof(logged_hunt_file), "%s/%s", hunt_path, LOGGED_HUNT);

    if (remove(treasure_file) == -1 && errno != ENOENT)
    {
        perror("Error");
        return;
    }

    if (remove(log_file) == -1 && errno != ENOENT)
    {
        perror("Error");
        return;
    }

    if (remove(logged_hunt_file) == -1 && errno != ENOENT)
    {
        perror("Error");
        return;
    }

    if (rmdir(hunt_path) == -1)
    {
        perror("Error");
        return;
    }

    printf("Hunt '%s' removed\n", hunt_id);
}

void printUsage()
{
    printf("Usage:\n");
    printf("  --add <hunt_id>\n");
    printf("  --list <hunt_id>\n");
    printf("  --view <hunt_id> <treasure_id>\n");
    printf("  --remove_treasure <hunt_id> <treasure_id>\n");
    printf("  --remove_hunt <hunt_id>\n");
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printUsage();
        return 1;
    }

    char *command = argv[1];
    char *hunt_id = argv[2];

    if (strcmp(command, "--add") == 0)
    {
        addTreasure(hunt_id);
    }
    else if (strcmp(command, "--list") == 0)
    {
        list_treasures(hunt_id);
    }
    else if (strcmp(command, "--view") == 0 && argc >= 4)
    {
        view_treasure(hunt_id, argv[3]);
    }
    else if (strcmp(command, "--remove_treasure") == 0 && argc >= 4)
    {
        removeTreasure(hunt_id, argv[3]);
    }
    else if (strcmp(command, "--remove_hunt") == 0)
    {
        removeHunt(hunt_id);
    }
    else
    {
        fprintf(stderr, "Invalid command!\n");
        printUsage();
        return 1;
    }

    return 0;
}