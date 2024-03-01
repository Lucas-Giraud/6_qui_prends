#include "../header/Robot.h" 

#define SA struct sockaddr

void error(char *msg)//arrête le programme et envoie un message d'erreur au système
{
    perror(msg);
    exit(1);
}

int choixCarte(struct PaquetCartes* main, struct Plateau* p)//fais le choix automatique d'une carte dans la 'main' du robot
{
    int choix = 0;
    int ligne = 0;
    int min = carteMin(p);
    bool meilleur = false;
    while(!meilleur)//tant que la meilleure carte n'a pas été trouvée
    {
        struct Carte cMin = main->paquet[choix];
        if(cMin.valeur < min)//si la valeur de la carte est plus petite que celles du plateau
        {
            choix += 1;//change de choix
            if(choix == main->taille)//sauf si c'est la dernière carte de la main au quel cas on joue la toute première
            {
                meilleur = true;
                choix = 0;
            }
        }
        else//sinon, la carte peut être posée
        {
            int dif = 104;//la différence de valeur entre deux carte ne peut pas être supérieure ou égale à 104 car elles vont de 1 à 104  
            for(int i=0; i<4; i++)
            {
                struct PaquetCartes* pc = p->lignes[i];
                if(pc->paquet[pc->taille-1].valeur < cMin.valeur)//cherche sur quelle ligne la carte devrait être posée
                {
                     int difTemp = cMin.valeur - pc->paquet[pc->taille-1].valeur;
                     if(dif > difTemp)//la carte est plus proche 
                     {    
                          dif = difTemp;
                          ligne = i;
                     }
                }
            }
            if(p->lignes[ligne]->taille == 5)//si il y a déjà 5 cartes de placées
            {
                choix +=1;//alors on change de choix
                if(choix == main->taille)//s'il s'agissait de la dernière carte de la main alors il vaut mieux prendre la plus petite carte de la main
                {
                    choix = 0;
                    meilleur = true;
                }
            }
            else//si elle peut être placée sans problèmes alors ce choix est le plus judicieux
            {
                meilleur = true;
            }
        }
        
    }
    
    return choix;
}

void* Joue(void* args)//thread qui fait le lien entre le joueur et le serveur
{
    bool termine = false;//initialisation de toutes les variables de la partie
    struct parametre_th p_th = *(struct parametre_th*) args;
    struct PaquetCartes* pMain = creerPaquet(10);
    struct Plateau* plateau = creerPlateau();
    char buffer[256];
    int n;
    while(!termine)//tant que la partie n'est pas terminée
    {
        n = read(p_th.socket,buffer,256);//récupère le paquet de carte qui constitue la main du joueur
        if (n < 0) 
            error("ERROR reading from socket");
        printf("Debut de la manche.\n");
        strToPaquet(pMain, buffer);

        bzero(buffer,256);
        n = read(p_th.socket, buffer, 256);//recupere le plateau de jeu
        if (n < 0) 
            error("ERROR reading from socket");
        strToPlateau(plateau, buffer);
        
        while(pMain->taille > 0)//tant que le robot possède des cartes
        {
            affichePlateau(plateau);//affiche le plateau, la main dans la console du robot
            affichePaquet(pMain);
            printf("\n");
            int choix = choixCarte(pMain, plateau);//choisi une carte, la retire de la main du robot et l'affiche
            struct Carte c = retireCarte(pMain, choix);
            printf("\n");
            afficheCarte(c);
            printf("\n");
            bzero(buffer, sizeof(buffer));
            snprintf(buffer, sizeof(buffer), "%d %d ", c.valeur, c.nbTete);//transforme la carte en chaine de caractère et l'envoie au serveur
            n = write(p_th.socket, buffer, sizeof(buffer));
            if(n<0)
                error("ERROR writing in socket");
        
            bzero(buffer, sizeof(buffer));
            n = read(p_th.socket, buffer, sizeof(buffer));//attends un message du serveur puis l'affiche
            if(n<0)
                error("ERROR reading from socket");
            printf("%s\n",buffer);
            bzero(buffer, sizeof(buffer));
            n = read(p_th.socket, buffer, sizeof(buffer));//attends le plateau mis à jour avec les cartes de chaque joueur
            if(n<0)
                error("ERROR reading from socket");  

            strToPlateau(plateau, buffer);
            if(pMain->taille == 0)//si le joueur n'a plus de cartes, affiche le plateau une dernière fois
                affichePlateau(plateau);
        }
        bzero(buffer, sizeof(buffer));
        n = read(p_th.socket, buffer, sizeof(buffer));//attends un message du serveur
        if(n<0)
            error("ERROR reading from socket");
        if((strcmp(buffer, "termine") == 0))//s'il s'agit du message 'termine' alors passe le booléen termine à true pour finir la partie
        {
            termine = true;
        }
        printf("Manche terminee\n");
        sleep(1);
    }
    detruitPaquet(pMain);//détruit la main et le plateau initialisés en début de partie
    detruitPlateau(plateau);
}

int main(int argc, char *argv[])
{
    printf("En attente de connection des autres joueurs...\n");
    int sockfd, portno, n, r2;//initialisation des variables nécessaires à la création du socket
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3) {
       printf("Trop peu d'arguments\n");
       exit(0);
    }
    portno = atoi(argv[2]);//procédure de création du socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(SA*)&serv_addr,sizeof(serv_addr)) < 0) //tentative de connection au serveur
        error("ERROR connecting");

    pthread_t joue[1];
    struct parametre_th p_th;//création et initialisation des paramètres qui seront passés au thread
    p_th.socket = sockfd;
    int rc = pthread_create(&joue[0], NULL, Joue, &p_th);//créé un thread en lui assignant la fonction 'Joue()' et en lui donnant le paramètre 'p_th'
    if (rc)
    {
        error("ERROR from pthread_create");
    }
    pthread_join(joue[0], NULL);//attend la fin de l'éxecution du thread avant de poursuivre l'éxectution du 'main()'
    bzero(buffer, sizeof(buffer));
    n = read(p_th.socket, buffer, sizeof(buffer));//attends le message du serveur contenant les scores et l'affiche
    if(n < 0)
        error("ERROR reading from socket");
    printf("%s\nPartie terminee.\n",buffer);
    close(sockfd);
    return 0;
}
