#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#define SERVER_PORT 9520
#define SERVER_IP "127.0.0.1" /* localhost */
#define MAX_PSEUDO 256
#define MAX_PLAYERS 2

#define INSCRIPTION_REQUEST 10
#define INSCRIPTION_OK 11
#define INSCRIPTION_KO 12
#define START_GAME 13
#define CANCEL_GAME 14
#define START_SCORE 15

/* struct message used between server and client */
typedef struct
{
  char messageText[MAX_PSEUDO];
  int code;
} StructMessage;

/* struct player used in the game */
typedef struct{
	char pseudo[MAX_PSEUDO];
	int sockfd;
  int* grid;
  int score;
} Player;

typedef struct{
  Player tabPlayer[MAX_PLAYERS];
  int nbPlayer;
}TabPlayer;

#endif
