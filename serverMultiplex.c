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

#define AC_BLACK "\x1b[30m"
#define AC_RED "\x1b[31m"
#define AC_GREEN "\x1b[32m"
#define AC_YELLOW "\x1b[33m"
#define AC_BLUE "\x1b[34m"
#define AC_MAGENTA "\x1b[35m"
#define AC_CYAN "\x1b[36m"
#define AC_WHITE "\x1b[37m"
#define AC_NORMAL "\x1b[m"

struct paquet {
    int socket;
    struct sockaddr_in adresse;
};

struct infosColor {
	char * receiveColor;
    int socket;
    struct sockaddr_in adresse;
    int state;
};

int isInArray(int * array, int size, int value) {
    int i;
    for (i = 0; i < size; i++) {
        if (array[i] == value) {
            return 1;
        }
    }
    return 0;
}

int compteUnique(int* T, int taille){
    int compteur = 0;
    int i, j;
    for(i = 0; i < taille; i++){
        int unique = 1;
        for(j = 0; j < taille; j++){
            if(T[i] == T[j] && j != i)
                unique = 0;
        }
        if(unique == 1)
            compteur++;
    }
    return compteur;
}

int sendTCP(int sock, void* msg, int sizeMsg) {
    int res;
    int sent = 0;
    while(sent < sizeMsg) {
        res = send(sock, msg+sent, sizeMsg-sent, 0);
        sent += res;
        if (res == -1) {
            printf("Problème lors de l'envoi du message\n");
            return -1;
        }
    }
    return sent;
}

int recvTCP(int sock, void* msg, int sizeMsg) {
    int res;
    int received = 0;
    while(received < sizeMsg) {
        res = recv(sock, msg+received, sizeMsg-received, 0);
        received += res;
        if (res == -1) {
            printf("Problème lors de la réception du message\n");
            return -1;
        } else if (res == 0) {
            return 0;
        }
    }
    return received;
}

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
    int* incomingTab = (int*)malloc(nodeNumber * sizeof(int));
    int** edgesConnexionTab = (int**)malloc(nodeNumber * sizeof(int*));
    for(int i = 1; i <= nodeNumber; i++){
        nodesTab[i] = 0;
        incomingTab[i] = 0;
    }
    while(fgets(buffer, bufferLength, file)) {
        if(buffer[0] == 'e'){
            int n = extractNumbers(buffer, 0);
            int n2 = extractNumbers(buffer, 1);
            nodesTab[n]++;
            incomingTab[n2]++;
        }
    }
    for(int i = 1; i <= nodeNumber; i++){
        if(nodesTab[i] != 0){
            printf("Tableau nodes indice : %i nombre de noeuds connectés : %i\n", i, nodesTab[i]);
            printf("%i attend %i connexions\n", i, incomingTab[i]);

        }
    }
    int totalConnexions = 0;
    int totalIncoming = 0;
    for(int i = 0; i <= nodeNumber; i++){
        totalConnexions = totalConnexions + nodesTab[i];
        totalIncoming = totalIncoming + incomingTab[i];
    }
    printf("[Serveur] Nombres de connexions (sockets) attendues : %i\n", totalConnexions);
    printf("[Serveur] Nombres de connexions (sockets) à faire : %i\n", totalIncoming);
    for(int i = 1; i <= nodeNumber; i++){
        if(nodesTab[i] != 0){
            edgesConnexionTab[i] = (int*)malloc(sizeof(int*) * nodesTab[i]);
            for (int j=0; j<nodesTab[i]; j++) edgesConnexionTab[i][j] = 0;
        } 
    }

    //Nouvelle boucle pour stocker les noeuds connectés
    fseek(file, 0, SEEK_SET);
    
    while(fgets(buffer, bufferLength, file)) {
        if(buffer[0] == 'e'){
            int n = extractNumbers(buffer, 0);
            int n2 = extractNumbers(buffer, 1);
            edgesConnexionTab[n][nextPlace(edgesConnexionTab[n], nodesTab[n])] = n2;
        }
    }
    fclose(file);

    int k = 1;
    for (int i = 1; i <= nodeNumber; i++) {
        printf("%i | Noeuds connectées : ", i);
        for (int j = 0; j < nodesTab[k]; j++) {
            printf(" %i ", edgesConnexionTab[i][j]);
        }
        printf("\n");
        k++;
    }

    printf("Fichier : Le nombre de noeuds nécessaires est %i \n", nodeNumber);

    // Fin parser

    /* etape 1 : creer une socket d'écoute des demandes de connexions*/
    int option = 1;
    int srv = socket(PF_INET, SOCK_STREAM, 0);
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if (srv == -1){
        perror("[Serveur] Problème lors de la création de la socket :");
        exit(1); 
    }
    printf("[Serveur] Création de la socket réussie \n");
    
    /* etape 2 : nommage de la socket */
    struct sockaddr_in sock_srv;
    sock_srv.sin_family = AF_INET;
    sock_srv.sin_addr.s_addr = INADDR_ANY;
    sock_srv.sin_port = htons((short)atoi(argv[1]));
    int res = bind(srv, (struct sockaddr *)&sock_srv, sizeof(sock_srv));
    if (res == -1){
        perror("[Serveur] : problème lors du nommage de la socket");
        exit(1);
    }

    /* etape 3 : mise en ecoute des demandes de connexions */
    int srvListen = listen(srv, 1000);
    if (srvListen == -1){
        perror("Serveur : problème lors de la mise en écoute de la socket");
        exit(1);
    }
  
    /* etape 4 : plus qu'a attendre la demande d'un client */

    fd_set set, settmp;
    int dsClient;
    FD_ZERO(&set); //initialisation à 0 des booléens de scrutation
    FD_SET(srv, &set); //ajout de la socket serveur au tableau de scrutation
    int maxDesc = srv;
    struct sockaddr_in sockClient; 
    socklen_t lgAdr;
    //struct paquet* voisinsAdr = (struct paquet*)malloc(nodeNumber * sizeof(struct paquet));
    struct paquet voisins[nodeNumber];
    int nodeIndex = 1;
    int* finalColors = (int*)malloc(nodeNumber * sizeof(int));
    int allSend = 0;
    while(1){
        settmp = set;
        if (select(maxDesc+1, &settmp, NULL, NULL, NULL) == -1) {
            printf("[Serveur] Problème lors du select\n");
        }
        for(int df = 2; df <= maxDesc; df++){
            if(df == srv){
                //socklen_t size = sizeof(sockClient);
                if (!FD_ISSET(df, &settmp)) {continue;}

                dsClient = accept(srv, (struct sockaddr *)&sockClient, &lgAdr);
                //struct paquet msg; useless
                if (recvTCP(dsClient, &voisins[nodeIndex], sizeof(struct paquet)) <= 0) {
                    printf("[Serveur] Problème lors de la réception de l'adresse d'écoute\n");
                    exit(0);
                }
                
                char adresse[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &voisins[nodeIndex].adresse.sin_addr, adresse, INET_ADDRSTRLEN);
                int port = htons(voisins[nodeIndex].adresse.sin_port);

                printf("[Serveur] %i) %s:%i\n", nodeIndex, adresse, port);
                int numVoisins = nodesTab[nodeIndex];
                numVoisins += incomingTab[nodeIndex];
                res = sendTCP(dsClient, &numVoisins, sizeof(int));
                if(res == -1) {
                    perror("[Serveur] : problème lors de l'envoi du nombre de connexions entrantes");
                    exit(1);
                }
                printf("%s[Serveur] Nombre de connexions entrantes du noeuds %i envoyé avec succès%s\n", AC_GREEN, nodeIndex, AC_NORMAL);
               
                // voisins[nodeIndex].adresse = sockClient; 
                voisins[nodeIndex].socket = dsClient;
                printf("[Serveur] : Envoi du nombre de voisins du noeuds %i\n", nodeIndex);
                res = sendTCP(dsClient, &nodesTab[nodeIndex], sizeof(int));
                if(res == -1) {
                    perror("[Serveur] : problème lors de l'envoi du nombre d'arêtes");
                    exit(1);
                }
                printf("%s[Serveur] Nombre de voisins du noeuds %i envoyé avec succès%s\n", AC_GREEN, nodeIndex, AC_NORMAL);

                int nodeN = nodeIndex;
                if (sendTCP(dsClient, &nodeN, sizeof(nodeN)) <= 0) {
                    printf("[Serveur/Ordre] Problème lors de l'envoi du numéro de noeuds\n");
                }                 

                if(!FD_ISSET(df, &settmp)) FD_SET(dsClient, &set);
                if(maxDesc < dsClient) maxDesc = dsClient;
                nodeIndex++;
                continue;
            } 
        } 

        if (nodeIndex == nodeNumber+1) {
            //printf("[Serveur] Tous les anneaux sont connectés, envoi des adresses\n");

            //récupération des adresses correspondantes aux nombres dans edgesConnexionTab
            struct paquet ssAdr;

            for (int i = 1; i<=nodeNumber; i++) {
                if (nodesTab[i] != 0) {
                    printf("[Serveur/Envoi] Début de l'envoi des voisins du noeud %i\n", i);
                    for(int j = 0; j < nodesTab[i]; j++){ //parcours du sous tableau
                        ssAdr.adresse = voisins[edgesConnexionTab[i][j]].adresse;
                        //printf("[Serveur/Envoi] Envoi de l'adresse du noeud %i\n", edgesConnexionTab[i][j]);
                        char adresses[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &voisins[edgesConnexionTab[i][j]].adresse.sin_addr, adresses, INET_ADDRSTRLEN);
                        //int port = htons(voisins[edgesConnexionTab[i][j]].adresse.sin_port);
                        //printf("[Serveur/Envoi] ---> Envoi de l'adresse: %s:%i\n", adresses, port);
                        if (sendTCP(voisins[i].socket, &ssAdr, sizeof(struct paquet)) <= 0) {
                            printf("[Serveur/Envoi] Problème lors de l'envoi des adresses\n");
                        }
                    }
                    printf("[Serveur/Envoi] Toutes les adresses voisines du noeud %i ont été envoyées\n", i);
                } else {
                    printf("[Serveur/Envoi] Aucun noeuds à envoyer au noeuds %i, je passe au prochain\n", i);
                }
            }

            int ordre = 1;
            printf("[Serveur/Ordre] Envoi des ordres de connexion%i\n", ordre);
            for (int i = 1; i<=nodeNumber; i++) {
                if (nodesTab[i] != 0) {
                    if (sendTCP(voisins[i].socket, &ordre, sizeof(ordre)) <= 0) {
                            printf("[Serveur/Ordre] Problème lors de l'envoi de l'ordre\n");
                    }
                } else {
                    printf("[Serveur/Envoi] Aucun ordre à envoyer pour le noeud%i\n", i);
                }
            }
            allSend  = 1;
        }
        if(allSend == 1) break;
    } 

    printf("%sFin du multiplexage\n%s", AC_MAGENTA, AC_NORMAL);

    //attente des couleurs des clients
    printf("Attente des noeuds...\n");
    for(int i = 1; i <= nodeNumber; i++){
        int receivedColor = 0;
        res = recvTCP(voisins[i].socket, &receivedColor, sizeof(receivedColor));
        if (res == -1 || res == 0) {
            perror("[Serveur/Thread] Erreur lors de la reception de la couleur finale\n");
            exit(0);
        } else {
            finalColors[i-1] = receivedColor;
            printf("%s[Serveur/Thread] Couleur finale reçue : %i\n%s", AC_GREEN, finalColors[i-1], AC_NORMAL);
        } 
    } 

    printf("----------------------------------------------------\n");

    printf("%sCalcul du nombre de couleurs ...\n%s", AC_MAGENTA, AC_NORMAL);
    for(int i = 0; i < nodeNumber; i++){
        printf("Couleur n°%i : %i\n", i+1, finalColors[i]);
    }
    int nbColors = compteUnique(finalColors, nodeNumber); 
    printf("%sNombre de couleurs : %i\n%s", AC_BLUE, nbColors, AC_NORMAL);


    /*FD_CLR(srv, &set);
        if(close(srv) == -1) {
            printf("[Serveur] : Problème lors de la fermeture de la socket\n");
            exit(1);
        }
    printf("[Serveur] : Socket fermée !\n");*/

    for(int i = 0; i <= nodeNumber; i++){
        free(edgesConnexionTab[i]);
        free(&nodesTab[i]);
        free(&incomingTab[i]);
        free(&voisins[i]);
        free(&finalColors[i]);
    }
    //version fonctionnelle mac
}



