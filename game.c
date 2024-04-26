#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "utils_v1.h"
#define GRID_LENGTH 20
#define TURNS 20

// Game loop
void gameLoop(Player *tabPlayers, int nbPlayers)
{   
    // init poll
    for (i = 0; i < nbPlayers; i++){
        fds[i].fd = tabPlayers[i].sockfd;
        fds[i].events = POLLIN;
    }

    initPlayerGrids(tabPlayers, nbPlayers);
    int nbPlayersAlreadyPlayed = 0;

    for (int i = 0; i < TURNS; i++)
    {   
        int tuile = tuileAuHasard();

        while (nbPlayersAlreadyPlayed < nbPlayers){
            printf("Tuile à placer : %d\n", tuile);
            printf("Grille actuelle : ");

            for(int j = 0; j < GRID_LENGTH; j++){
                printf("%d ", grid[j]);
            }

            printf("\n");

            printf("Entrez la position où placer la tuile : ");
            scanf("%d", &position);
            placerTuile(position, tuile, grid);
        }
        nbPlayersAlreadyPlayed++;
    }

    freePlayerGrids(tabPlayers, nbPlayers);

    /*
        while (nbPlayersAlreadyPlayed < MAX_PLAYERS)
    {
        // poll during 1 second
        ret = poll(fds, MAX_PLAYERS, 1000);
        checkNeg(ret, "server poll error");

        if (ret == 0)
            continue;

        // check player something to read
        for (i = 0; i < MAX_PLAYERS; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                ret = sread(tabPlayers[i].sockfd, &msg, sizeof(msg));
                // tester si la connexion du client a été fermée: close(sockfd) ==> read renvoie 0
                // OU utiliser un tableau de booléens fds_invalid[i] pour indiquer
                // qu'un socket a été traité et ne doit plus l'être (cf. exemple19_avec_poll)
                // printf("poll detected POLLIN event on client socket %d (%s)... %s", tabPlayers[i].sockfd, tabPlayers[i].pseudo, ret == 0 ? "this socket is closed!\n" : "\n");

                if (ret != 0)
                {
                    tabPlayers[i].shot = msg.code;
                    printf("%s joue %s\n", tabPlayers[i].pseudo, codeToStr(msg.code));
                    nbPlayersAlreadyPlayed++;
                }
            }
        }
    }

    disconnect_players(tabPlayers, nbPLayers);
    sclose(sockfd);
    */

    return 0;
}

int tuileAuHasard(){
    return randomIntBetween(1, 30);
}

int *initGrid(){
    int *grid = (int *)malloc(GRID_LENGTH * sizeof(int));

    fori(i, 0, GRID_LENGTH)
    {
        grid[i] = 0;
    }

    return grid;
}

void initPlayerGrids(Player *tabPlayers, int nbPlayers){
    for (int i = 0; i < nbPlayers; i++)
    {
        tabPlayers[i].grid = initGrid();
    }
}

void freePlayerGrids(Player *tabPlayers, int nbPlayers){
    for (int i = 0; i < nbPlayers; i++)
    {
        free(tabPlayers[i].grid);
    }
}

bool placerTuile(int position, int tuile, int *grid){
    if (position < 0 || position >= GRID_LENGTH)
    {
        printf("Position invalide\n");
        return false;
    }

    bool loop = false;

    for (i = position; i < GRID_LENGTH; i++)
    {
        if (grid[i] == 0)
        {
            grid[i] = tuile;
            printf("Tuile placée à la position %d\n", i);
            return true;
        }

        if (i == GRID_LENGTH && !loop)
        {
            i = 0;
            loop = true;
        }

        if (i == position && loop)
        {
            printf("La grille est remplie !\n");
            return false;
        }
    }
    return false;
}