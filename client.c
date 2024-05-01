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
#include "game.h"

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

	if(argc < 2){
        perror("Missing port");
        exit(1);
    }
    int SERVER_PORT = atoi(argv[1]);

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
			printf("La partie va commencer \n ");
		int *grid;
		grid = initGrid(); // sert uniquement pour des tests
		int emplacement;
	    char buffer[10]; // Taille suffisante pour stocker une entrée d'entier
		printf("La partie va commencer \n\n ");

			
		while (1) {
			
			for (int i = 0; i < GRID_LENGTH; i++){

				// Lire la tuile du serveur
				if (sread(sockfd, &tuileAuHasard, sizeof(int)) <= 0) {
					perror("Erreur de lecture du serveur");
					break; // Sortir de la boucle infinie
				}

				printf("Voici la tuile à placer : %d\n", tuileAuHasard);

				printf("\nVoici votre grille actuelle : \n");
				for (int i = 0; i < 20; i++)
				{
					printf("%d ",grid[i]);
				}
				printf("\n");
				
				

				printf("\nOù souhaitez vous placer a tuile ?\n");

				sread(0,buffer, sizeof(buffer)); //lecture de l'emplacement au clavier
				emplacement = atoi(buffer); // Convertit la chaîne de caractères en entier

				while (!placerTuile(&emplacement, tuileAuHasard, grid)){//vérifie l'emplacement dans la grille et place la tuile
					printf("Où souhaitez vous placer a tuile ?\n");
					sread(0,buffer, sizeof(buffer));
					emplacement = atoi(buffer); 
				}
				printf("----------------------------------------\n");

				// Envoyer l'emplacement au serveur
				if (swrite(sockfd, &emplacement, sizeof(int)) <= 0) {
					perror("Erreur d'écriture vers le serveur\n");
					break; // Sortir de la boucle infinie
				}
			}


		

			printf("envoie du score au server fils qu est de 10 \n");
			int score = 10; 
			swrite(sockfd,&score,sizeof(int));

			// Récupération de la taille 
			int size;
			sread(sockfd, &size, sizeof(int));

			// Allouer de la mémoire pour le tableau
			TabPlayer* newplayer = smalloc(size * sizeof(TabPlayer));

			// Recevoir le tableau
			for(int i = 0; i < size; i++) {
    			sread(sockfd, &(newplayer[i]), sizeof(TabPlayer));
				printf("score dans tabplayer : %d \n " , newplayer->tabPlayer->score);
			}

			afficherScores(newplayer->tabPlayer,size);
			

			
		
		
		}
		
		
	}

	printf("\nFin du jeu\n");
}

