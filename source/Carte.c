#include "Carte.h"

void afficheCarte(struct Carte c)// affiche la valeur de la carte 'c'
{
    if(c.valeur/100 > 0)// 3 affichages différents pour avoir un rendu égal suivant la taille de la valeur
        printf(" %d   ", c.valeur);
    else
        if(c.valeur/10 > 0)
            printf("  %d   ", c.valeur);
        else
            printf("   %d   ", c.valeur);
}

struct PaquetCartes* creerPaquet(int tailleMax)//initiallise le paquet en allouant un espace mémoire à lui et au paquet qu'il représente
{
    struct PaquetCartes* p = (struct PaquetCartes*) malloc(sizeof(struct PaquetCartes));
    p->paquet = (struct Carte*) malloc(tailleMax*sizeof(struct Carte));
    p->taille = 0;//et initialise la taille du paquet et le nombre de têtes qu'il contient à 0
    p->nbTete = 0;
}

void detruitPaquet(struct PaquetCartes* p)//libère les espaces mémoires occupés par le paquet 'p'
{
    free(p->paquet); 
    free(p);
}

void ajouteCarte(struct PaquetCartes* p, struct Carte c)//ajoute la carte 'c' dans le paquet 'p' et met à jour sa taille et son nombre de têtes
{
    p->paquet[p->taille] = c;
    p->taille += 1;
    p->nbTete += c.nbTete;
}

struct Carte retireCarte(struct PaquetCartes* p, int indice)//retire la carte d'indice 'indice' du paquet 'p'
{
    struct Carte c = p->paquet[indice];
    for(int i=indice; i<p->taille-1; i++)
    {
        p->paquet[i] = p->paquet[i+1];
    }
    p->taille -= 1;
    p->nbTete -= c.nbTete;
    return c;
}

void videPaquet(struct PaquetCartes* p)//retire la première carte du paquet 'p' jusqu'à ce qu'il soit vide
{
    int t = p->taille;
    for(int i = 0; i < t; i++)
    {
        retireCarte(p,0);
    }
}

void affichePaquet(struct PaquetCartes* p)//affiche le paquet 'p' en appelant la fonction 'afficheCarte()' sur chaque carte de 'p' 
{
    for(int i = 0; i < p->taille; i++){
        afficheCarte(p->paquet[i]);
    }
}

struct PaquetCartes* creerPioche()//créé le paquet de carte contenant les 104 cartes du jeu
{
    struct PaquetCartes* pioche = creerPaquet(104);
    for(int i = 0; i < 104; i++)
    {
        int val = i + 1;
        int nbTete;
        if(val == 55)
            nbTete = 7;//la seule carte à avoir 7 têtes est 55 car elle est divisible par 11 (5 têtes) et par 5 (2 têtes)
        else
        {
            if(val % 5 == 0)//si la valeur est divisible par 5
            {
                if(val % 10 == 0)//et par 10
                    nbTete = 3;//alors la valeur fini par 0 et vaut donc 3 têtes
                else
                    nbTete = 2;//sinon la valeur fini par 5 et vaut donc 5 têtes
            }
            else
            {
                if(val % 11 == 0)//si la valeur est divisible par 11, c'est à dire que la valeur est 2 fois le meme nombre (11,22,33,44,...)
                    nbTete = 5;//alors elle vaut 5 têtes
                else
                    nbTete = 1;//sinon c'est une carte sans specificité et vaut donc 1 tête
            }
        }
        struct Carte c;//on crée la carte, on l'initialise avec sa valeur et son nombre de têtes calculé précédement 
        c.valeur = val;
        c.nbTete = nbTete;
        ajouteCarte(pioche, c);//puis on l'ajoute dans le paquet

    }
}

void paquetToStr(struct PaquetCartes* p, char* str)//transforme le paquet 'p' en chaine de caractère
{
    char s[35];
    for(int i = 0; i < p->taille; i++)//transforme chaque carte de 'p' en chaine de caractère
    {
        snprintf(s, sizeof(s), "%d %d ",p->paquet[i].valeur, p->paquet[i].nbTete);//forme une chaine de caractère de la forme 'valeur nbTete ' puis l'ajoute à 'str'
        strcat(str,s);
    }
    snprintf(s, sizeof(s), "t%dt ", p->nbTete);//ajoute le nombre de têtes total de 'p' entouré du caractère 't' à 'str'
    strcat(str, s);
}

void strToPaquet(struct PaquetCartes* p, char* str)//transforme la chaine de caractère 'str' en une instance de PaquetCartes* représenté par 'p'
{
    char d1[] = " ";//délimiteurs de la chaine de caractère
    char d2[] = "t";
    char* recupCarte1;
    bool b = true;
    struct Carte c;
    char* str2 = (char*) malloc(strlen(str)*sizeof(char));//copie la chaine de caractère 'str' pour pouvoir utiliser deux fois 'strtok()'
    strcpy(str2,str);
    recupCarte1 = strtok(str, d1);//récupère tous les caractère se trouvant avant le délimiteur d1 dans 'str'
    videPaquet(p);
    while(recupCarte1 != NULL)//tant que la chaine de caractère n'est pas vide
    {
        if(b)//transforme les caractères récupérés en integer puis les associe une fois sur deux à la valeur de la carte
        {
            c.valeur = atoi(recupCarte1);
            b = false;
        }
        else//et l'autre fois sur deux au nombre de têtes de la carte
        {
            c.nbTete = atoi(recupCarte1);
            b = true;
            ajouteCarte(p, c);//puis ajoute la carte à 'p'
        }
        recupCarte1 = strtok(NULL, d1);//récupère les caractères se trouvant avant le délimiteur d1 dans la dernière chaine de caractère qui à subit 'strtok()'
    }
    char* recupCarte2 = strtok(str2, d2);//récupère les caractères se trouvant avant le délimiteur d2 dans 'str2' 
    recupCarte2 = strtok(NULL, d2);
    p->nbTete = atoi(recupCarte2);//et les transforment en integer pour les associer au nombre de têtes total du paquet

}

/*
int main()  //test des fonctions de la classe
{
    struct Carte c;
    c.valeur = 2;
    c.nbTete = 5;
    afficheCarte(c);

    struct Carte c2;
    c2.valeur = 22;
    c2.nbTete = 52;
    afficheCarte(c2);

    struct Carte c3;
    c3.valeur = 23;
    c3.nbTete = 53;
    afficheCarte(c3);

    struct PaquetCartes* p = creerPaquet(10);
    ajouteCarte(p, c);
    ajouteCarte(p, c2);
    ajouteCarte(p, c3);

    printf("\n");

    affichePaquet(p);
    retireCarte(p,1);
    printf("\n");
    affichePaquet(p);

    struct PaquetCartes* p1 = creerPioche();
    affichePaquet(p1);
    detruitPaquet(p);
    detruitPaquet(p1);
    return 0;
}*/