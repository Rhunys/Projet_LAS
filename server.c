#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "message.h"
#include "utils_v1.h"
#include "ipc.h"

#define BACKLOG 5
#define TIME_INSCRIPTION 20
#define PERM 0666
#define KEY 123
#define TAILLE 80

typedef struct ServerChild
{
	int pipe[2];
	int pipe2[2];
	int sockfd;
} ServerChild;

/*** globals variables ***/
Player tabPlayers[MAX_PLAYERS];
ServerChild tabServerChild[MAX_PLAYERS];

volatile sig_atomic_t end_inscriptions = 0;

void endServerHandler(int sig)
{
	end_inscriptions = 1;
}

void disconnect_players(Player *tabPlayers, int nbPlayers)
{
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
int initSocketServer(int port)
{
	int sockfd = ssocket();

	// setsockopt -> to avoid Address Already in Use
	// to do before bind !
	int option = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));

	sbind(port, sockfd);
	slisten(sockfd, BACKLOG);

	return sockfd;
}

void run(void *argv, void *argv2, void *argv3)
{
	int *pipefd = argv;
	int *pipefd2 = argv2;
	int sockfd = *(int *)argv3;

	int tuileAuHasard;
	sclose(pipefd[1]);
	sclose(pipefd2[0]);
	int nbJoueurs = 0;
	StructMessage msg;
	int nbScore = 0;

	while (nbJoueurs < MAX_PLAYERS)
	{

		sread(pipefd[0], &tuileAuHasard, sizeof(int));
		printf("Valeur reçue par le parent : %d\n", tuileAuHasard);

		swrite(sockfd, &tuileAuHasard, sizeof(int));
		printf("Serveur envoie au client : %d\n", tuileAuHasard);

		int emplacement;

		sread(sockfd, &emplacement, sizeof(int));
		printf("Le client a décide de placer sa tuile en %d \n", emplacement);

		swrite(pipefd2[1], &emplacement, sizeof(int));
		printf("envoie de l'emplacement au server \n");

		nbJoueurs++;
	}
	sleep(5);

	while(nbScore < 2){	

		sread(sockfd, &msg, sizeof(msg)); // Récupération du msg start_score
		swrite(pipefd2[1],&msg,sizeof(msg));
		// Récupération du total 
		int score;
		sread(sockfd,&score,sizeof(int));
		// envoie du nombre de joeuru a l'enfant 
		swrite(sockfd,&nbJoueurs,sizeof(int));
		// envoie du score au parent 
		swrite(pipefd2[1],&score,sizeof(int));

		int semID = getSemaphore();
		sem_down0(semID);

		// Récupération de la mémoire partagée 
		TabPlayer * tabRankingSM = getSharedMemory();
		TabPlayer* tabRanking = malloc (nbJoueurs * sizeof(TabPlayer));

		if(!tabRanking){
			perror("ALLOCATION ERROR");
			exit(1);
		}
			
		for(int i=0 ; i<nbJoueurs ; i++){
			strcpy(tabRanking->tabPlayer->pseudo, tabRankingSM->tabPlayer->pseudo);
			tabRanking->tabPlayer->score= tabRankingSM->tabPlayer->score;
			swrite(sockfd,&tabRanking->tabPlayer,sizeof(TabPlayer)); 
		}

	}
	


}

int main(int argc, char **argv)
{
	int sockfd, newsockfd, i;
	StructMessage msg;
	struct pollfd fds[MAX_PLAYERS];

	ssigaction(SIGALRM, endServerHandler);

	sockfd = initSocketServer(SERVER_PORT);
	printf("Le serveur tourne sur le port : %i \n", SERVER_PORT);

	i = 0;
	int nbPLayers = 0;

	// CRÉATION DE LA MÉMOIRE PARTAGÉE 
	 int sharedMemory = createSharedMemory();
	TabPlayer *tablePlayerIPC = attacheSHM(sharedMemory);
	int semId = createSemaphore();


	// INSCRIPTION PART
	alarm(TIME_INSCRIPTION);

	while (!end_inscriptions)
	{

		newsockfd = accept(sockfd, NULL, NULL); // saccept() exit le programme si accept a été interrompu par l'alarme

		if (newsockfd > 0)
		{

			sread(newsockfd, &msg, sizeof(msg));

			if (msg.code == INSCRIPTION_REQUEST)
			{
				printf("Inscription demandée par le joueur : %s\n", msg.messageText);

				strcpy(tabPlayers[i].pseudo, msg.messageText);
				tabPlayers[i].sockfd = newsockfd;
				i++;

				if (nbPLayers < MAX_PLAYERS)
				{
					if (nbPLayers == MAX_PLAYERS)
					{
						alarm(0); // cancel alarm
						end_inscriptions = 1;
					}
					msg.code = INSCRIPTION_OK;
					nbPLayers++;
				}

				else
				{
					msg.code = INSCRIPTION_KO;
				}
				swrite(newsockfd, &msg, sizeof(msg));
				printf("Nb Inscriptions : %i\n", nbPLayers);
			}
		}
	}

	if (nbPLayers != MAX_PLAYERS)
	{
		printf("Partie annulée .. pas assez de joueurs\n");
		msg.code = CANCEL_GAME;
		for (i = 0; i < nbPLayers; i++)
		{
			swrite(tabPlayers[i].sockfd, &msg, sizeof(msg));
		}
		disconnect_players(tabPlayers, nbPLayers);
		sclose(sockfd);
		exit(0);
	}
	else
	{
		msg.code = START_GAME;
		for (i = 0; i < nbPLayers; i++)
			swrite(tabPlayers[i].sockfd, &msg, sizeof(msg));
	}

	if (msg.code == START_GAME)
	{
		printf("La partie va commencer \n");

		// CREATION DES TUILES 

		for (int i = 0; i < nbPLayers; i++)
		{
			printf("Création du server fils numéro : %d \n ", i);

			// 1/ Création du Pipe
			spipe(tabServerChild[i].pipe);
			spipe(tabServerChild[i].pipe2);

			fork_and_run3(run, tabServerChild[i].pipe, tabServerChild[i].pipe2, &tabPlayers[i].sockfd);

			sclose(tabServerChild[i].pipe[0]);
			sclose(tabServerChild[i].pipe2[1]);

			fds[i].fd = tabServerChild[i].pipe2[0];
			fds[i].events = POLLIN;
		}

		int nbJoueurs = 0;
		while (nbJoueurs < MAX_PLAYERS) //************* Faire boucle de 20 tours *****************
		{

			int j = 0;
			int tuileAuHasard = 20;
			spoll(fds, nbPLayers, 1);

			// Envoi de la tuile aléatoire à chaque processus enfant
			for (j = 0; j < nbPLayers; j++)
			{
				swrite(tabServerChild[j].pipe[1], &tuileAuHasard, sizeof(tuileAuHasard));
			}

			// Lecture de l'emplacement de chaque processus enfant
			for (j = 0; j < nbPLayers; j++)
			{

				if (fds[j].revents & POLLIN)
				{
					int placement;
					sread(tabServerChild[j].pipe2[0], &placement, sizeof(int));
					printf("le client place la tuile à l'emplacement %d\n", placement);
				}
			}
			nbJoueurs++;
		}
		
	}

	
	int nbScore = 0;

	while(nbScore < 2){

		for (int i = 0; i < nbPLayers; i++){
			sread(tabServerChild[i].pipe2[0],&msg, sizeof(msg));		
		}
		
		printf("code recu : %d \n " , msg.code);
		if(msg.code == START_SCORE){
			// RÉCUPÉRATION DU SCORE
			printf("PARTIE FINIE");


			sem_down0(semId);

			int score;
			// Lecture des score 
			for (int i = 0; i < nbPLayers; i++)
				{
					sread(tabServerChild[i].pipe2[0], &score, sizeof(int));	
					tabPlayers->score = score;
				}
			// **************Trier les scores avce fonction game *******************
			// Trier tabPlayers

			//Envoie du score a l'ipc 

			for(int i=0 ; i< nbPLayers; i++){
					strcpy(tablePlayerIPC->tabPlayer->pseudo ,tabPlayers[i].pseudo);
					tablePlayerIPC->tabPlayer->score = tabPlayers[i].score;
				}

			sem_up0(semId);

		}

	}
	

}
