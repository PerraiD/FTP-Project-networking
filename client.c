/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
client <adresse-serveur>
------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <string.h>
#include "libnet.c"

#include <fcntl.h> // for open
#include <unistd.h> // for close

#define DIR_DL "./"


typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;


// envoyer la taille du fichier sur le serveur puis recevoir le fichier et comparer la taille reçu !




// voir Fread() pour lire octet par octet  un fichier et le découpé en buffer





int main(int argc, char **argv) {

    int 	socket_descriptor, 	/* descripteur de socket */
		    longueur; 		/* longueur d'un buffer utilisé */
    sockaddr_in adresse_locale; 	/* adresse de socket local */
    hostent *	ptr_host; 		/* info sur une machine hote */
    char 	buffer[256];
    char *	host; 			/* nom de la machine distante */
    char  filePath[100]; 		/* chemain du fichier à envoyée */

    if (argc != 2) {
	perror("usage : client <adresse-serveur> ");
	exit(1);
    }

    host = argv[1];
    

   // printf("adresse du serveur  : %s \n", host);

    if ((ptr_host = gethostbyname(host)) == NULL) {
	perror("  erreur : impossible de trouver le serveur a partir de son adresse.");
	exit(1);
    }

    /* copie caractere par caractere des infos de ptr_host vers adresse_locale */
    bcopy((char*)ptr_host->h_addr, (char*)&adresse_locale.sin_addr, ptr_host->h_length);
    adresse_locale.sin_family = AF_INET; /* ou ptr_host->h_addrtype; */

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

    /*-----------------------------------------------------------*/
    /* SOLUTION 2 : utiliser un nouveau numero de port */
    adresse_locale.sin_port = htons(4999);
    /*-----------------------------------------------------------*/

    printf("numero de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port));

    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	perror("erreur : impossible de creer la socket de connexion avec le serveur.");
	exit(1);
    }

    /* tentative de connexion au serveur dont les infos sont dans adresse_locale */
    if ((connect(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
	perror("erreur : impossible de se connecter au serveur.");
	exit(1);
    }

    printf(" <<< connexion etablie avec le serveur FTP.>>>> \n \n");

    
    int choix = 0;
    while(choix != EXIT){

        printf("Choissisez votre action en indiquant le numéro : \n");
        printf("1 - télécharger un fichier sur le serveur (PUT) \n");
        printf("2 - télécharger un ficher  du serveur (GET) \n");
        printf("3 - executer une commande sur le serveur \n");
        printf("4 - quittez\n");
        scanf("%d",&choix);

        switch(choix){
            case UPLOAD:
                
                printf("indiquez le chemin absolue du fichier que vous souhaitez envoyer : \n");
                //affichage du fichier courant sur le serveur pwd;
                // possibilité de faire une commande 
                scanf("%s",filePath);

                //on avertie le serveur de ce que l'on veux faire
                if(execute_action(UPLOAD,socket_descriptor)>0){
                    //transfert du fichier.
                    transfert_fichier(socket_descriptor,filePath);
                }

            break;
            case DOWLOAD:

                printf("indiquez le fichier que vous souhaitez télécharger :\n");
                //affichage du fichier courant
                //lister les fichiers disponible et laisser le choix de pouvoir voir effectuer des commandes.
                scanf("%s",filePath);

                if(execute_action(DOWLOAD,socket_descriptor)>0){
                     //send du nom de fichier;
                    int sizeN= strlen(filePath);

                    int sentTailleN = send(socket_descriptor,&sizeN,sizeof(int),0);
                    int sentName=send(socket_descriptor,filePath,strlen(filePath),0);
                    if(sentName>0){
                        reception_fichier(&socket_descriptor);
                    }
                }else{
                    perror("l'action que vous avez demandé n'a pas pu aboutir");
                }


            break;
            case COMMAND:

            break;
            case EXIT: 
                exit(0);
            break;


        }
        //exit(0);
    } 


    /* mise en attente du prgramme pour simuler un delai de transmission */
    //sleep(3);

    printf("message envoye au serveur. \n");

    /* lecture de la reponse en provenance du serveur */
    while((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
    	printf("reponse du serveur : \n");
    	write(1,buffer,longueur);
    }

    printf("\nfin de la reception.\n");

    close(socket_descriptor);

    printf("connexion avec le serveur fermee, fin du programme.\n");

    exit(0);

}

