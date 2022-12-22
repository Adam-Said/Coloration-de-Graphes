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

struct nodeInfos {
    int socket;
    struct sockaddr_in address;
	int receiveColor;
	int sendColor;
    int state;
	int order;
};

// Fonction d'envoi de message en TCP à travers les sockets
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

// Fonction de réception de message en TCP à travers les sockets
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


// Renvoi le premier nombre contenu dans un nom de fichier
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

// Extrait la valeur numéro "number" dans une ligne "line"
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

// Renvoi la première case disponible (qui contient un 0) dans le tableau
int nextPlace(int * tab, int size){
    int i = 0;
    while (i <= size && tab[i] != 0){
        i++;
    }
    return i;
}


int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("Utilisation : [port_serveur] [File_name]\n");
		exit(1);
	}

	//récupération du nombre de noeuds total
    int nodeNumber = getFirstNumber(argv[2]);

    //récupération filepath
    char* filepath = malloc(strlen(argv[2]) + 16); // ./Files/+nom fichier +\0
    filepath[0] = '\0';
    strcat(filepath, "./Files/");
    strcat(filepath, argv[2]);

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
	int* connOut = (int*)malloc(nodeNumber * sizeof(int)); // Tableau contenant le nombre de connexions sortantes de chaque noeud i
	int* connIn = (int*)malloc(nodeNumber * sizeof(int)); // Tableau contenant le nombre de connexions entrantes de chaque noeud i
	int** connTab = (int**)malloc(nodeNumber * sizeof(int*)); // Tableau contenant les voisins sortants de chaque noeud i
	for(int i = 1; i <= nodeNumber; i++){
		connOut[i] = 0;
		connIn[i] = 0;
	}
	while(fgets(buffer, bufferLength, file)) {
		if(buffer[0] == 'e'){
			int n = extractNumbers(buffer, 0);
			int n2 = extractNumbers(buffer, 1);
			connOut[n]++;
			connIn[n2]++;
		}
	}

	for(int i = 1; i <= nodeNumber; i++){
		if(connOut[i] != 0){
			printf("Tableau nodes indice : %i nombre de noeuds connectés : %i\n", i, connOut[i]);
			printf("%i attend %i connexions\n", i, connIn[i]);
		}
	}

	int totalOut = 0;
	int totalIn = 0;
	for(int i = 0; i <= nodeNumber; i++){
		totalOut += connOut[i];
		totalIn += connIn[i];
	}

	for(int i = 1; i <= nodeNumber; i++){
		if(connOut[i] != 0){
			connTab[i] = (int*)malloc(sizeof(int*) * connOut[i]);
			for (int j=0; j<connOut[i]; j++) connTab[i][j] = 0;
		}
	}

	//Nouvelle boucle pour stocker les noeuds connectés
	fseek(file, 0, SEEK_SET);

	while(fgets(buffer, bufferLength, file)) {
		if(buffer[0] == 'e'){
			int n = extractNumbers(buffer, 0);
			int n2 = extractNumbers(buffer, 1);
			connTab[n][nextPlace(connTab[n], connOut[n])] = n2;
		}
	}

	fclose(file);

	int k = 1;
	for (int i = 1; i <= nodeNumber; i++) {
		printf("%i | Noeuds connectées : ", i);
		for (int j = 0; j < connOut[k]; j++) {
			printf(" %i ", connTab[i][j]);
		}
		printf("\n");
		k++;
	}

	printf("Fichier : Le nombre de noeuds nécessaires est %i \n", nodeNumber);
	// Fin parser

	// Creation de la socket de réception
	int option = 1;
	int socketSrv = socket(PF_INET, SOCK_STREAM, 0);
	setsockopt(socketSrv, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	if (socketSrv == -1){
		perror("[Serveur] Problème lors de la création de la socket :");
		exit(1); 
	}

	struct sockaddr_in sock_srv;
	sock_srv.sin_family = AF_INET;
	sock_srv.sin_addr.s_addr = INADDR_ANY;
	sock_srv.sin_port = htons((short)atoi(argv[1]));
	int res = bind(socketSrv, (struct sockaddr *)&sock_srv, sizeof(sock_srv));
	if (res == -1){
		perror("[Serveur] : problème lors du nommage de la socket");
		exit(1);
	}

	int srvListen = listen(socketSrv, 1000);
	if (srvListen == -1){
		perror("Serveur : problème lors de la mise en écoute de la socket");
		exit(1);
	}

	fd_set set, settmp;
	FD_ZERO(&set); //initialisation à 0 des booléens de scrutation
	FD_ZERO(&settmp);
	FD_SET(socketSrv, &set); //ajout de la socket serveur au tableau de scrutation
	int maxDesc = socketSrv;
	struct nodeInfos nodesTab[nodeNumber]; // Tableau avec les infos des noeuds
	int currentIndex = 1;
	int colorReady = 0;

	while(1){ // Début multiplexage
		settmp = set;
		// Attente d'un évènement
		if (select(maxDesc+1, &settmp, NULL, NULL, NULL) == -1) {
			printf("[Serveur] Problème lors du select\n");
		}
		// Lors d'un évènement parours des sockets pour trouver la socket déclenchée
		for(int df = 2; df <= maxDesc; df++){
			if(df == socketSrv){
				if (!FD_ISSET(df, &settmp)) {continue;}
				
				// Acceptation d'un nouveau client
				struct sockaddr_in sockClient; 
				socklen_t lgAdr;
				int dsClient = accept(socketSrv, (struct sockaddr *)&sockClient, &lgAdr);

				// Réception des infos du client
				if (recvTCP(dsClient, &nodesTab[currentIndex], sizeof(struct nodeInfos)) <= 0) {
					printf("[Serveur] Problème lors de la réception de l'adresse d'écoute\n");
					exit(0);
				}
				
				// Stockage des infos du client
				nodesTab[currentIndex].socket = dsClient;

				char adresse[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &nodesTab[currentIndex].address.sin_addr, adresse, INET_ADDRSTRLEN);
				int port = htons(nodesTab[currentIndex].address.sin_port);
				printf("[Serveur] %i) %s:%i\n", currentIndex, adresse, port);

				// Envoi au client de son nombre de voisins
				int nombreVoisins = connOut[currentIndex] + connIn[currentIndex];
				res = sendTCP(dsClient, &nombreVoisins, sizeof(int));
				if(res == -1) {
					perror("[Serveur] : problème lors de l'envoi du nombre de voisins total");
					exit(1);
				}
				printf("%s[Serveur] Nombre de voisins total du noeuds %i envoyé avec succès%s\n", AC_GREEN, currentIndex, AC_NORMAL);
				
				// Envoi au client de son nombre de connexions sortantes
				printf("[Serveur] : Envoi du nombre de voisins sortants au noeud %i\n", currentIndex);
				res = sendTCP(dsClient, &connOut[currentIndex], sizeof(int));
				if(res == -1) {
					perror("[Serveur] : problème lors de l'envoi du nombre de voisins sortants");
					exit(1);
				}
				printf("%s[Serveur] Nombre de voisins sortants du noeuds %i envoyé avec succès%s\n", AC_GREEN, currentIndex, AC_NORMAL);

				// Envoi au client son numéro de noeud
				if (sendTCP(dsClient, &currentIndex, sizeof(int)) <= 0) {
					printf("[Serveur/Ordre] Problème lors de l'envoi du numéro de noeuds\n");
				}

				// Ajout du client au multiplexage
				if(!FD_ISSET(df, &settmp)) FD_SET(dsClient, &set);
				if(maxDesc < dsClient) maxDesc = dsClient;
				currentIndex++;
			}
		}

		// Si on a reçu tous les clients
		if(currentIndex == nodeNumber + 1) {
			for (int i = 1; i <= nodeNumber; i++) {
				// Si nombre de connexion sortantes de i différent de 0
				if (connOut[i] != 0) {
					for(int j = 0; j < connOut[i]; j++){
						struct nodeInfos infosVoisin;
						infosVoisin.address = nodesTab[connTab[i][j]].address;
						if (sendTCP(nodesTab[i].socket, &infosVoisin, sizeof(struct nodeInfos)) <= 0) {
							printf("[Serveur/Envoi] Problème lors de l'envoi des adresses\n");
						}
					}
				} 
				// Aucune connexion sortante de i
				else {
					printf("[Serveur/Envoi] Aucun noeuds à envoyer au noeuds %i, je passe au prochain\n", i);
				}
			}

			// Démarrage des connexions entre clients
			int ordre = 1;
            for (int i = 1; i <= nodeNumber; i++) {
                if (connOut[i] != 0) {
                    if (sendTCP(nodesTab[i].socket, &ordre, sizeof(int)) <= 0) {
                            printf("[Serveur/Ordre] Problème lors de l'envoi de l'ordre\n");
                    }
                } else {
                    printf("[Serveur/Envoi] Aucun ordre à envoyer pour le noeud%i\n", i);
                }
            }
            colorReady = 1;
		}

		if (colorReady == 1) {
			printf("Début coloration");

		}
		
	}
	return 0;
}
