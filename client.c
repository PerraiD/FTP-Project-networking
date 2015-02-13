/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
client <adresse-serveur> <message-a-transmettre>
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

#include <fcntl.h> // for open
#include <unistd.h> // for close

typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;


// envoyer la taille du fichier sur le serveur puis recevoir le fichier et comparer la taille reçu !

long taille_fichier(FILE* fichier){

	long tailleFichier=-1;

	if(fichier){
		tailleFichier=ftell(fichier);
		fclose (fichier);
	}

return tailleFichier;

}


// voir Fread() pour lire octet par octet  un fichier et le découpé en buffer
int transfert_fichier(int socket_descriptor,char * fileName){

	FILE* fichier;
	fichier = fopen(fileName,"ab"); //"données.txt"

	long fileSize = taille_fichier(fichier);

	//réouverture car l'ouverture  précédente avec ab place la tête de lecture a la fin du fichier
	fichier = fopen(fileName,"rb");
    int tailleName=strlen(fileName);

    int sent=send(socket_descriptor,&tailleName,sizeof(int),0);
			sent=send(socket_descriptor,&fileSize,sizeof(long),0);


				sent= send(socket_descriptor,fileName,strlen(fileName),0);
				printf("envoyer : %d \n",sent);



    char sendBuffer[1500];
    int bytesRead = fread(sendBuffer,1, sizeof(fileSize), fichier);

    while(!feof(fichier))
    {
      //TODO: check for errors here
      send(socket_descriptor, sendBuffer, bytesRead, 0);
      bytesRead = fread(sendBuffer, 1,sizeof(fileSize), fichier);

    }


    close(socket_descriptor);
    fclose(fichier);

    return 1;
}


int main(int argc, char **argv) {

    int 	socket_descriptor, 	/* descripteur de socket */
		longueur; 		/* longueur d'un buffer utilisé */
    sockaddr_in adresse_locale; 	/* adresse de socket local */
    hostent *	ptr_host; 		/* info sur une machine hote */
    char 	buffer[256];
    char *	host; 			/* nom de la machine distante */
    char * filePath; 		/* chemain du fichier à envoyée */

    if (argc != 3) {
	perror("usage : client <adresse-serveur> <chemin du fichier à transmettre>");
	exit(1);
    }

    host = argv[1];
    filePath= argv[2];

   // printf("adresse du serveur  : %s \n", host);

    if ((ptr_host = gethostbyname(host)) == NULL) {
	perror("erreur : impossible de trouver le serveur a partir de son adresse.");
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

    printf("connexion etablie avec le serveur. \n");

    printf("envoi d'un message au serveur. \n");

    transfert_fichier(socket_descriptor,filePath);

    /* envoi du message vers le serveur */

    // todo : envoyer le fichier a la place du nom de fichier


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

