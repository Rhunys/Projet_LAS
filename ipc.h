#ifndef IPC_H
#define IPC_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

#include "utils_v1.h"
#include "message.h"

#define KEY 123 
#define PERM 0666 // lire et ecrire mais pas éxécuter 




int createSharedMemory();

// Renvoie un pointeur vers une structure de données partagée TabPlayer
// Post : Le pointeur renvoyé pointe vers la mémoire partagée existante
TabPlayer* getSharedMemory();

// Écrit le score d'un joueur dans la mémoire partagée
// Pré : player est un index valide dans la structure TabPlayer (0 <= player < N)
// Pré : score est un entier représentant le score du joueur
// Post : Le score du joueur player est mis à jour dans la mémoire partagée
void writeSharedMemory(int player, int score);

// Attache une mémoire partagée identifiée par shmID et renvoie un pointeur vers la structure de données TabPlayer
// Pré : shmID est l'identifiant d'une mémoire partagée valide
// Post : Le pointeur renvoyé pointe vers la mémoire partagée attachée
TabPlayer* attacheSHM(int shmID);

// Supprime une mémoire partagée identifiée par shm_id
// Pré : shm_id est l'identifiant d'une mémoire partagée valide
// Post : La mémoire partagée identifiée par shm_id est supprimée
void deleteMemory(int shm_id);

// Crée un sémaphore et renvoie son identifiant
// Post : Le sémaphore est créé et initialisé à 0
int createSemaphore();

// Renvoie l'identifiant d'un sémaphore existant
// Post : L'identifiant du sémaphore est renvoyé
int getSemaphore();


#endif /* IPC_H */
