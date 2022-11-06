#include <netinet/in.h>
#include <stdio.h> //perror
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h> //close
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/select.h>
#include <math.h>

struct paquet {
    int socket;
    struct sockaddr_in adresse;
};

int getFirstNumber(char *fileName){
    char* texte = fileName;
	char nombre[10];
	char * ptexte = texte ;
	char * pnombre = nombre ;
	while(*ptexte && !isdigit(*ptexte)) ptexte++;
    while(isdigit(*ptexte))
	{
		*pnombre = *ptexte ;
		pnombre++;
		ptexte++;
	}
	*pnombre = '\0' ; 
	return atoi(nombre);
}


int extractNumbers(char* line, int number)
{
    const char* s = line;
    int i1, i2;
    if (2 == sscanf(s, "%*[^0123456789]%i%*[^0123456789]%i", &i1, &i2))
    {
        if(number == 0) {
            return i1;
          }
        else if (number == 1)
        {
            return i2;
        }
    }
  return 0;
}

int nextPlace(int * tab, int size){
    int i = 0;
    while (i <= size && tab[i] != 0){
        i++;
    }
    return i;
}


int main(int argc, char *argv[])
{
  /* etape 0 : gestion des paramètres si vous souhaitez en passer */
  if (argc != 3)
  {
    printf("Utilisation : [port_serveur] [File_name]\n");
    exit(1);
  }

    // Partie parser
    
    printf("Fichier à parser %s \n", argv[2]);

    //récupération du nombre de noeuds
    int nodeNumber = getFirstNumber(argv[2]);

    //récupération filepath
    char* filepath = malloc(strlen(argv[2]) + 16); // ./Files/+nom fichier +\0
    filepath[0] = '\0';
    strcat(filepath, "./Files/");
    strcat(filepath, argv[2]);
    //printf("Le fichier est : %s\n", filepath);

    // Lecture du fichier
    FILE* file = fopen(filepath, "r");
    if(file == NULL){
        perror("Fichier : erreur ouverture fichier \n");
        free(filepath);
        exit(1);   
    }

    int bufferLength = 255;
    char buffer[bufferLength];

    //Récupération du nombres de noeuds connectés à chaque noeud

    int* nodesTab = (int*)malloc(nodeNumber * sizeof(int));
    int** edgesConnexionTab = (int**)malloc(nodeNumber * sizeof(int*));

    for(int i = 0; i < nodeNumber; i++){
        nodesTab[i] = 0;
    }

    while(fgets(buffer, bufferLength, file)) {
        if(buffer[0] == 'e'){
            int n = extractNumbers(buffer, 0);
            nodesTab[n]++;
        }
    }

    for(int i = 1; i <= nodeNumber; i++){
        if(nodesTab[i] != 0){
            printf("Tableau nodes indice : %i nombre de noeuds connectés : %i\n", i, nodesTab[i]);
        }
    }

    fclose(file); 

    
    for(int i = 1; i <= nodeNumber; i++){
        edgesConnexionTab[i] = (int*)malloc(sizeof(int*) * nodesTab[i]);
    }  

    //Nouvelle boucle pour stocker les noeuds connectés
    FILE* file2 = fopen(filepath, "r");
    if(file2 == NULL){
        perror("Fichier : erreur ouverture fichier \n");
        free(filepath);
        exit(1);   
    }

    while(fgets(buffer, bufferLength, file)) {
        if(buffer[0] == 'e'){
            int n = extractNumbers(buffer, 0);
            int n2 = extractNumbers(buffer, 1);
            edgesConnexionTab[n][nextPlace(edgesConnexionTab[n], nodesTab[n])] = n2;
        }
    }

    fclose(file2);

    printf("Fichier : Le nombre d'anneaux nécessaires est %i \n", nodeNumber);

    // Fin parser

    int numberClient = nodeNumber;

    struct paquet* clientAdr = (struct paquet*)malloc(numberClient* sizeof(struct paquet));

    /* etape 1 : creer une socket d'écoute des demandes de connexions*/
    int srv = socket(PF_INET, SOCK_STREAM, 0);
    if (srv == -1){
        perror("Serveur : problème lors de la création de la socket");
        exit(1);
    }
    printf("Serveur : création de la socket réussi !\n");
    /* etape 2 : nommage de la socket */
    struct sockaddr_in sock_srv;
    sock_srv.sin_family = AF_INET;
    sock_srv.sin_addr.s_addr = INADDR_ANY;
    sock_srv.sin_port = htons((short)atoi(argv[1]));
    int res = bind(srv, (struct sockaddr *)&sock_srv, sizeof(sock_srv));
    if (res == -1){
        perror("Serveur : problème lors du nommage de la socket");
        exit(1);
    }
    printf("Serveur : nommage de la socket réussi !\n");

    /* etape 3 : mise en ecoute des demandes de connexions */
    int srvListen = listen(srv, 1000);
    if (srvListen == -1){
        perror("Serveur : problème lors de la mise en écoute de la socket");
        exit(1);
    }
    printf("Serveur : socket serveur sur écoute.\n");
  
    /* etape 4 : plus qu'a attendre la demande d'un client */

    fd_set set, settmp;
    FD_ZERO(&set); //initialisation à 0 des booléens de scrutation
    FD_SET(srv, &set); //ajout de la socket serveur au tableau de scrutation
    int maxDesc = srv;
    printf("[Serveur] : attente de connexion des clients.\n");
    while(1){
        struct paquet* voisinsAdr = (struct paquet*)malloc(nodeNumber * sizeof(struct paquet));
        settmp = set;
        if (select(maxDesc+1, &settmp, NULL, NULL, NULL) == -1) {
            printf("[Serveur] Problème lors du select\n");
        }
        printf("[Serveur] : select réussi !\n");
        for(int df = 2; df <= maxDesc; df++){
            //Acceptation de la connexion d'un client
            socklen_t size = sizeof(sock_srv);
            int dsC = accept(srv, (struct sockaddr *)&sock_srv, &size);
            printf("[SERVEUR] Le client connecté est %s:%i.\n",inet_ntoa(sock_srv.sin_addr), ntohs(sock_srv.sin_port));
            struct paquet paquet;

            int res = sendTCP(dsC, &nodesTab[df], sizeof(int), 0);
            if(res == -1) {
                perror("[Serveur] : problème lors de l'envoi du nombre d'arêtes");
                exit(1);
            }
            if(!FD_ISSET(df, &settmp)) FD_SET(dsC, &set);
            if(maxDesc < dsC) maxDesc = dsC;
            close(dsC);
            
        }
    }

    // fermeture socket
    if(close(srv) == -1) {
        printf("[Serveur] : pb fermeture socket\n");
        exit(1);
    }
    printf("Serveur : socket fermée !\n");
    printf("Serveur : c'est fini\n");

    
    close(srv);

    for(int i = 0; i < nodeNumber; i++){
        free(edgesConnexionTab[i]);
        free(&nodesTab[i]);
    } 

}