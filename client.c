#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "message.h"
#include "utils_v1.h"

/**
 * PRE: serverIP : a valid IP address
 *      serverPort: a valid port number
 * POST: on success, connects a client socket to serverIP:serverPort ;
 *       on failure, displays error cause and quits the program
 * RES: return socket file descriptor
 */
int initSocketClient(char *serverIP, int serverPort)
{
	int sockfd = ssocket();
	sconnect(serverIP, serverPort, sockfd);
	return sockfd;
}

int main(int argc, char **argv){

	char pseudo[MAX_PSEUDO];
	int sockfd;
	int ret;
	int tuileAuHasard;

	StructMessage msg;
	

	/* retrieve player name */
	printf("Bienvenue dans le programe d'inscription au serveur de jeu\n");
	printf("Pour participer entrez votre nom :\n");
	ret = sread(0, pseudo, MAX_PSEUDO);
	checkNeg(ret, "read client error");
	pseudo[ret - 1] = '\0';
	strcpy(msg.messageText, pseudo);
	msg.code = INSCRIPTION_REQUEST;

	sockfd = initSocketClient(SERVER_IP, SERVER_PORT);
	
	swrite(sockfd, &msg, sizeof(msg));


	/* wait server response */
	sread(sockfd, &msg, sizeof(msg));

	switch (msg.code){

	case INSCRIPTION_OK:
		printf("Réponse du serveur : Inscription acceptée\n");
		break;
	case INSCRIPTION_KO:
		printf("Réponse du serveur : Inscription refusée\n");
		sclose(sockfd);
		exit(0);
	default:
		printf("Réponse du serveur non prévue %d\n", msg.code);
		break;
	}


	/* wait start of game or cancel */
	sread(sockfd, &msg, sizeof(msg));
	
	if (msg.code == START_GAME){
		int nbJoueurs= 0;
		int joueurRanking = 0; 
			
		while (nbJoueurs < 2) {
			printf("La partie va commencer \n ");
			
			// Lire la tuile du serveur
			if (sread(sockfd, &tuileAuHasard, sizeof(int)) <= 0) {
				perror("Erreur de lecture du serveur");
				break; 
			}

			printf("Client reçoit du serveur : %d\n", tuileAuHasard);

			int emplacement = 2;
			printf("Le client choisit de poser sa tuile à %d \n", emplacement);

			// Envoyer l'emplacement au serveur
			if (swrite(sockfd, &emplacement, sizeof(int)) <= 0) {
				perror("Erreur d'écriture vers le serveur");
				break; 
			}
			nbJoueurs++;
		}
		while(joueurRanking < 2){ // Tant que les deux joueurs n'ont pas recu leurs score 

			printf("envoie du code start score");
			msg.code = START_SCORE;
			swrite(sockfd, &msg, sizeof(msg));
			printf("message.code ; attentdu : MONSCORE : %d\n ", msg.code);

			printf("envoie du score au server fils qu est de 10 ");
			int score = 10; 
			swrite(sockfd,&score,sizeof(int));

			int size;   
			sread(sockfd,&size,sizeof(size));
					
			TabPlayer * players =  malloc (size * sizeof(TabPlayer));
			if(!players){
				perror("ALLOCATION ERROR");
				exit(1);
			}

			for (int i = 0; i < size; ++i){
			TabPlayer* newplayer;
			sread(sockfd,&newplayer,sizeof(TabPlayer));
			strcpy(players->tabPlayer->pseudo,newplayer->tabPlayer->pseudo);
			players->tabPlayer->score = newplayer->tabPlayer->score;
			}
			//   printRanking(players,size); **** AFFICHER RANKING METHODE GAME.C *****
		
		}

		

	}

	

	
	
}
