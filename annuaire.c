// Modifié à partir du code de Robin L'Huilier

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
#include <ctype.h>


struct sousAnneau {
    int descripteur;
    struct sockaddr_in sockaddr_in;
};

struct paquet {
    int requete;
    int information;
    int information2;
    struct sockaddr_in adresse;
};

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


int recv2TCP(int sock, void* msg, int sizeMsg) {
    int taille;
    recvTCP(sock, &taille, sizeof(int));
    if (taille > sizeMsg) {
        printf("Problème, buffer trop petit! (taille attendue %i, taille réelle %i)\n", taille, sizeMsg);
        return -1;
    }
    return recvTCP(sock, msg, taille);
}

int send2TCP(int sock, void* msg, int sizeMsg) {
    sendTCP(sock, &sizeMsg, sizeof(int));
    return sendTCP(sock, msg, sizeMsg);
}

int createSocket() {
    int ds = socket(PF_INET, SOCK_STREAM, 0);
    if (ds == -1){
        perror("Problème création socket :");
        exit(1); 
    }
    printf("Création de la socket réussie \n");
    return ds;
}

struct sockaddr_in bindSocket(char* port, int ds) {
    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    if(atoi(port) != -1) {
        ad.sin_port = htons((short) atoi(port));
    }
    printf("Port: %hu\n", htons(ad.sin_port));
    int res;
    res = bind(ds, (struct sockaddr*)&ad, sizeof(ad));
    if (res == 0) {
        printf("Socket nommée avec succès\n");
    } else {
        printf("Problème nommage socket : %i \n", res);
        exit(1);
    }
    return ad;
}

int listenTCP(int ds, int nbMaxAttente) {
    int resListen = listen(ds, nbMaxAttente);
    if (resListen == -1) {
        printf("Problème lors de l'écoute\n");
        exit(1);    
    } else {
        printf("En écoute\n");
    }
    
    return resListen;
}

int getListenAddr(int descripteur, struct sockaddr_in* sock) {
    struct paquet msg;
    
    if (recv2TCP(descripteur, &msg, sizeof(struct paquet)) <= 0) {
        printf("[SERVEUR] Problème lors de la réception de l'adresse d'écoute\n");
        return -1;
    }
    
    (*sock) = msg.adresse;
    return 1;
}

int sendAdrSubRing(struct sousAnneau sousAnneau[], int taille) {
    if (taille < 3) {
        printf("[SERVEUR] Impossible de créer un anneau avec moins de trois sous-anneaux\n");
        return -1;
    }
    struct paquet msg;
    msg.requete = 3;
    for (int i=0; i<taille-1; i++) {
        msg.adresse = sousAnneau[i+1].sockaddr_in;
        char adresse[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &msg.adresse.sin_addr, adresse, INET_ADDRSTRLEN);
        int port = htons(msg.adresse.sin_port);
        printf("[SERVEUR] Envoi de l'adresse: %s:%i\n", adresse, port);
        if (send2TCP(sousAnneau[i].descripteur, &msg, sizeof(struct paquet)) <= 0) {
            printf("[SERVEUR] Problème lors de l'envoi des adresses\n");
        }
    }
    msg.adresse = sousAnneau[0].sockaddr_in;
    char adresse[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &msg.adresse.sin_addr, adresse, INET_ADDRSTRLEN);
    int port = htons(msg.adresse.sin_port);
    printf("[SERVEUR] Envoi de l'adresse: %s:%i\n", adresse, port);
    if (send2TCP(sousAnneau[taille-1].descripteur, &msg, sizeof(struct paquet)) <= 0) {
        printf("[SERVEUR] Problème lors de l'envoi des adresses\n");
    }
    return 0;
}

int removeRingList(struct sousAnneau sousAnneaux[], int descripteur, int taille) {
    int index;
    int max = sousAnneaux[0].descripteur;
    for (index=0; index<taille; index++) {
        if (sousAnneaux[index].descripteur == descripteur) break;
        if (max < sousAnneaux[index].descripteur) max = sousAnneaux[index].descripteur;
    }
    if (index == taille) return -1;

    while (index < taille-1) {
        sousAnneaux[index] = sousAnneaux[index+1];
        if (max < sousAnneaux[index].descripteur) max = sousAnneaux[index].descripteur;
        index++;
    }
    return max;
}

void removeRing(
    int descripteur, 
    fd_set* set,
    int* currentMax,
    int* maxDescripteur,
    struct sousAnneau sousAnneaux[]
) {
    FD_CLR(descripteur, set);
    printf("[SERVEUR] Un sous-anneau s'est déconnecté\n");
    close(descripteur);
    int newMax = removeRingList(sousAnneaux, descripteur, (*currentMax));
    if (newMax == -1) {
        printf("[SERVEUR] Problème: le sous-anneau n'était pas dans la liste\n");
    } else {
        (*currentMax)--;
        (*maxDescripteur) = newMax;
    }
}

int setRingNum(int descripteur, int* currentMax, struct sousAnneau sousAnneaux[], struct sockaddr_in sockClient, fd_set* set,int* maxDescripteur
) {
    char adresse[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &sockClient.sin_addr, adresse, INET_ADDRSTRLEN);
    printf("[SERVEUR] Connexion d'un sous-anneau: %s\n", adresse);
    FD_SET(descripteur, set);
    if ((*maxDescripteur) < descripteur) (*maxDescripteur) = descripteur;
    sousAnneaux[(*currentMax)].descripteur = descripteur;
    sousAnneaux[(*currentMax)].sockaddr_in = sockClient;
    struct paquet attribution;
    attribution.requete = 2;
    attribution.information = (*currentMax);
    (*currentMax)++;
    return send2TCP(descripteur, &attribution, sizeof(struct paquet));
}

void dispListRing(struct sousAnneau sousAnneaux[], int taille) {
    char adresse[INET_ADDRSTRLEN];
    printf("[SERVEUR] Liste des sous-anneaux:\n");
    for (int i=0; i<taille; i++) {
        inet_ntop(AF_INET, &sousAnneaux[i].sockaddr_in.sin_addr, adresse, INET_ADDRSTRLEN);
        int port = htons(sousAnneaux[i].sockaddr_in.sin_port);
        printf("[SERVEUR] %i) %i : %s:%i\n", i, sousAnneaux[i].descripteur, adresse, port);
    }
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

int * extractNumbers(char* line, int number)
{
    const char* s = line;
    int i1, i2;
    if (2 == sscanf(s, "%*[^0123456789]%i%*[^0123456789]%i", &i1, &i2))
    {
        if(number == 0)
            return i1;
        else if (number == 1)
        {
            return i2;
        } else {
            exit;
        }
    }
}

int nextPlace(int tab[], int size){
    int i = 0;
    while (i <= size && tab[i] != 0){
        i++;
    }
    return i;
}
    

int main(int argc, char *argv[])
{
  // Mise en place du serveur
  if (argc != 3)
  {
    printf("[Server] Utilisation : [port_serveur] [File_Name]\n");
    exit(1);
  }

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
  char c;
  if(file == NULL){
    perror("Fichier : erreur ouverture fichier \n");
    free(filepath);
    exit(1);   
  }
  char* edges [nodeNumber];
  for(int i = 0; i < nodeNumber; i++){
    edges[i] = "";
  }

  int bufferLength = 255;
  char buffer[bufferLength];

  //Récupération du nombres de noeuds connectés à chaque noeuds

  int nodesTabSize = nodeNumber;
  int nodesTab[250] = {0};

  while(fgets(buffer, bufferLength, file)) {
      if(buffer[0] == 'e'){ 
        int n = extractNumbers(buffer, 0);
        int n2 = extractNumbers(buffer, 1);
        nodesTab[n]++;
        nodesTab[n2]++;
      }
  }

  for(int i = 0; i < nodesTabSize; i++){
    if(nodesTab[i] != 0){
        printf("Tableau nodes indice : %i nombre de noeuds connectés : %i\n", i, nodesTab[i]);
    }
  }

  fclose(file); 

  //Nouvelle boucle pour stocker les noeuds connectés
  FILE* file2 = fopen(filepath, "r");
  if(file2 == NULL){
    perror("Fichier : erreur ouverture fichier \n");
    free(filepath);
    exit(1);   
  }

  int* edgesConnexionTab[250] = {0};

  for(int i = 0; i < nodeNumber; i++){
    edgesConnexionTab[i] = malloc(sizeof(int) * nodesTab[i]);
  }  

  while(fgets(buffer, bufferLength, file)) {
      if(buffer[0] == 'e'){ 
        int n = extractNumbers(buffer, 0);
        int n2 = extractNumbers(buffer, 1);
        edgesConnexionTab[n][nextPlace(edgesConnexionTab[n], nodesTab[n])] = n2;
        edgesConnexionTab[n2][nextPlace(edgesConnexionTab[n2], nodesTab[n2])] = n;
      }
  }

  int k = 0;
  for (int i = 1; i <= nodeNumber; i++) {
        // pointer to hold the address of the row
        int* ptr = edgesConnexionTab[i];
        printf("%i | Noeuds connectées : ", i);
        for (int j = 0; j < nodesTab[k]; j++) {
            printf("%d ", *ptr);
            ptr++;
        }
        printf("\n");
        k++;
        edgesConnexionTab[i]++;
    }

  fclose(file2); 

  printf("Fichier : Le nombre d'anneaux nécessaires est %i \n", nodeNumber);
  //fclose(file); 
   
  //printf("Fichier : Lecture terminée, total lu : %d octets \n", total_lu); 

  // Création du serveur
  int nombreSousAnneaux = nodeNumber;
  int ds = createSocket();
  bindSocket(argv[1],ds);
  listenTCP(ds, 100);

  // Création de la socket client
   fd_set set, settmp;
    int dsClient;
    FD_ZERO(&set);
    FD_SET(ds, &set);
    int maxDescripteur = ds;
    struct sockaddr_in sockClient; 
    socklen_t lgAdr;
    int nbMaxAnneau = 100;
    struct sousAnneau sousAnneaux[nbMaxAnneau];
    int currentMaxAnneau = 0;

    while (1) {
        settmp = set;
        if (select(maxDescripteur+1, &settmp, NULL, NULL, NULL) == -1) {
            printf("[SERVEUR] Problème lors du select\n");
            continue;
        }

        for (int df=2; df <= maxDescripteur; df++) {
            if (!FD_ISSET(df, &settmp)) {
                continue;
            }

            if (df == ds) {
                dsClient = accept(ds, (struct sockaddr*)&sockClient, &lgAdr);
                if (getListenAddr(dsClient, &sockClient) < 0) {
                    continue;
                }
                if (setRingNum(dsClient, &currentMaxAnneau, sousAnneaux, sockClient, &set, &maxDescripteur) <= 0) {
                    removeRing(dsClient, &set, &currentMaxAnneau, &maxDescripteur, sousAnneaux);
                } else {
                    dispListRing(sousAnneaux, currentMaxAnneau);
                }
                continue;
            }
        }

        if (currentMaxAnneau == nombreSousAnneaux) {
            printf("[SERVEUR] Tous les anneaux sont connectés, envoi des adresses\n");
            if (sendAdrSubRing(sousAnneaux, currentMaxAnneau) == -1) {
                printf("[SERVEUR] Problème lors de l'envoi des adresses\n");
            } else {
                printf("[SERVEUR] Envoi réussi\n");
            }
            if (close(ds) == -1) {
                printf("[SERVEUR] Problème lors de la fermeture du descripteur\n");
            }
            printf("[SERVEUR] Au revoir.\n");
            exit(0);
        }
    }

    //libération des tableaux
    for(int i = 0; i < nodeNumber; i++){
        free(edgesConnexionTab[i]);
    } 
}