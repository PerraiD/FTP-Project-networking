/*----------------------------------------------
  Serveur à lancer avant le client
  ------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h> 	/* pour les sockets */
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h> 		/* pour hostent, servent */
#include <string.h> 		/* pour bcopy, ... */
#include "libnet.c" 
#include <fcntl.h> // for open
#include <unistd.h> // for close


typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

/*------------------------------------------------------*/

/*------------------------------------------------------*/

/*------------------------------------------------------*/
int main(int argc, char **argv) {

    int 		socket_descriptor, 		/* descripteur de socket */
                nouv_socket_descriptor, 	/* [nouveau] descripteur de socket */
                longueur_adresse_courante, /* longueur d'adresse courante d'un client */
                action; // action courante demander par le client.

    sockaddr_in 	adresse_locale, 		/* structure d'adresse locale*/
                    adresse_client_courant; 	/* adresse client courant */
    hostent*		ptr_hote; 			/* les infos recuperees sur la machine hote */
    char 		machine[TAILLE_MAX_NOM+1]; 	/* nom de la machine locale */
    char        filePath[TAILLE_MAX_NOM];

    gethostname(machine,TAILLE_MAX_NOM);		/* recuperation du nom de la machine */
    /* recuperation de la structure d'adresse en utilisant le nom */
    if ((ptr_hote = gethostbyname("localhost")) == NULL) {
        perror("erreur : impossible de trouver le serveur a partir de son nom.");
        exit(1);
    }


    // create directory of file server
    mkdir(DIR_DL, 0777);

    /* initialisation de la structure adresse_locale avec les infos recuperees */

    /* copie de ptr_hote vers adresse_locale */
    bcopy((char*)ptr_hote->h_addr, (char*)&adresse_locale.sin_addr, ptr_hote->h_length);
    adresse_locale.sin_family		= ptr_hote->h_addrtype; 	/* ou AF_INET */
    adresse_locale.sin_addr.s_addr	= INADDR_ANY; 			/* ou AF_INET */

    /* 2 facons de definir le service que l'on va utiliser a distance */
    /* (commenter l'une ou l'autre des solutions) */

    /*-----------------------------------------------------------*/
    /* SOLUTION 1 : utiliser un service existant, par ex. "irc" */
    /*
       if ((ptr_service = getservbyname("irc","tcp")) == NULL) {
       perror("erreur : impossible de recuperer le numero de port du service desire.");
       exit(1);
       }
       adresse_locale.sin_port = htons(ptr_service->s_port);
       */
    /*-----------------------------------------------------------*/
    /* SOLUTION 2 : utiliser un nouveau numero de port */
    adresse_locale.sin_port = htons(4999);
    /*-----------------------------------------------------------*/

    printf("numero de port pour la connexion au serveur : %d \n",
           ntohs(adresse_locale.sin_port) /*ntohs(ptr_service->s_port)*/);

    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("erreur : impossible de creer la socket de connexion avec le client.");
        exit(1);
    }

    /* association du socket socket_descriptor à la structure d'adresse adresse_locale */
    if ((bind(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
        perror("erreur : impossible de lier la socket a l'adresse de connexion.");
        exit(1);
    }

    /* initialisation de la file d'ecoute */
    listen(socket_descriptor,5);

    /* attente des connexions et traitement des donnees recues */
    for(;;) {

        longueur_adresse_courante = sizeof(adresse_client_courant);

        /* adresse_client_courant sera renseigné par accept via les infos du connect */
        if ((nouv_socket_descriptor =accept(socket_descriptor, (sockaddr*)(&adresse_client_courant), &longueur_adresse_courante)) < 0) {
            perror("erreur : impossible d'accepter la connexion avec le client.");
            exit(1);
        }
        int pid = fork();

        if (pid == 0) {
          
            
            int actionR= recv(nouv_socket_descriptor,&action,sizeof(int),0);
            int tname;
            int tnameR;
            char * nomDeFichier;

            if(actionR>0){ // si une action est envoyer par le client on la traite
                    switch(action){
                        case UPLOAD :

                        reception_fichier(&nouv_socket_descriptor);

                        break;

                        case DOWLOAD : 
                            //reception du nom de fichier a envoyer au client
                            //todo vérification de la réception des éléments
                            
                            //réception de la taille du nom
                            tnameR=recv(nouv_socket_descriptor,&tname,sizeof(int),0);

                            nomDeFichier= malloc(sizeof(char)*(tname+1));
                            //réception du nom
                            int nomRecu= recv(nouv_socket_descriptor,nomDeFichier,sizeof(char)*(tname),0);
                            nomDeFichier[tname] = '\0';

                            if(nomRecu>0){
                                //envoie du fichier au client.
                                transfert_fichier(nouv_socket_descriptor,nomDeFichier);        
                            }
                        
                        break;
                    }

                        
                }   
            

            exit(0);

        } else  if (pid== -1){
            perror("impossible de créer un fils");
            exit(1);
        }else{
            close(nouv_socket_descriptor);

        }



    }

}

