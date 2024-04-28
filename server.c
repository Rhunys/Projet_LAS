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
#define TIME_INSCRIPTION 20
#define PERM 0666 
#define KEY 123
#define TAILLE 80


typedef struct Player{
	char pseudo[MAX_PSEUDO];
	int sockfd;
	int shot;
} Player;

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

void run(void * argv, void * argv2, void * argv3){
	int *pipefd = argv;
	int *pipefd2 = argv2;
	int sockfd = *(int *)argv3;
			 
	int tuileAuHasard; 

	sread(pipefd[0], &tuileAuHasard, sizeof(tuileAuHasard));
	printf("Valeur reçue par le parent : %d\n", tuileAuHasard);


	swrite(sockfd,&tuileAuHasard,sizeof(int));
	printf("Serveur envoie au client : %d\n", tuileAuHasard);

	sleep(2);

	sclose(pipefd[0]);

	int emplacement; 

	sread(sockfd,&emplacement,sizeof(int));
	printf("Le client a décide de placer sa tuile en %d \n", emplacement);

	swrite(pipefd2[1],&emplacement,sizeof(int));
	printf("envoie de l'emplacement au server");

	sclose(pipefd2[1]);

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

		// filedes[0] -> lire le pipe
		// filedes[1] -> écrire sur pipe
		for(int i = 0 ; i < MAX_PLAYERS; i ++){
			
			while (1)
			{
				printf("Création du server fils numéro : %d \n " , i);
			// 1/ Création du Pipe 
			int pipefd[2];
			spipe(pipefd); 

			int pipefd2[2];
			spipe(pipefd2);
			
			fork_and_run3(run, pipefd, pipefd2,&tabPlayers[i].sockfd);

			sclose(pipefd[0]);

			int tuileAuHasard = 20; 

			nwrite(pipefd[1], &tuileAuHasard, sizeof(tuileAuHasard));

			sclose(pipefd[1]);

			sclose(pipefd2[1]);
			int placement;
			sread(pipefd2[0], &placement, sizeof(int));
			printf("le client place la tuile à l'emplacement %d\n", placement);

			sclose(pipefd2[0]);
			}
			
			
		}
		sleep(5);

	}
	
}
	