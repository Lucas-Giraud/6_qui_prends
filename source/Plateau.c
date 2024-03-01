#include "Plateau.h"


struct Plateau* creerPlateau()//initialise le plateau en lui allouant tous les espaces memoires necessaires
{
    struct Plateau* p = (struct Plateau*) malloc(sizeof(struct Plateau));
    p->lignes = (struct PaquetCartes**) malloc(4*sizeof(struct PaquetCartes*));
    for(int i=0; i<4; i++){
        p->lignes[i] = creerPaquet(5);
    }
    return p;
}

void detruitPlateau(struct Plateau* p)//detruit le plateau en liberant tous les espaces memoires qu'il poossede
{
    for(int i=0; i<4; i++)
        detruitPaquet(p->lignes[i]);
    free(p->lignes);
    free(p);
}

int ajouteCarteLigne(struct Plateau* p, struct Carte c, int ligne)//ajoute la carte 'c' à la ligne 'ligne' du plateau 'p'
{
    int recupTete = 0;
    struct PaquetCartes* pc = p->lignes[ligne];
    if(pc->taille <5)//si il y a moins de 5 cartes dans la ligne, place la carte 
    {
        ajouteCarte(pc,c);
    }
    else
    {//sinon il s'agit de la sixième carte donc récupère le nombre de têtes de la ligne et la remplace par la carte jouée
        recupTete = pc->nbTete;
        videPaquet(pc);
        ajouteCarte(pc,c);
    }
    return recupTete;
}

void affichePlateau(struct Plateau* p)//affiche chaque ligne avec son nombre de tete et son indice
{
    for(int i=0; i<4; i++)
    {
        affichePaquet(p->lignes[i]);
        for(int j=5; j>(p->lignes[i]->taille) ; j--)
            printf("       ");
        printf("(%d)   %d.\n", p->lignes[i]->nbTete, i);
    }
    printf("\n");
}

void plateauToStr(struct Plateau* p, char* str)//transforme le plateau 'p' en une chaine de caractère
{
    for(int i=0; i<4; i++)
    {//transforme chaque ligne en une chaine de caractère et les sépare par un 's'
        struct PaquetCartes* pc = p->lignes[i];
        paquetToStr(pc, str);
        strcat(str, "s");//s comme séparateur
    }
}

void strToPlateau(struct Plateau* p, char* str)//transforme la chaine de caractère 'str' en un plateau
{
    char d1[] = "s";//délimiteur 1
    char* recupPlat[4];
    char* recupLigne = strtok(str, d1);//récupère les caractère se trouvant avant le délimiteur d1 

    for(int i=0; i<4; i++)
    {
        recupPlat[i] = recupLigne;//stocke chaque ligne dans un tableau de chaine de caractère
        recupLigne = strtok(NULL, d1);  
    }

    for(int i=0; i<4; i++)//obligé de faire 2 boucle for sinon strtok est cassé
    {
        strToPaquet(p->lignes[i], recupPlat[i]);//transforme chaque chaine de cractère récupérée en un paquet de carte représentant une ligne
    }
}

int carteMin(struct Plateau* p)//récupère la valeur de la carte la plus petite parmis les dernières cartes de chaque ligne
{
    int minVal = 105;//la plus grand carte est 104 donc on trouvera forcément plus petit que 105
    for(int i=0; i<4; i++)
    {
        struct PaquetCartes* pc = p->lignes[i];
        if(minVal > pc->paquet[pc->taille-1].valeur)
            minVal = pc->paquet[pc->taille-1].valeur;
    }
    return minVal;
}

int minTete(struct Plateau* p)//récupère le plus petit nombre de têtes parmis chaque lignes du plateau
{
    int minTete = 99;
    for(int i=0; i<4; i++)
    {
        if(minTete > p->lignes[i]->nbTete)
            minTete = p->lignes[i]->nbTete;
    }
    return minTete;
}

/*
int main()//test des fonctions de la classe
{
    struct Carte c;
    c.valeur = 2;
    c.nbTete = 5;

    struct Carte c2;
    c2.valeur = 22;
    c2.nbTete = 52;

    struct Carte c3;
    c3.valeur = 23;
    c3.nbTete = 53;

    struct Carte c4;
    c4.valeur = 24;
    c4.nbTete = 54;
    
    struct Carte c5;
    c5.valeur = 25;
    c5.nbTete = 55;
    
    struct Carte c6;
    c6.valeur = 26;
    c6.nbTete = 56;

    struct Plateau* p = creerPlateau();
    //affichePlateau(p);
    ajouteCarteLigne(p,c,0);
    //affichePlateau(p);
    ajouteCarteLigne(p,c2,1);
    //affichePlateau(p);
    ajouteCarteLigne(p,c3,1);
    //affichePlateau(p);
    ajouteCarteLigne(p,c4,2);
    //affichePlateau(p);
    ajouteCarteLigne(p,c5,2);
    //affichePlateau(p);
    ajouteCarteLigne(p,c6,3);
    //affichePlateau(p);

    char str[140];
    plaToStr(p, str);

    struct Plateau* p1 = creerPlateau();
    strToPla(p1,str);

    affichePlateau(p1);
    
    detruitPlateau(p);
    detruitPlateau(p1);

    return 0;
}*/