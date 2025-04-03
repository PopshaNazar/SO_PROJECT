
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>

#define STRING_SIZE 250
#define HUNT_FILE "hunt.log"
#define TREASURE_FILE "treasure.dat"

typedef struct
{
    char treasure_id[20]; // sa nu uit sa il fa unic
    char user_name[STRING_SIZE];
    float latitude;
    float longitude;
    char clue[STRING_SIZE];
    int value;
} Treasure;

int create_hunt_dir(char *hunt_id)
{
    char dir_path[STRING_SIZE];
    snprintf(dir_path, sizeof(dir_path), "hunts/%s", hunt_id);

    if (mkdir("hunts", 0755) == -1 && errno != EEXIST)
    {
        perror("Eroare la creare director 'hunts'");
        return -1;
    }

    if (mkdir(dir_path, 0755) == -1 && errno != EEXIST)
    {
        perror("Eroare la creare director 'hunt'");
        return -1;
    }

    // Creează symlink pentru log
    char symlink_path[STRING_SIZE];
    snprintf(symlink_path, sizeof(symlink_path), "logged_hunt_%s", hunt_id);
    char log_path[STRING_SIZE];
    snprintf(log_path, sizeof(log_path), "%s/%s", dir_path, HUNT_FILE);
    unlink(symlink_path); // sterge symlink deja care deja exista
    symlink(log_path, symlink_path);

    return 0;
}

void addTreasure(char *hunt_id)
{
    if (create_hunt_dir(hunt_id) == -1)
        return;

    char file_path[STRING_SIZE];
    snprintf(file_path, sizeof(file_path), "hunts/%s/%s", hunt_id, TREASURE_FILE);

    int fd = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644); // 1x 4r 2w [0ugo]
    if (fd == -1)
    {
        perror("Eroare deschidere fisier treasure");
        return;
    }
    Treasure t;

    printf("Treasure ID: ");
    fgets(t.treasure_id, STRING_SIZE, stdin);
    t.treasure_id[strcspn(t.treasure_id, "\n")] = '\0';

    printf("Username: ");
    fgets(t.user_name, STRING_SIZE, stdin);
    t.user_name[strcspn(t.user_name, "\n")] = '\0';

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

    write(fd, &t, sizeof(Treasure));
    close(fd);

    printf("treasure s-a adaugat fara probleme");
}

int main(int argc, char **argv)
{
    // if(argc > 3 ){
    //     printf("s-a introdus prea multe argumente\n");
    //     return 1;
    // }
    if (!strcmp(argv[1], "--add"))
    {
        addTreasure(argv[2]);
    }

    return 0;
}