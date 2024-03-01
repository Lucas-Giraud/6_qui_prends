#include "MaitreJeu.h"

void error(char *msg)//arrête le programme et envoie un message d'erreur au système
{
    perror(msg);
    exit(1);
}

struct MaitreJeu* creerMJ()//créé une instance de 'MaitreJeu*' et initialise tous ses attributs 
{
     struct MaitreJeu* MJ = (struct MaitreJeu*) malloc(sizeof(struct MaitreJeu));
     MJ->plateau = creerPlateau();
     MJ->pioche = creerPioche();
     MJ->nbJoueur = 0;
     MJ->cartesJouees = creerPaquet(10);//max 10 joueurs
     MJ->nbTeteRecup = (int*) malloc(10*sizeof(int));
     MJ->score = (int*) calloc(10, sizeof(int));
     MJ->joueur1 = -1;
     MJ->manche = 0;
     MJ->tour = 1;
     MJ->scoreAtteint = false;
     initPlateau(MJ);
     return MJ;
}

void detruitMj(struct MaitreJeu* MJ)//libère tous les espaces mémoire alloués à 'MJ'
{
     detruitPlateau(MJ->plateau);
     detruitPaquet(MJ->pioche);
     detruitPaquet(MJ->cartesJouees);
     free(MJ->nbTeteRecup);
     free(MJ->score);
     free(MJ);
}

struct PaquetCartes* distribution(struct MaitreJeu* MJ)//récupère 10 cartes qui formeront la main du joueur
{
     struct PaquetCartes* pc = creerPaquet(10);
     for(int i=0; i<10; i++)//pioche 10 cartes au hasard dans la pioche
     {
          int r = rand()%MJ->pioche->taille;
          struct Carte c = retireCarte(MJ->pioche,r);
          ajouteCarte(pc,c);
     }
     bool trie = false;
     while(!trie)//puis trie cette main dans l'ordre croissant
     {
          trie = true;
          for(int i=0; i<9; i++)
          {
               if(pc->paquet[i].valeur > pc->paquet[i+1].valeur)
               {    
                    trie = false;
                    struct Carte temp = pc->paquet[i];
                    pc->paquet[i] = pc->paquet[i+1];
                    pc->paquet[i+1] = temp;
               }
          }
     }
     return pc;
}

void initPlateau(struct MaitreJeu* MJ)//initialise le plateau de la partie
{
     struct Carte cartes[4];
     for(int i=0; i<4; i++)//pioche 4 cartes au hasard
     {
          int r = rand()%MJ->pioche->taille;
          cartes[i] = retireCarte(MJ->pioche,r);
     }
     bool trie = false;
     while(!trie)//puis les trie dans l'ordre croissant
     {
          trie = true;
          for(int i=0; i<3; i++)
          {
               if(cartes[i].valeur > cartes[i+1].valeur)
               {    
                    trie = false;
                    struct Carte temp = cartes[i];
                    cartes[i] = cartes[i+1];
                    cartes[i+1] = temp;
               }
          }
     }
     for(int i=0; i<4; i++)//et en place une sur chaque ligne du plateau
     {
          ajouteCarteLigne(MJ->plateau,cartes[i],i);
     }
}

void scoreToStr(struct MaitreJeu* MJ, char* str)//transforme le tableau de score en une chaine de caractère
{
     char s[40];
     for(int i = 0; i < MJ->nbJoueur; i++)
     {
          snprintf(s, sizeof(s), "Joueur %d : %d Tetes de Boeuf;\n", i, MJ->score[i]);
          strcat(str,s);
     }
}

void* thJoueur(void* args)//thread qui  fait le lien entre le serveur et le joueur auquel il est assigné
{
     int n;  //initialise les variables de la partie
     struct parametre_th p_th = *(struct parametre_th*) args;
     char buffer[256]; 
     while(p_th.MJ->nbJoueur < p_th.joueurTot)//attends que tous les joueurs soient présent pour lancer la partie
     {
          sleep(1);
     } 
     while (p_th.MJ->manche < p_th.mancheTot && !p_th.MJ->scoreAtteint)//tant que le nombre de manche défini et que personne n'a atteint 66 têtes
     {

          if(p_th.MJ->manche != p_th.mancheTot)//nous trouvons cete condition ridicule mais quand nous l'enlevons, la fin de partie ne se fait pas
          {
               bzero(buffer,sizeof(buffer));
               paquetToStr(distribution(p_th.MJ), buffer); //récupère et transforme en chaine de caractère la main du joueur
               n = write(p_th.socket[p_th.idJoueur], buffer, sizeof(buffer)); //puis l'envoie au joueur
               if(n<0)
                    error("ERROR writing in socket");
               bzero(buffer,sizeof(buffer));
               plateauToStr(p_th.MJ->plateau, buffer);//transforme le plateau en chaine de caractère puis l'envoie au joueur
               n = write(p_th.socket[p_th.idJoueur], buffer, sizeof(buffer)); 
               if(n<0)
                    error("ERROR writing in socket");
          }
          while(p_th.MJ->tour <= 10)//tant que 10 tours ne sont pas passés (les joueurs ont 10 cartes)
          {
               bzero(buffer, sizeof(buffer));
               n = read(p_th.socket[p_th.idJoueur], buffer, 256);//attends le message du joueur contenant la carte qu'il a joué
               if(n < 0)
                    error("ERROR reading from socket");
               attendCarte(p_th, buffer);//attends que tous les joueurs aient joué
               if(p_th.idJoueur == p_th.MJ->joueur1)//le premier joueur à avoir choisi
               { 
                    joueCartes(p_th);//place les cartes de tout le monde 
                    sleep(1);
                    videPaquet(p_th.MJ->cartesJouees);//sort les autre joueurs de leur attente
                    sleep(1);
                    p_th.MJ->tour += 1;//et augment le nombre de tour de 1
               }//le contenu de cette condition ne doit être effectuée qu'une seul fois donc on donne accès seulement à 1 joueur
               else
               {
                    while(p_th.MJ->cartesJouees->taille != 0)//tant que les cartes ne sont pas jouées, attend
                    {
                         sleep(2);
                    }
               }
               p_th.MJ->score[p_th.idJoueur] += p_th.MJ->nbTeteRecup[p_th.idJoueur];//incrémente le score du joueur avec le nombre de têtes qu'il a récupéré
               bzero(buffer, sizeof(buffer));
               snprintf(buffer, sizeof(buffer), "Vous avez pris %d tetes de boeuf\n", p_th.MJ->nbTeteRecup[p_th.idJoueur]);
               n = write(p_th.socket[p_th.idJoueur],buffer,sizeof(buffer)); //envoie le message contenant le nombre de têtes récupérées au joueur
               if(n<0)
                    error("ERROR writing in socket");
               bzero(buffer, sizeof(buffer));
               sleep(1);
               plateauToStr(p_th.MJ->plateau,buffer);
               n = write(p_th.socket[p_th.idJoueur],buffer,sizeof(buffer)); //envoie le plateau mis à jour au joueur
               if(n<0)
                    error("ERROR writing in socket");
               p_th.MJ->joueur1 = -1;
               
          }
          bzero(buffer, sizeof(buffer));
          int manchePrec = p_th.MJ->manche;
          sleep(p_th.idJoueur+1);
          if( manchePrec == p_th.MJ->manche)//si le nombre de manche n'a pas encore été incrémenté 
          {  

               p_th.MJ->manche += 1;//le fait
               if(p_th.MJ->manche < p_th.mancheTot)//si toutes les manches n'ont pas été jouées
               {
                    printf("Manche %d/%d", p_th.MJ->manche+1, p_th.mancheTot);//réinitialise la pioche, le plateau et le nombre de tour
                    detruitPaquet(p_th.MJ->pioche);
                    p_th.MJ->pioche = creerPioche();
                    detruitPlateau(p_th.MJ->plateau);
                    p_th.MJ->plateau = creerPlateau();
                    initPlateau(p_th.MJ);
                    p_th.MJ->tour = 1;          
               }
          }
          else
          {
               sleep(1);//sinon attend
          }
          sleep(p_th.idJoueur+1);
          fin_66(p_th.MJ);//vérifie si quelqu'un à atteint ou dépassé les 66 têtes
          if(p_th.MJ->manche == p_th.mancheTot || p_th.MJ->scoreAtteint)//si la partie est terminée (toutes les manches ont été jouées ou le score est trop élévé)
          {
               snprintf(buffer, sizeof(buffer), "termine");//le message devient 'termine'
          }
          n = write(p_th.socket[p_th.idJoueur],buffer,sizeof(buffer)); //envoie soit un message vide soit termine
          if(n<0)
               error("ERROR writing in socket");  
     }
     bzero(buffer, sizeof(buffer));
     snprintf(buffer, sizeof(buffer), "Vous etes le joueur %d.\n", p_th.idJoueur);//donne au message le numéro du joueur  
     scoreToStr(p_th.MJ, buffer);//lui ajoute le score de tout le monde
     sleep(1);
     n = write(p_th.socket[p_th.idJoueur], buffer, sizeof(buffer));//et l'envoie au joueur
     if(n < 0)
          error("ERROR writing in socket");
}

void fin_66(struct MaitreJeu* MJ)//vérifie si un joueur à atteint ou dépassé le score de 66 têtes
{
     for(int i = 0; i < MJ->nbJoueur; i++)
     {
          if(MJ->score[i] >= 66)
               MJ->scoreAtteint = true;
     }
}

void attendCarte(struct parametre_th p_th, char* str)//ajoute la carte jouée à la liste des cartes jouées et attends que la liste soit pleine
{
     if(p_th.MJ->joueur1 == -1)
          p_th.MJ->joueur1 = p_th.idJoueur;
     struct Carte c;
     char* p = strtok(str, " ");//récupère les caractères se trouvant avant le délimiteur " " dans 'str'
     c.valeur = atoi(p);//et donne leur valeur à la carte 'c'
     p = strtok(NULL, " ");//idem mais pour le nombre de têtes
     c.nbTete = atoi(p);
     p_th.MJ->cartesJouees->paquet[p_th.idJoueur]= c;
     p_th.MJ->cartesJouees->taille += 1;
     while(p_th.MJ->cartesJouees->taille < p_th.MJ->nbJoueur)//tant que tous les joueurs n'ont pas choisi, attend
     {
          sleep(1);
     }
}


int joueCartes(struct parametre_th p_th)//ajoute les cartes sur le plateau en respectant les règles
{
     for(int j = 0; j < p_th.MJ->nbJoueur; j++)//pour toutes les cartes
     {
          struct Carte cMin = p_th.MJ->cartesJouees->paquet[0];//en commençant par la première du paquet
          int place = 0;
          for(int i = 1; i < p_th.MJ->nbJoueur; i++)//récupère la carte ayant la plus petite valeur ainsi que sa place (il s'agit aussi du numéro du joueur qui l'a choisie)
          {
               struct Carte c = p_th.MJ->cartesJouees->paquet[i];
               if(cMin.valeur > c.valeur)
               {
                    cMin = c;
                    place = i;
               }
          }

          int valMin = carteMin(p_th.MJ->plateau);//récupère la valeur de la dernière carte la plus petite du plateau
          if(cMin.valeur < valMin)//si la valeur de 'cMin' est plus petie que celle du plateau
          {
               int minTete = p_th.MJ->plateau->lignes[0]->nbTete;
               int choix = 0;
               for(int i = 1; i < 4; i++)//cherche la ligne contenant le moins de têtes de boeuf
               {
                    if(minTete > p_th.MJ->plateau->lignes[i]->nbTete)
                    {
                         minTete = p_th.MJ->plateau->lignes[i]->nbTete;
                         choix = i;
                    }
               }
               p_th.MJ->nbTeteRecup[place] = p_th.MJ->plateau->lignes[choix]->nbTete;//remplace la ligne par la carte du joueur et récupère les têtes présentes
               videPaquet(p_th.MJ->plateau->lignes[choix]);
               ajouteCarteLigne(p_th.MJ->plateau, cMin, choix);
               p_th.MJ->cartesJouees->paquet[place].valeur = 105;//la valeur de la carte jouée devient 105 uniquement dans la liste afin qu'elle ne soit plus choisie
          }//cela evite de jouer n fois la même carte
          else//sinon la carte peut être placée
          {
               sleep(0.5);
               int dif = 104;//la difference de valeur entre deux carte ne peut pas etre superieure ou egale a 104 car elles vont de 1 a 104  
               int ligne;
               for(int i=0; i<4; i++)//cherche sur quelle ligne la carte sera placée
               {//il s'agit de la ligne avec la carte la plus proche et de valeur inférieure de celle qu'on veut placer
                    struct PaquetCartes* pc = p_th.MJ->plateau->lignes[i];
                    if(pc->paquet[pc->taille-1].valeur < cMin.valeur)
                    {
                         int difTemp = cMin.valeur - pc->paquet[pc->taille-1].valeur;
                         if(dif > difTemp)//la carte est plus proche 
                         {    
                              dif = difTemp;
                              ligne = i;
                         }
                    }
               }
               p_th.MJ->nbTeteRecup[place] = ajouteCarteLigne(p_th.MJ->plateau, cMin, ligne);//ajoute la carte à cette ligne et récupère le nombre de têtes de la ligne si elle était déjà pleine
               p_th.MJ->cartesJouees->paquet[place].valeur = 105;//idem que pour cMin plus petite
          }
     }
}

int main(int argc, char *argv[])
{
     srand(time(NULL));//initialisations des variables nécessaires à la crétion du socket et du bon déroulement de la partie
     int sockfd, newsockfd=0, portno, clilen, rc, mancheTot, joueurTot, robotTot = 0, n;
     char buffer[256], temp[10];
     struct sockaddr_in serv_addr, cli_addr;
     struct MaitreJeu* MJ = creerMJ();
     struct parametre_th p_th;

     if (argc < 2) {//verifie qu'un port a ete renseigne
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);//cree un socket et verifie qu'il a bien ete ouvert
     if (sockfd < 0) 
        error("ERROR opening socket");
     setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) //lie le socket au serveur
          error("ERROR on binding");
     
     printf("Combien de manches souhaitez-vous faire ? (1..10)\n");
     fgets(temp, sizeof(temp), stdin);//attends un nombre de manche compris entre 1 et 10
     mancheTot = atoi(temp);
     while(mancheTot<1 || mancheTot>10)
     {//réitère la question si l'utilisateur entre n'importe quoi
          printf("Votre choix doit être compris entre 1 et 10.\n");
          bzero(temp, sizeof(temp));
          fgets(temp, sizeof(temp), stdin);
          mancheTot = atoi(temp);
     }
     
     printf("Combien de joueurs sont attendus ? (1..10)\n");
     fgets(temp, sizeof(temp), stdin);//attends un nombre de joueurs compris entre 1 et 10, robots inclus
     joueurTot = atoi(temp);
     while(joueurTot<1 || joueurTot>10)
     {//réitère la question si l'utilisateur entre n'importe quoi
          printf("Votre choix doit être compris entre 1 et 10.\n");
          bzero(temp, sizeof(temp));
          fgets(temp, sizeof(temp), stdin);
          joueurTot = atoi(temp);
     }
     pthread_t threadJoueur[joueurTot];
     p_th.joueurTot = joueurTot;//initialise les paramètres qui seront passés au thread
     p_th.MJ = MJ;
     p_th.mancheTot = mancheTot;
     p_th.socket = (int*) malloc(10*sizeof(int));
     printf("Joueurs connectés %d/%d\n", MJ->nbJoueur, joueurTot);
     while(MJ->nbJoueur < joueurTot)//tant que tous les joueurs ne sont pas connectés, attend une connection
     {
          int n = listen(sockfd,joueurTot);//attends une connection et 5 seront dans la file avant que d'autres requetes ne soit refusée
          clilen = sizeof(cli_addr);//recupere la taille de l'adresse du client
          newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);//essaie de connecter le client au serveur et affiche un message suivant la reussite de l'operation
          if (newsockfd < 0) 
               error("ERROR on accept");
          p_th.socket[MJ->nbJoueur] = newsockfd;
          p_th.idJoueur = MJ->nbJoueur;
          rc = pthread_create(&threadJoueur[MJ->nbJoueur], NULL, thJoueur, &p_th);//créé un thread en lui assignant la fonction 'thJoueur()' et en lui donnant le paramètre 'p_th'
          if (rc)
          {
               error("ERROR from pthread_create");
          }
          MJ->nbJoueur += 1;
          printf("Joueurs connectés %d/%d\n", MJ->nbJoueur, joueurTot);
     }
     printf("Lancement de la partie....\n");
     printf("Manche 1/%d", mancheTot);
     
     for(int i = 0; i < joueurTot; i++)//attends que tous les joueurs aient terminés leur éxecution avant de pursuivre celle du main()
     {
          n = pthread_join(threadJoueur[i], NULL);
          if(n < 0)
          {
               error("ERROR when joining");
          }
     }
     bzero(buffer, sizeof(buffer));
     scoreToStr(MJ, buffer);//affiche les scores de la partie
     printf("La partie est terminee !\n%s", buffer);

     for(int i = 0; i < joueurTot; i++)//ferme tous les sockets
     {
          close(p_th.socket[i]);
     }
     close(sockfd);
     free(p_th.socket);//et libère tous les espaces mémoire alloués
     detruitMj(MJ);
     return 0;
}