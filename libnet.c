#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h> 	/* pour les sockets */
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h> 		/* pour hostent, servent */
#include <string.h> 		/* pour bcopy, ... */
#include <fcntl.h> // for open
#include <unistd.h> // for close

#define TAILLE_MAX_NOM 250

#define BUFFER_MAX_SIZE 1500

#define UPLOAD 1
#define DOWLOAD 2
#define COMMAND 3
#define CD_CMD 11
#define PWD_CMD 12
#define	LS_CMD 13
#define EXIT 0


void delete_retC(char * chaine){
	char * t = strchr(chaine , '\n' );
	if( t ) *t = '\0';
}


/**
	test d'existence d'un fichier ou dossier.
*/
int file_exists (char * fileName)
{
   struct stat buf;
   int i = stat ( fileName, &buf );
     /* File found */
     if ( i == 0 )
     {
       return 1;
     }
     return 0;

}

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
    	perror("erreur de reception de la taille de chaine");
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

void transfert_fichier(int socket_descriptor,char * filePath){


  

  int bytes_written,bytes_to_write;

	FILE* fichier;
	fichier = fopen(filePath,"ab"); //"données.txt"
	printf(" chemin : %s\n",filePath);
	long fileSize = taille_fichier(fichier);

	//réouverture car l'ouverturein  précédente avec ab place la tête de lecture a la fin du fichier
	fichier = fopen(filePath,"rb");

	//réecriture de filepath en fileName si pas de / filepath n'est pas modifier
	exctract_file_name(filePath);
	printf(" nom du fichier : %s\n",filePath);
  printf(" taille du fichier a envoyée %ld\n",fileSize);
    //send du nom
    send_string(socket_descriptor,filePath);

    //envoie de la taille du fichier a recevoir.
    int sent=send(socket_descriptor,&fileSize,sizeof(long),0);

    //envoie du fichier.
    char sendBuffer[BUFFER_MAX_SIZE];
    //int bytesRead = fread(sendBuffer,1, sizeof(fileSize), fichier);
		bytes_to_write = fread(sendBuffer, 1, BUFFER_MAX_SIZE, fichier);
  /* Check for end-of-file */
    int sendi= 0;
    while (bytes_to_write) {
  /* Debug message */
  /*printf("[DEBUG] Buffering %ld bytes\n", bytes_to_write);*/
  /* Check for file error */
        if (ferror(fichier)) {
      /* Drop error message */
                puts("I/O file error !");
      
       }
  /* Send buffer */
    do {
    /* Send with error check */
        if ((bytes_written = send(socket_descriptor, sendBuffer, bytes_to_write,0)) == -1) {
        /* Drop error message */
        perror("write()");
        
        }
       sendi=sendi + bytes_written;
    /* Repeat send until no more data */
    } while (bytes_to_write - bytes_written > 0);
    /* Next read */
    bytes_to_write = fread(sendBuffer, 1, BUFFER_MAX_SIZE, fichier);
    }

    printf(" send : %d\n", sendi);
		//close(socket_descriptor);
    fclose(fichier);
  
}



/**
	fonction de réception de fichier
*/

int reception_fichier(void* sock, char * pathfile)
{

    //cast du socket
    int *tmp = (int*) sock;
    int socket_descriptor = *tmp;

    long  tailleFichier = 0;
    int tailleNomFichier = 0;
    int recu=0;
    int ack=0;
    FILE * fichier; // création du future fichier réassemblé.



    char * nomDeFichier= recv_string(socket_descriptor);
    printf(" nom du fichier reçu : %s\n", nomDeFichier);
    //reception de la taille du fichier
    int tailleRecu= recv(socket_descriptor,&tailleFichier,sizeof(long),0);


    //verification de la bonne réception d'un fichier -1 en cas d'erreur
    if(tailleRecu < 0){
    	perror("erreur de la réception de la taille du fichier");
    }

    printf("taille du fichier a recevoir: %ld \n", tailleFichier);
    printf("nom du fichier a recevoir : %s \n",nomDeFichier);

    char * path = NULL;

    if(pathfile == NULL ){
       path = malloc(sizeof(char)*(strlen(DIR_DL) + strlen(nomDeFichier) + 2));
      strcpy(path, DIR_DL);
      strcat(path, "/");
      strcat(path, nomDeFichier);


    }else{
       path = malloc(sizeof(char)*(strlen(pathfile) + strlen(nomDeFichier) + 2));
      //on supprime un retour a la ligne provenant de l'envoie précédent.
       delete_retC(pathfile);      
       
       strcpy(path, pathfile);
       strcat(path, "/");
       strcat(path, nomDeFichier);
    }
         
 
    fichier = fopen(path, "wb");
    if (fichier != NULL) {

       char recvBuff[BUFFER_MAX_SIZE];
      
        
        int bytesReceived = 0; //recv(socket_descriptor, recvBuff,BUFFER_MAX_SIZE, 0);
        int recu = bytesReceived;
        int rest = tailleFichier;
      
          while(recu != tailleFichier)
          {

            if(rest-BUFFER_MAX_SIZE>0){
              bytesReceived = recv(socket_descriptor, recvBuff, BUFFER_MAX_SIZE, 0);
              rest = rest - BUFFER_MAX_SIZE;
            }        
            else{
              bytesReceived = recv(socket_descriptor, recvBuff, rest, 0);
              rest = 0;
            }  
              fwrite(recvBuff, bytesReceived, 1, fichier);

              recu=recu+bytesReceived;
              
              printf(" %d \n ",recu);
                           
          }
            
               
           fclose(fichier);
		 }
  else
  {     
        perror("erreur impossible d'ouvrir le fichier");
  }

    printf("reception terminer \n");
 return 1;
}


