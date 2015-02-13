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

#include <fcntl.h> // for open
#include <unistd.h> // for close

#define TAILLE_MAX_NOM 256

#define HOME_SERVER "/tmp/filesFTP"

#define BUFFER_MAX_SIZE 1500

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

/*------------------------------------------------------*/
void* prise_en_charge_client(void* sock)
{

    //cast du socket
    int *tmp = (int*) sock;
    int socketDescriptor = *tmp;
    long  tailleFichier = 0;
    int tailleNomFichier=0;

    FILE * fichier; // création du future fichier réassemblé.

    int tnr= recv(socketDescriptor,&tailleNomFichier,sizeof(int),0);
    if (tnr <= 0) {
        perror("erreur de réception");
    }

    printf("%d\n", tailleNomFichier);

    char * nomDeFichier= malloc(sizeof(char)*(tailleNomFichier+1));



    //reception de la taille du fichier
    int tailleRecu= recv(socketDescriptor,&tailleFichier,sizeof(long),0);


    //verification de la bonne réception d'un fichier -1 en cas d'erreur
    if(tailleRecu > 0){

        //reception du nom de fichier

        int nomRecu= recv(socketDescriptor,nomDeFichier,sizeof(char)*(tailleNomFichier),0);
        nomDeFichier[tailleNomFichier] = '\0';

        if(nomRecu<=0){
            perror("erreur de la réception du nom de fichier ");
        }
    }else{
        perror("erreur de la réception de la taille du fichier");
    }

    printf("taille du fichier a recevoir: %ld \n", tailleFichier);
    printf("nom du fichier a recevoir : %s \n",nomDeFichier);

    char * path = malloc(sizeof(char)*(strlen(HOME_SERVER) + strlen(nomDeFichier) + 2));
    strcpy(path, HOME_SERVER);
    strcat(path, "/");
    strcat(path, nomDeFichier);

    printf("%s \n", path);

    fichier = fopen(path, "wb");
    if (fichier != NULL) {

        char recvBuff[BUFFER_MAX_SIZE];

        //        int bytesReceived = recv(socketDescriptor, recvBuff,sizeof(char)*BUFFER_MAX_SIZE, 0);
        int bytesReceived = recv(socketDescriptor, recvBuff,sizeof(recvBuff), 0);

        printf("%d\n",bytesReceived);
        while(bytesReceived != 0)
        {
            printf("%d\n",bytesReceived);
            // you should add error checking here
            fwrite(recvBuff, bytesReceived, 1, fichier);

            //          bytesReceived = recv(socketDescriptor, recvBuff, sizeof(char)*BUFFER_MAX_SIZE, 0);
            bytesReceived = recv(socketDescriptor, recvBuff, sizeof(recvBuff), 0);
        }

        fclose(fichier);

    }
    else
    {
        perror("erreur impossible d'ouvrir le fichier");
    }
    close(socketDescriptor);


    printf("message envoye. \n");

    return NULL;


}
/*------------------------------------------------------*/

/*------------------------------------------------------*/
int main(int argc, char **argv) {

    int 		socket_descriptor, 		/* descripteur de socket */
                nouv_socket_descriptor, 	/* [nouveau] descripteur de socket */
                longueur_adresse_courante; 	/* longueur d'adresse courante d'un client */
    sockaddr_in 	adresse_locale, 		/* structure d'adresse locale*/
                    adresse_client_courant; 	/* adresse client courant */
    hostent*		ptr_hote; 			/* les infos recuperees sur la machine hote */
    char 		machine[TAILLE_MAX_NOM+1]; 	/* nom de la machine locale */

    gethostname(machine,TAILLE_MAX_NOM);		/* recuperation du nom de la machine */
    /* recuperation de la structure d'adresse en utilisant le nom */
    if ((ptr_hote = gethostbyname("localhost")) == NULL) {
        perror("erreur : impossible de trouver le serveur a partir de son nom.");
        exit(1);
    }


    // create directory of file server
    mkdir(HOME_SERVER, 0777);

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
        if ((nouv_socket_descriptor =
             accept(socket_descriptor,
                    (sockaddr*)(&adresse_client_courant),
                    &longueur_adresse_courante))
            < 0) {
            perror("erreur : impossible d'accepter la connexion avec le client.");
            exit(1);
        }
        int pid = fork();

        if (pid == 0) {
            /* traitement du message */
            printf("reception d'un message.\n");

            prise_en_charge_client(&nouv_socket_descriptor);

            exit(0);

        } else  if (pid== -1){
            perror("impossible de créer un fils");
            exit(1);
        }else{
            close(nouv_socket_descriptor);

        }



    }

}

