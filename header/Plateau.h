#ifndef PLATEAU_H
#define PLATEAU_H

#include "Carte.h"


struct Plateau//meme si que 1 attribut, c'est mieux parce que ca evite de faire un PaquetCartes*** et c'est plus facile de travailler avec * que *** 
{
    struct PaquetCartes** lignes;
};

struct Plateau* creerPlateau();
void plateauToStr(struct Plateau*, char*);
void strToPlateau(struct Plateau*, char*);
void detruitPlateau(struct Plateau*);
void affichePlateau(struct Plateau*);
int ajouteCarteLigne(struct Plateau*, struct Carte, int);
int carteMin(struct Plateau*);
int minTete(struct Plateau*);

#endif