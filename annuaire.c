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

int listenTCP(int ds) {
    int resListen = listen(ds, 2);
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

void removeRing(int descripteur, fd_set* set,int* currentMax,int* maxDescripteur,struct sousAnneau sousAnneaux[]
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
    afficheLigne();
    printf("[SERVEUR] Liste des sous-anneaux:\n");
    for (int i=0; i<taille; i++) {
        inet_ntop(AF_INET, &sousAnneaux[i].sockaddr_in.sin_addr, adresse, INET_ADDRSTRLEN);
        int port = htons(sousAnneaux[i].sockaddr_in.sin_port);
        printf("[SERVEUR] %i) %i : %s:%i\n", i, sousAnneaux[i].descripteur, adresse, port);
    }
    afficheLigne();
}


int main(int argc, char *argv[])
{
  // Mise en place du serveur
  if (argc != 3)
  {
    printf("[Server] Utilisation : [port_serveur] [number_of_ring]\n");
    exit(1);
  }

  // Création du serveur
  int nombreSousAnneaux = atoi(argv[2]);
  int ds = createSocket();
  bindSocket(argv[1],ds);
  listenTCP(ds);

  // Création de la socket client
  fd_set set, settmp;
  int dsClient;
  FD_ZERO(&set);
  FD_SET(ds, &set);
  int maxDs = ds;
  struct sockaddr_in sockClient; 
  socklen_t lgAdr;
  int MAX_RING = 100;
  struct sousAnneau sousAnneaux[MAX_RING];
  int currentNbRing = 0;

  while (1)
  {
    settmp = set;
      if (select(maxDs+1, &settmp, NULL, NULL, NULL) == -1) {
        printf("[SERVEUR] Problème lors du select\n");
        continue;
      }

      for (int df=2; df <= maxDs; df++) {
        if (!FD_ISSET(df, &settmp)) {
            continue;
        }

        if (df == ds) {
            dsClient = accept(ds, (struct sockaddr*)&sockClient, &lgAdr);
            if (getListenAddr(dsClient, &sockClient) < 0) {
                continue;
            }
            if (setRingNum(dsClient, &currentNbRing, sousAnneaux, sockClient, &set, &maxDs) <= 0) {
                removeRing(dsClient, &set, &currentNbRing, &maxDs, sousAnneaux);
            } else {
                dispListRing(sousAnneaux, currentNbRing);
            }
            continue;
        }
      }

      if (maxDs == nombreSousAnneaux) {
            printf("[SERVEUR] Tous les anneaux sont connectés, envoi des adresses\n");
            if (envoyerAdressesSousAnneaux(sousAnneaux, maxDs) == -1) {
                printf("[SERVEUR] Problème lors de l'envoi des adresses\n");
            } else {
                printf("[SERVEUR] Envoi réussi\n");
            }
            if (close(ds) == -1) {
                printf("[SERVEUR] Problème lors de la fermeture du descripteur\n");
                afficheErreur();
            }
            printf("[SERVEUR] Au revoir.\n");
            exit(0);
        }


  }
  

}