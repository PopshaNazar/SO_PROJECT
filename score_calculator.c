
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char id[256];
    char username[256];
    float latitude;
    float longitude;
    char clue[256];
    int value;
} Treasure;

typedef struct
{
    char username[256];
    int total_score;
} UserScore;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <hunt_name>\n", argv[0]);
        return 1;
    }

    char path[512];
    snprintf(path, sizeof(path), "hunts/%s/treasure.dat", argv[1]);

    FILE *file = fopen(path, "rb");
    if (!file)
    {
        perror("Error opening treasure file");
        return 1;
    }

    Treasure t;
    UserScore scores[100];
    int user_count = 0;

    while (fread(&t, sizeof(Treasure), 1, file))
    {
        int found = 0;
        for (int i = 0; i < user_count; i++)
        {
            if (strcmp(scores[i].username, t.username) == 0)
            {
                scores[i].total_score += t.value;
                found = 1;
                break;
            }
        }

        if (!found && user_count < 100)
        {
            strcpy(scores[user_count].username, t.username);
            scores[user_count].total_score = t.value;
            user_count++;
        }
    }

    fclose(file);

    printf("Scores for hunt '%s':\n", argv[1]);
    printf("User | Score\n");
    printf("--------------------\n");
    for (int i = 0; i < user_count; i++)
    {
        printf("%s | %d\n", scores[i].username, scores[i].total_score);
    }
    printf("--------------------\n");

    return 0;
}