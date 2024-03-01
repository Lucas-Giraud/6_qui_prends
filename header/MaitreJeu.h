#ifndef MAITREJEU_H
#define MAITREJEU_H

#include "Plateau.h"

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

struct MaitreJeu//structure représentant le Maitre du Jeu
{
    struct Plateau* plateau;
    struct PaquetCartes* pioche;
    int nbJoueur;
    struct PaquetCartes* cartesJouees;//tableau stockant les cartes jouées à chaque tour
    int* nbTeteRecup;//tableau stockant temporairement les têtes récuprées par les joueurs
    int* score;//tableau contenant les scores de chaque joueur
    int joueur1;//premier joueur à avoir posé sa carte
    int manche;//représent la manche actuelle
    int tour;//représente le tour actuel
    bool scoreAtteint;//false si aucun score n'est supérieur ou égal à 66 et true sinon
};

struct parametre_th//structure contenant les paramètres à donner au thread
{
    struct MaitreJeu* MJ;
    int* socket;
    int idJoueur;
    int joueurTot;
    int mancheTot;
};

struct PaquetCartes* distribution(struct MaitreJeu*);
struct MaitreJeu* creerMJ();
void* thJoueur(void*);
void attendCarte(struct parametre_th, char*);
void scoreToStr(struct MaitreJeu*, char*);
void initPlateau(struct MaitreJeu*);
void detruitMj(struct MaitreJeu*);
void fin_66(struct MaitreJeu*);
void error(char *);
int joueCartes(struct parametre_th);

#endif