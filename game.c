#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "game.h"
#include "utils_v1.h"

#define GRID_LENGTH 20
#define TURNS 20
#define TILES_MIN 1
#define TILES_MAX 31

int *sacTuiles;
int tuilesRestantes;
int pointsSerie[20] = {0, 1, 3, 5, 7, 9, 11, 15, 20, 25, 30, 35, 40, 50, 60, 70, 85, 100, 150, 300};

// Initialize the bag of tiles
void initializeSacTuiles()
{
    sacTuiles = malloc(TILES_MAX * sizeof(int));
    tuilesRestantes = TILES_MAX;

    // Fill the bag with 1 except index 10 to 18 with 2
    for (int i = 0; i < TILES_MAX; i++)
    {
        if (i >= 10 && i <= 18)
        {
            sacTuiles[i] = 2;
        }
        else
        {
            sacTuiles[i] = 1;
        }
    }
}

// Game loop
int main(int argc, char *argv[])
{
    initializeSacTuiles();
    Player player1;
    player1.grid = initGrid();
    Player player2;
    player2.grid = initGrid();
    Player tabPlayers[2] = {player1, player2};
    gameLoop(tabPlayers, 2);
    return 0;
}
void gameLoop(Player *tabPlayers, int nbPlayers)
{
    initPlayerGrids(tabPlayers, nbPlayers);
    
    for (int i = 0; i < TURNS; i++)
    {   
        int tuile = tuileAuHasard();
        int nbPlayersAlreadyPlayed = 0;

        while (nbPlayersAlreadyPlayed < nbPlayers)
        {
            Player player = tabPlayers[nbPlayersAlreadyPlayed];
            nbPlayersAlreadyPlayed++;

            if(tuile == 31){
                printf("\nTuile à placer : Joker\n");
            } else {
                printf("\nTuile à placer : %d\n", tuile);
            }

            printf("Grille actuelle (Joueur %d): ", nbPlayersAlreadyPlayed);

            for (int j = 0; j < GRID_LENGTH; j++)
            {   
                printf("%d ", player.grid[j]);
            }

            printf("\n");

            int position;
            printf("Entrez la position où placer la tuile : ");
            scanf("%d", &position);
            while(!placerTuile(position, tuile, player.grid)){
                printf("Entrez la position où placer la tuile : ");
                scanf("%d", &position);
            };

            for (int j = 0; j < GRID_LENGTH; j++)
            {   
                printf("%d ", player.grid[j]);
            }
            printf("\n");
        }
        
    }
   
    printf("\nFin du jeu\n");
    calculerScores(tabPlayers, nbPlayers);
    afficherScores(tabPlayers, nbPlayers);

    char *classement = afficherClassement(tabPlayers, nbPlayers);
    printf("%s\n", classement);

    freePlayerGrids(tabPlayers, nbPlayers);
}

int *initGrid()
{
    int *grid = (int *)malloc(GRID_LENGTH * sizeof(int));

    for (int i = 0; i < GRID_LENGTH; i++)
    {
        grid[i] = 0;
    }

    return grid;
}

void initPlayerGrids(Player *tabPlayers, int nbPlayers)
{
    for (int i = 0; i < nbPlayers; i++)
    {
        tabPlayers[i].grid = initGrid();
    }
}

void freePlayerGrids(Player *tabPlayers, int nbPlayers)
{
    for (int i = 0; i < nbPlayers; i++)
    {
        free(tabPlayers[i].grid);
    }
    free(sacTuiles);
}

int tuileAuHasard()
{
    if (tuilesRestantes == 0)
    {
        printf("Il n'y a plus de tuiles dans le sac\n");
        return -1;
    }

    int tuile = randomIntBetween(TILES_MIN, TILES_MAX);

    if (sacTuiles[tuile] > 0)
    {
        sacTuiles[tuile]--;
        tuilesRestantes--;
        return tuile;
    }
    else
    {
        return tuileAuHasard();
    }
}

bool placerTuile(int position, int tuile, int *grid)
{
    if (position <= 0 || position > GRID_LENGTH)
    {
        printf("\nPosition invalide ! Veuillez introduire un nombre entre 1 et 20\n");
        return false;
    }

    for (int i = position-1; i < GRID_LENGTH; i++)
    {
        if (grid[i] == 0)
        {
            grid[i] = tuile;
            printf("Tuile placée à la position %d\n", i + 1);
            return true;
        }

        if (i == GRID_LENGTH)
        {
            i = 0;
        }
    }
    return true;
}

void calculerScores(Player *tabPlayers, int nbPlayers)
{   
    printf("\nCalcul des scores\n");
    for (int i = 0; i < nbPlayers; i++)
    {
        Player player = tabPlayers[i];
        player.score = 0;
        int serie = 1;
        for (int j = 1; j < GRID_LENGTH; j++)
        {   
            if (player.grid[j] >= player.grid[j - 1])
            {
                serie++;
            } else {
                player.score += pointsSerie[serie-1];
                serie = 1;
            }
        }
        player.score += pointsSerie[serie-1];
        tabPlayers[i] = player;
    }
}

void afficherScores(Player *tabPlayers, int nbPlayers)
{
    for (int i = 0; i < nbPlayers; i++)
    {
        printf("\nScore du joueur %d : %d\n", i+1, tabPlayers[i].score);
    }
}

char* afficherClassement(Player *tabPlayers, int nbPlayers)
{
    Player *tabPlayersCopy = malloc(nbPlayers * sizeof(Player));
    memcpy(tabPlayersCopy, tabPlayers, nbPlayers * sizeof(Player));
    qsort(tabPlayersCopy, nbPlayers, sizeof(Player), comparePlayers);
    char *classement = malloc(100 * sizeof(char));
    strcpy(classement, "Classement :\n");
    for (int i = 0; i < nbPlayers; i++)
    {
        char *playerScore = malloc(100 * sizeof(char));
        sprintf(playerScore, "Joueur %d : %d\n", i+1, tabPlayersCopy[i].score);
        strcat(classement, playerScore);
        free(playerScore);
    }
    free(tabPlayersCopy);
    return classement;
}

int comparePlayers(const void *a, const void *b)
{
    Player *playerA = (Player *)a;
    Player *playerB = (Player *)b;
    return playerB->score - playerA->score;
}