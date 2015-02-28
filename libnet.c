#define TAILLE_MAX_NOM 256
#define DIR_DL "/tmp/filesFTP" // a revoir !
#define BUFFER_MAX_SIZE 1500

#define UPLOAD 1
#define DOWLOAD 2
#define	COMMAND 3 
#define EXIT 4



/**
	envoie du code d'action au serveur.
*/
int execute_action(int action,int socket){

	  int sent=send(socket,&action,sizeof(int),0);

return sent;
}


/**
	fonction d'optention de la taille d'un fichier
*/

long taille_fichier(FILE* fichier){

	long tailleFichier=-1;

	if(fichier){
		tailleFichier=ftell(fichier);
		fclose (fichier);
	}

return tailleFichier;

}


/**
	fonction de réception d'une chaine
*/

char * recv_string(int socket){

	int tStrg; // taille de la string
    char * strg; // la string a recevoir

    if(recv(socket,&tStrg,sizeof(int),0)>0){

		strg= malloc(sizeof(char)*(tStrg+1));
     	//réception de la string
    	if(recv(socket,strg,sizeof(char)*(tStrg),0)>0){
    		strg[tStrg] = '\0';	
    	}
     	 else{
     		perror("erreur de reception de la chaine"); 	
     	 }

    }else{
    	perror("erreur de reception de la chaine");
    }

    return strg;
}


/**
	fonction d'envoie d'une chaine
*/

void send_string(int socket,char * chaine){

	int sizeS= strlen(chaine);

	if(send(socket,&sizeS,sizeof(int),0)>0){
		if(send(socket,chaine,strlen(chaine),0)<0){
			perror("problème d'envoie de la chaine");
		}
	}else{
		perror("problème d'envoie de la chaine");
	}
}


/**
	fonction d'extraction du nom du fichier a partir d'un chemin absolue ou relatif saisie
	!!! ATTENTION le pointeur est directement modifier  donc filepath=/doc/machin devient filepath=machin.!!

	TODO : renvoyer un pointeur vers la valeur du nom de fichier.
*/

void exctract_file_name(char * filePath){

    char * strTmp;
    char * toFree;
    
    toFree = strdup(filePath);

    while ((strTmp = strsep(&toFree, "/")) != NULL)
    {   
    
        strcpy(filePath,strTmp);
    }

    free(toFree);
                  
}

/**
	fonction d'envoie d'un fichier sur une socket.
	 
*/

int transfert_fichier(int socket_descriptor,char * filePath){

	FILE* fichier;
	fichier = fopen(filePath,"ab"); //"données.txt"
	printf(" chemin : %s\n",filePath);
	long fileSize = taille_fichier(fichier);

	//réouverture car l'ouverturein  précédente avec ab place la tête de lecture a la fin du fichier
	fichier = fopen(filePath,"rb");

	//réecriture de filepath en fileName si pas de / filepath n'est pas modifier
	exctract_file_name(filePath);
	printf(" nom du fichier : %s\n",filePath);
    
    //send du nom
    send_string(socket_descriptor,filePath);

    //envoie de la taille du fichier a recevoir.
    int sent=send(socket_descriptor,&fileSize,sizeof(long),0);

    //envoie du fichier.
    char sendBuffer[BUFFER_MAX_SIZE];
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



/**
	fonction de réception de fichier
*/

void* reception_fichier(void* sock)
{

    //cast du socket
    int *tmp = (int*) sock;
    int socketDescriptor = *tmp;

    long  tailleFichier = 0;
    int tailleNomFichier = 0;

    FILE * fichier; // création du future fichier réassemblé.

 

    char * nomDeFichier= recv_string(socketDescriptor);
    printf(" nom du fichier reçu : %s\n", nomDeFichier);
    //reception de la taille du fichier
    int tailleRecu= recv(socketDescriptor,&tailleFichier,sizeof(long),0);


    //verification de la bonne réception d'un fichier -1 en cas d'erreur
    if(tailleRecu > 0){
    	perror("erreur de la réception de la taille du fichier");
    }

    printf("taille du fichier a recevoir: %ld \n", tailleFichier);
    printf("nom du fichier a recevoir : %s \n",nomDeFichier);

    char * path = malloc(sizeof(char)*(strlen(DIR_DL) + strlen(nomDeFichier) + 2));
    strcpy(path, DIR_DL);
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
           //printf("%d\n",bytesReceived);
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
    
    //Envoyer une message disant que la réception a été bonne.
    close(socketDescriptor);


    

    return NULL;


}












