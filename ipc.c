#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

#include "utils_v1.h"
#include "message.h" 

#define KEY 123 
#define SEM_KEY 369
#define PERM 0666 // lire et ecrire mais pas éxécuter 



int createSharedMemory(){
     int shim_id = sshmget(KEY, sizeof(TabPlayer), IPC_CREAT | PERM); 
     return shim_id;
} 

TabPlayer* getSharedMemory(){
    int shm_id = sshmget(KEY, sizeof(TabPlayer), 0);
     return (TabPlayer *)sshmat(shm_id); // pas de perm dans ce cas ci donc 0
}

void writeSharedMemory(int player, int score ){

    // Récupération de la mémoire partagée 
    TabPlayer *tab = getSharedMemory();
    tab->tabPlayer[player].score = score;

    sshmdt(tab);
}

TabPlayer* attacheSHM(int shm_id){
    TabPlayer* z = sshmat(shm_id);
    return z; 
}

// delete memory 
void deleteMemory(int shm_id){
    sshmdelete(shm_id); // supression de la mémoire partagée, idem que ipcrm -m
}

// create semaphore 
int createSemaphore(){
   return sem_create(SEM_KEY, 1, PERM, 0);//initialise a 0 
    
}

// get semaphore
int getSemaphore(){
    return sem_get(SEM_KEY,1);
}