/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
client <adresse-serveur>
------------------------------------------------------------*/
#define DIR_DL "./"

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




typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;


// envoyer la taille du fichier sur le serveur puis recevoir le fichier et comparer la taille reçu !

// voir Fread() pour lire octet par octet  un fichier et le découpé en buffer





int main(int argc, char **argv) {

    int 	       socket_descriptor, 	/* descripteur de socket */
		            longueur;
                     		/* longueur d'un buffer utilisé */
    sockaddr_in adresse_locale; 	/* adresse de socket local */

    hostent *	  ptr_host; 		/* info sur une machine hote */
    servent *   ptr_service;

    char 	      buffer[256];
    char *	    host; 			/* nom de la machine distante */
    char        filePath[100]; 		/* chemain du fichier à envoyée */

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
    if ((ptr_service = getservbyname("ftp","tcp")) == NULL) {
    	perror("erreur : impossible de recuperer le numero de port du service desire.");
    	exit(1);
    }
    adresse_locale.sin_port = htons(ptr_service->s_port);
    */
    /*-----------------------------------------------------------*/

    /*-----------------------------------------------------------*/
    /* SOLUTION 2 : utiliser un nouveau numero de port */
     adresse_locale.sin_port = htons(2000);
    /*-----------------------------------------------------------*/

    //printf("numero de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port));

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


      //Réception de la commande pwd;
      char * pwd = recv_string(socket_descriptor);
      printf("vous êtes situez : \n %s\n", pwd);

      int choix_menu = 10;
      int choix_cmd= 0;
      char * contents;
      int ack = 0;

    while(choix_menu != EXIT){
        printf("\n ---------------------------------------------\n");
        printf("Choissisez votre action en indiquant le numéro : \n");
        printf("1 - télécharger un fichier sur le serveur (PUT) \n");
        printf("2 - télécharger un ficher  du serveur (GET) \n");
        printf("3 - executer une commande sur le serveur (ls,pwd,cd) \n");
        printf("0 - quittez\n");
        scanf("%d",&choix_menu);

        switch(choix_menu){
            case UPLOAD:
                
                //on avertie le serveur de ce que l'on veux faire
                if(execute_action(UPLOAD,socket_descriptor)>0){

          

                     printf("indiquez le chemin absolue du fichier que vous souhaitez envoyer : \n");
                     // possibilité de faire une commande
                     scanf("%s",filePath);
                     //transfert du fichier.
                     //envoie du pwd pour que le serveur enregistre le fichier au bon endroit
                    
                     send_string(socket_descriptor,pwd);

                     //char cmd[100];
                     //snprintf(cmd,100,"%s %s",pwd,filePath);                   
  

                     if(file_exists(filePath)){                
                         transfert_fichier(socket_descriptor,filePath);
                         int rack=recv(socket_descriptor,&ack,sizeof(int),0);
                         if(rack>1 && ack==1){
                           printf("fichier correctement envoyée \n");
                         }else{
                            perror("erreur de l'envoie de du fichier !");
                         }
                     }else{
                      printf("erreur le fichier n'est pas  disponible sur votre espace de travail \n");

                  }
                    
                }

            break;
            case DOWLOAD:
                if(execute_action(DOWLOAD,socket_descriptor)>0){
                  printf("indiquez le fichier que vous souhaitez télécharger :\n");
                  //affichage du fichier courant
                  scanf("%s",filePath);
                 
                  //test de l'existance du fichier sur le serveur

                   //envoie du nom de fichier
                    send_string(socket_descriptor,filePath);

                     //file path null car on souhaite qu'il le télécharge dans le dossier courant
                    reception_fichier(&socket_descriptor,NULL); 
                 


                }else{
                    perror("l'action que vous avez demandé n'a pas pu aboutir");
                }
              

            break;
            case COMMAND:

                  printf("quels commandes souhaitez vous effectuer (indiquez le numéro): \n");
                  printf("[11] ce déplacer sur le serveur (CD) \n");
                  printf("[12] connaitre le chemin absolue (pwd) \n");
                  printf("[13] lister les fichiers disponible (ls) \n");
                  scanf("%d",&choix_cmd);

                  switch(choix_cmd){

                        case PWD_CMD:

                        printf("vous êtes : \n %s",pwd);

                        break;

                        case LS_CMD:

                        if(execute_action(LS_CMD,socket_descriptor)>0){
                          char cmd[100];
                          snprintf(cmd,100,"ls %s ",pwd);
                          int fok=0;

                          // on envoie le fichier dans lequel on veux se déplacer.
                          send_string(socket_descriptor,pwd);
                          // on reçois que le serveur possède bien que le dossier.
                          int sf= recv(socket_descriptor,&fok,sizeof(int),0);

                          // on a recu le bit pour prévenir que le dossier est bien disponible.
                          if(sf>0){
                              if(fok){
                                
                                send_string(socket_descriptor,cmd);
                                char* ls=recv_string(socket_descriptor);
                                printf("\n contenu du dossier courant : \n");
                                printf("%s \n",ls);

                              }else{
                                printf(" le fichier n'est pas disponible sur le serveur. \n");
                              }

                          }else{
                            perror(" erreur de réception de la vérification de présence du fichier ou dossier sur le serveur");
                          }              
                              
                        }else{
                          perror(" l'action demander n'a pas pu aboutir");
                        }

                        break;

                        case CD_CMD:

                          if(execute_action(CD_CMD,socket_descriptor)>0){
                              printf("chemin (dossier ou ../) auquel accéder : \n ");
                              scanf("%s",filePath);
                              //envoie d'une demande au serveur pour vérifier l'existance du dossier demandé
                              send_string(socket_descriptor,filePath);
                              int fok=0;

                              //ici l'aquittement correspond a l'acceptation du fichier/dossier a atteindre
                              int rfok=recv(socket_descriptor,&fok,sizeof(int),0);

                              if(rfok>0 && fok==1){

                                if(!strstr(filePath,"../")){
                                    char cd[100]="";
                                    pwd=strtok(pwd,"\n");
                                    strcat(pwd,"/");
                                    strcat(pwd,filePath);
                                  }else{
                                    printf("on ne peut pas remonter dans le fichier encore");
                                  }


                              }else{
                                printf("le dossier que vous souhaitez atteindre est incorrecte \n");
                              }

                            //voir modification du pwd en fonction du chemin demander
                            // vérifier l'existance d'un fichier

                          }
                        break;
                  }


            break;

            case EXIT:
            close(socket_descriptor);

            printf("connexion avec le serveur fermee, fin du programme.\n");
            exit(0);
            break;

        }

    }
    return 0;
}
