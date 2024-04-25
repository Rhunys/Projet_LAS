#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/ipc.h>
#include <poll.h>

/**
 * RES: A pseudo-random integer value 
 * in the range 1 to 30 corresponding to a tile
 */
int tuileAuHasard();

/**
 * RES: A pointer to an array of integers of size GRID_LENGTH
 * initialized with 0
 */
int* initGrid();

/**
 * PRE:  position: a valid position in the grid
 *       tuile: a valid tile
 *       grid: a valid grid
 * POST: places the tile at the given position in the grid
 * RES:  the updated grid
 */
int* placerTuile(int position, int tuile, int *grid);

/**
 * PRE:  tabPlayers: a valid array of players
 *       nbPlayers: the number of players
 * POST: runs the game loop
 */
void gameLoop(Player *tabPlayers, int nbPlayers);

