#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "message.h"
#include "utils_v1.h"

#define MAX_PLAYERS 2
#define BACKLOG 5
#define TIME_INSCRIPTION 30
#define PERM 0666 
#define KEY 123
#define TAILLE 80

/*** globals variables ***/
Player tabPlayers[MAX_PLAYERS];
volatile sig_atomic_t end_inscriptions = 0;

void endServerHandler(int sig){
	end_inscriptions = 1;
}

void disconnect_players(Player *tabPlayers, int nbPlayers){
	for (int i = 0; i < nbPlayers; i++)
		sclose(tabPlayers[i].sockfd);
	return;
}


/**
 * PRE:  serverPort: a valid port number
 * POST: on success, binds a socket to 0.0.0.0:serverPort and listens to it ;
 *       on failure, displays error cause and quits the program
 * RES:  return socket file descriptor
 */
int initSocketServer(int port){
	int sockfd = ssocket();

	// setsockopt -> to avoid Address Already in Use
	// to do before bind !
	int option = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));

	sbind(port, sockfd);
	slisten(sockfd, BACKLOG);

	return sockfd;
}

int main(int argc, char **argv){
	int sockfd, newsockfd, i;
	StructMessage msg;
	//struct pollfd fds[MAX_PLAYERS];
	// char winnerName[256];
	
	ssigaction(SIGALRM, endServerHandler);

	sockfd = initSocketServer(SERVER_PORT);
	printf("Le serveur tourne sur le port : %i \n", SERVER_PORT);

	// GET SHARED MEMORY 
	//int shm_id = sshmget(KEY, sizeof(int), IPC_CREAT | PERM); 
	//int* z = sshmat(shm_id);

	i = 0;
	int nbPLayers = 0;


	// INSCRIPTION PART
	alarm(TIME_INSCRIPTION);

	while (!end_inscriptions){
		
		newsockfd = accept(sockfd, NULL, NULL);// saccept() exit le programme si accept a été interrompu par l'alarme
		
		if (newsockfd > 0){

			sread(newsockfd, &msg, sizeof(msg));

			if (msg.code == INSCRIPTION_REQUEST){
				printf("Inscription demandée par le joueur : %s\n", msg.messageText);

				strcpy(tabPlayers[i].pseudo, msg.messageText);
				tabPlayers[i].sockfd = newsockfd;
				i++;

				if (nbPLayers < MAX_PLAYERS){
					if (nbPLayers == MAX_PLAYERS){
						alarm(0); // cancel alarm
						end_inscriptions = 1;
					}
					msg.code = INSCRIPTION_OK;
					nbPLayers++;	
				}
				 
				else{
					msg.code = INSCRIPTION_KO;
				}
				swrite(newsockfd, &msg, sizeof(msg));
				printf("Nb Inscriptions : %i\n", nbPLayers);
			}
		}
	}

	if (nbPLayers != MAX_PLAYERS){
		printf("Partie annulée .. pas assez de joueurs\n");
		msg.code = CANCEL_GAME;
		for (i = 0; i < nbPLayers; i++){
			swrite(tabPlayers[i].sockfd, &msg, sizeof(msg));
		}
		disconnect_players(tabPlayers, nbPLayers);
		sclose(sockfd);
		exit(0);
	}
	else{
		msg.code = START_GAME;
		for (i = 0; i < nbPLayers; i++)
			swrite(tabPlayers[i].sockfd, &msg, sizeof(msg));
	}

	if(msg.code == START_GAME){
		printf("La partie va commencer \n");

		// 1/ Création du Pipe 
		int pipefd[2];
		spipe(pipefd); 

		// Creation des serveurs fils 
		int childId = sfork();

		

		// filedes[0] -> lire le pipe
		// filedes[1] -> écrire sur pipe

		if(childId > 0){

			sclose(pipefd[0]);

			int tuileAuHasard = 20; 

			nwrite(pipefd[1], &tuileAuHasard, sizeof(tuileAuHasard));

			sclose(pipefd[1]);


		}
		else{

			sclose(pipefd[1]);
			 
			int tuileAuHasard; 

			sread(pipefd[0], &tuileAuHasard, sizeof(tuileAuHasard));
			printf("Valeur reçue par l'enfant : %d\n", tuileAuHasard);

			sclose(pipefd[0]);


		}

	}
	
}
	
