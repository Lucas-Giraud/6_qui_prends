#ifndef CARTE_H
#define CARTE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct Carte//structure représantant une carte avec sa valeur et son nombre de tete de boeuf
{
    int valeur;
    int nbTete;
};

struct PaquetCartes//structure représantant un paquet de carte par un tableau de carte, la taille de ce paquet, et le nombre de tête qu'il contient
{
    struct Carte* paquet;
    int taille;
    int nbTete;
};

struct PaquetCartes* creerPaquet(int);
struct PaquetCartes* creerPioche();
struct Carte retireCarte(struct PaquetCartes*, int);
void ajouteCarte(struct PaquetCartes*, struct Carte);
void paquetToStr(struct PaquetCartes*, char*);
void strToPaquet(struct PaquetCartes*, char*);
void detruitPaquet(struct PaquetCartes*);
void affichePaquet(struct PaquetCartes*);
void videPaquet(struct PaquetCartes*);
void afficheCarte(struct Carte);

#endif