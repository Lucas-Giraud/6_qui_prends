#include "../header/Joueur.h"

#define SA struct sockaddr

void error(char *msg)//arrête le programme et envoie un message d'erreur au système
{
    perror(msg);
    exit(1);
}

void afficheIndice(int nbIndice)//affiche les indices des cartes de la main du joueur
{
    printf("\n");
    for(int i=0; i<nbIndice; i++)
        printf("  %d.   ", i);
    printf("\n");
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
        n = read(p_th.socket, buffer, 256);//récupère le plateau de jeu
        if (n < 0) 
            error("ERROR reading from socket");
        strToPlateau(plateau, buffer);
        
        while(pMain->taille > 0)//tant que le joueur possède des cartes
        {
            affichePlateau(plateau);//affiche le plateau, la main et les indices des cartes dans la console du joueur
            affichePaquet(pMain);
            afficheIndice(pMain->taille);
            printf("\n");
            printf("Quelle carte souhaitez vous jouer ?\n");

            fgets(buffer, sizeof(buffer), stdin);//attends le choix de carte du joueur
            int choix = atoi(buffer);
            while(choix<0 || choix>pMain->taille-1)//réitère la demande si le joueur fais un choix non conforme
            {
                printf("Votre choix doit être compris entre 0 et %d.\n",pMain->taille-1);
                bzero(buffer, sizeof(buffer));
                fgets(buffer, sizeof(buffer), stdin);
                choix = atoi(buffer);
            }
            struct Carte c = retireCarte(pMain, choix);//retire la carte choisie de la main du joueur
            bzero(buffer, sizeof(buffer));
            snprintf(buffer, sizeof(buffer), "%d %d ", c.valeur, c.nbTete);//la transforme en chaine de caractère et l'envoie au serveur
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
