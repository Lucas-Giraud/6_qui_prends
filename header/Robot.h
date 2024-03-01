#ifndef ROBOT_H
#define ROBOT_H

#include "Plateau.h"

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <pthread.h>

struct parametre_th  //structure contenant les paramètres à donner au thread
{
    struct PaquetCartes* pMain;
    struct Plateau* plateau;
    int socket;
    bool termine;
};

void* Joue(void*);
void error(char*);
int choixCarte(struct PaquetCartes*, struct Plateau*);


#endif 