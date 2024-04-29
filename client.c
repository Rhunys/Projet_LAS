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
		int *grid;
		grid = initGrid(); // sert uniquement pour des tests
		int emplacement;
	    char buffer[10]; // Taille suffisante pour stocker une entrée d'entier

	    while(1){ 
			printf("La partie va commencer \n ");
			
			// Lire la tuile du serveur
			if (sread(sockfd, &tuileAuHasard, sizeof(int)) <= 0) {
				perror("Erreur de lecture du serveur");
				break; // Sortir de la boucle infinie
			}

			/*// Lire grid du serveur
			if (sread(sockfd, &grid, sizeof(grid)) <= 0) {
				perror("Erreur de lecture du serveur (grid)");
				break; // Sortir de la boucle infinie
			}*/

			printf("Client reçoit du serveur : %d\n", tuileAuHasard);

			printf("voici votre grille : \n");
			for (int i = 0; i < 20; i++)
			{
				printf("%d ",grid[i]);
			}
			printf("\n");
			
			

			printf("Où souhaitez vous placer a tuile ?\n");

			sread(0,buffer, sizeof(buffer)); //lecture de l'emplacement au clavier
			emplacement = atoi(buffer); // Convertit la chaîne de caractères en entier

			while (!placerTuile(emplacement, tuileAuHasard, grid)){//vérifie l'emplacement dans la grille et place la tuile
				printf("Où souhaitez vous placer a tuile ?\n");
				sread(0,buffer, sizeof(buffer));
				emplacement = atoi(buffer); 
			}
			

			printf("Le client choisit de poser sa tuile à %d \n", emplacement);

			// Envoyer l'emplacement au serveur
			if (swrite(sockfd, &emplacement, sizeof(int)) <= 0) {
				perror("Erreur d'écriture vers le serveur");
				break; // Sortir de la boucle infinie
			}
		}
	}
}
