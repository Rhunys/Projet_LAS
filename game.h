#ifndef _GAME_H
#define _GAME_H

#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/ipc.h>
#include <poll.h>

#include "message.h"
#include "utils_v1.h"

void initializeSacTuiles();
/**
 * PRE:  tabPlayers: a valid array of players
 *       nbPlayers: the number of players
 * POST: initializes the player grids
 */
int* initGrid();

/**
 * PRE:  tabPlayers: a valid array of players
 *       nbPlayers: the number of players
 * POST: initializes the player grids
 */
void initializeSacTuiles();

/**
 * PRE:  tabPlayers: a valid array of players
 *       nbPlayers: the number of players
 * POST: initializes the player grids
 */
void initPlayerGrids(Player *tabPlayers, int nbPlayers);

/**
 * Frees the memory allocated for the player grids.
 * 
 * PRE: tabPlayers   A pointer to an array of players.
 * POST: nbPlayers   The number of players.
 * 
 */
void freePlayerGrids(Player *tabPlayers, int nbPlayers);

/**
 * RES: A pseudo-random integer value 
 * in the range 1 to 30 corresponding to a tile
 */
int tuileAuHasard();

/**
 * PRE:  position: a valid position in the grid
 *       tuile: a valid tile
 *       grid: a valid grid
 * POST: places the tile at the given position in the grid
 * RES:  true if a tile has been placed, false otherwise
 */
bool placerTuile(int *position, int tuile, int *grid);

/**
 * PRE:  tabPlayers: a valid array of players
 *       nbPlayers: the number of players
 * POST: runs the game loop
 */
void gameLoop(Player *tabPlayers, int nbPlayers);

/**
 * PRE:  tabPlayers: a valid array of players
 *       nbPlayers: the number of players
 * POST: calculates the scores of the players
 */
void calculerScores(Player *tabPlayers, int nbPlayers);

/**
 * PRE:  tabPlayers: a valid array of players
 *       nbPlayers: the number of players
 * POST: displays the scores of the players
 */
void afficherScores(Player *tabPlayers, int nbPlayers);

/**
 * PRE:  tabPlayers: a valid array of players
 *       nbPlayers: the number of players
 * POST: displays the scores of the players
 */
char* afficherClassement(Player *tabPlayers, int nbPlayers);

/**
 * PRE:  a: a pointer to a player
 *       b: a pointer to a player
 * POST: compares the scores of two players
 * RES:  a positive value if a > b, a negative value if a < b, 0 otherwise
 */
int comparePlayers(const void *a, const void *b);



#endif //_GAME_H