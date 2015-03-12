/*----------------------------------------------
  Serveur à lancer avant le client
  ------------------------------------------------*/
#define DIR_DL "/tmp/FTP_server"

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


/**
  fonction d'envoie de la sortie d'une commande system
  */

// a voir pour l'histoire d'un chemin fictif pour le pwd ou seulement dans les sous dossier du serveur.
void send_cmd(char * cmd,int socket){

    printf("commande :  %s\n",cmd);
    FILE* cmdF;
    char cmd_c[100]="";
    char cmd_t[100]="";
    char contents[300]="";

    strcat(cmd_t,cmd);

    //certain \n se retrouvait dans le code => suppression de ce symbole.
    delete_retC(cmd_t);

    cmdF = popen(cmd_t, "r");
    char tmp[200]="";


    if(cmdF!=NULL){
        while(fgets(contents,300, cmdF) != NULL){
            strcat(tmp,contents);
        }

        strcat(tmp, "\n");

        strcpy(contents,tmp);

    }else{
        perror("commande interrompu");
    }
    pclose(cmdF);


    send_string(socket,contents);

}


int main(int argc, char **argv) {

    int 	     	socket_descriptor_cmd, 		/* descripteur de socket */
                    nouv_socket_descriptor_cmd, 	/* [nouveau] descripteur de socket */
                    longueur_adresse_courante, /* longueur d'adresse courante d'un client */
                    action,
                    ack; // action courante demander par le client.


    sockaddr_in 	adresse_locale, 		/* structure d'adresse locale*/
                    adresse_client_courant; 	/* adresse client courant */
    hostent*		  ptr_hote; 			/* les infos recuperees sur la machine hote */

    char 		    machine[TAILLE_MAX_NOM+1]; 	/* nom de la machine locale */
    char        filePath[TAILLE_MAX_NOM];

    gethostname(machine,TAILLE_MAX_NOM);		/* recuperation du nom de la machine */
    /* recuperation de la structure d'adresse en utilisant le nom */
    if ((ptr_hote =gethostbyname(machine)) == NULL) {
        perror("erreur : impossible de trouver le serveur a partir de son nom.");
        exit(1);
    }


    // create directory of file server
    mkdir(DIR_DL, 1777);

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
       if ((ptr_service = getservbyname("ftp","tcp")) == NULL) {
       perror("erreur : impossible de recuperer le numero de port du service desire.");
       exit(1);
       }
       adresse_locale.sin_port = htons(ptr_service->s_port);
       */
    /*-----------------------------------------------------------*/
    /* SOLUTION 2 : utiliser un nouveau numero de port */
    adresse_locale.sin_port = htons(2000); // port de commande
    /*-----------------------------------------------------------*/

    printf("numero de port pour la connexion au serveur : %d \n",
           ntohs(adresse_locale.sin_port) /*ntohs(ptr_service->s_port)*/);

    /* creation de la socket principale de connexion "commande"*/
    if ((socket_descriptor_cmd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("erreur : impossible de creer la socket de connexion avec le client.");
        exit(1);
    }

    /* association du socket socket_descriptor à la structure d'adresse adresse_locale */
    if ((bind(socket_descriptor_cmd, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
        perror("erreur : impossible de lier la socket a l'adresse de connexion.");
        exit(1);
    }

    /* initialisation de la file d'ecoute */
    listen(socket_descriptor_cmd,5);

    /* attente des connexions et traitement des donnees recues */
    for(;;) {

        longueur_adresse_courante = sizeof(adresse_client_courant);

        /* adresse_client_courant sera renseigné par accept via les infos du connect */
        if ((nouv_socket_descriptor_cmd = accept(socket_descriptor_cmd, (sockaddr*)(&adresse_client_courant), &longueur_adresse_courante)) < 0) {
            perror("erreur : impossible d'accepter la connexion avec le client.");
            exit(1);
        }



        int pid = fork();

        if (pid == 0) {

            //envoie a la connexion du client du chemin ou il se trouve.
            char * pwd= "pwd";
            send_cmd("pwd",nouv_socket_descriptor_cmd);
            //ACK
            int actionR= recv(nouv_socket_descriptor_cmd,&action,sizeof(int),0);
            char * nomDeFichier;
            char * toDl;
            while(action != EXIT){

                if(action>0){ // si une action est envoyer par le client on la traite

                    switch(action){

                        case UPLOAD:
                            printf("action UPLOAD \n");
                            int upOK=0;

                            int rupok = recv(nouv_socket_descriptor_cmd,&upOK,sizeof(int),0);
                            printf("%d\n",upOK);
                            printf("%d\n", rupok );
                            if(rupok >0 && upOK){
                                //fichier sur le serveur ou déposer le fichier.
                                toDl = recv_string(nouv_socket_descriptor_cmd);
                                if(toDl != NULL ){

                                    if(reception_fichier(&nouv_socket_descriptor_cmd,toDl) == 1){
                                        ack=1;
                                        send(nouv_socket_descriptor_cmd,&ack,sizeof(int),0);
                                    }
                                }else{
                                    printf("erreur de reception du chemin ou sauvegarder le fichier");
                                }
                            }

                            break;

                        case DOWLOAD:
                            printf("action DOWNLAOD \n");
                            //reception du nom de fichier a envoyer au client
                            //todo vérification de la réception des éléments
                            int downok=0;

                            nomDeFichier= recv_string(nouv_socket_descriptor_cmd);

                            if(nomDeFichier != NULL){    //transfert du fichier demander;
                                if(file_exists(nomDeFichier)){
                                    downok = 1;

                                    if(send(nouv_socket_descriptor_cmd,&downok,sizeof(int),0)>0){

                                        transfert_fichier(nouv_socket_descriptor_cmd,nomDeFichier);
                                    }
                                }else {
                                    send(nouv_socket_descriptor_cmd,&downok,sizeof(int),0);
                                }

                            }


                            break;

                        case LS_CMD:
                            printf("action LS_CMD \n");
                            int fok=0;
                            int sfok=0;
                            char * filePath=recv_string(nouv_socket_descriptor_cmd);
                            delete_retC(filePath);

                            if(file_exists(filePath)){
                                fok=1;
                                sfok=send(nouv_socket_descriptor_cmd,&fok,sizeof(int),0);
                                if(sfok > 0){
                                    char * cmd=recv_string(nouv_socket_descriptor_cmd);
                                    send_cmd(cmd,nouv_socket_descriptor_cmd);
                                }else{

                                    perror("erreur d'envoie de la vérification du fichier/dossier");
                                }
                            }else{ // le dossier n'est pas disponible on renvoie 0;

                                sfok=send(nouv_socket_descriptor_cmd,&fok,sizeof(int),0);
                                if(sfok < 0){

                                    perror("erreur d'envoie de la vérification du fichier/dossier");
                                }
                            }


                            break;
                        case CD_CMD:
                            printf("action CD_CMD \n");
                            //récupération d'une demande de vérification
                            char * path=recv_string(nouv_socket_descriptor_cmd);
                            delete_retC(path);
                            int f_ok= file_exists(path);
                            printf("%d \n",f_ok);
                            send(nouv_socket_descriptor_cmd,&f_ok,sizeof(int),0);
                            break;
                    }

                    int ract=recv(nouv_socket_descriptor_cmd,&action,sizeof(int),0);

                    if(ract == 0 || ract < 0){
                        action = EXIT;
                        printf("problème  ou déconnexion du client \n" );
                        close(nouv_socket_descriptor_cmd);
                    }

                }



            }


        } else  if (pid== -1){
            perror("impossible de créer un fils");
            exit(1);
        }else{

            close(nouv_socket_descriptor_cmd);

        }


    }

    return 0;
}

