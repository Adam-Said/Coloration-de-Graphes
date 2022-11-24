#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <stdio.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include<arpa/inet.h>
#include<string.h>
#include <sys/stat.h>
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

int main(int argc, char *argv[]) {
  if (argc != 5){
    printf("utilisation : client ip_serveur port_serveur ip_client port_client\n");
    exit(0);
  }

  /* etape 1 : créer une socket */   
  int option = 1;
  int ds = socket(PF_INET, SOCK_STREAM, 0);
  setsockopt(ds, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  if (ds == -1){
    perror("[Client] : pb creation socket :\n");
    exit(1);
  }
  //printf("[Client] : creation de la socket réussie \n");
  /* etape 1.2 : nommage de la socket client */
  struct sockaddr_in sock_clt;
  socklen_t size =sizeof(struct sockaddr_in);
  sock_clt.sin_family = AF_INET;
  sock_clt.sin_addr.s_addr = inet_addr(argv[3]);
  sock_clt.sin_port = htons((short)atoi(argv[4]));
  int res = bind(ds, (struct sockaddr*) &sock_clt, size);

  if (res == -1){
      perror("[Client] : pb nommage socket :\n");
      exit(1);
  }

  /* etape 2 : designer la socket du serveur */
  int dsServ = socket(PF_INET, SOCK_STREAM, 0);
  setsockopt(dsServ, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  if (dsServ == -1)
  {
      perror("Problème creation socket :");
      exit(1);
  }
  //printf("Création de la socket réussie \n");

  struct sockaddr_in sock_srv;
  sock_srv.sin_family = AF_INET;
  sock_srv.sin_addr.s_addr = inet_addr(argv[1]);
  sock_srv.sin_port = htons(atoi(argv[2]));
  socklen_t lgAdr = sizeof(struct sockaddr_in);
  
  int resConnexion = connect(dsServ, (struct sockaddr *)&sock_srv, lgAdr);
  if( resConnexion == 0){
    printf("%s[Client] Première connexion au serveur réussie%s\n", AC_GREEN, AC_NORMAL);
  } else {
    perror("Erreur lors de la connexion au serveur\n");
    exit(0);
  }

  int sock_list = listen(ds, 1000);
  if (sock_list == -1){
        perror("[Client] : problème lors de la mise en écoute de la socket");
        exit(1);
  }
  //printf("[Client] : socket client sur écoute.\n");

  struct paquet msg;
  
  msg.adresse = sock_clt;
  if (sendTCP(dsServ, &msg, sizeof(struct paquet)) <= 0){
      printf("%s[Client] Problème lors de l'envoi de l'adresse d'écoute%s\n", AC_RED, AC_WHITE);
  }
  printf("%s[Client] Envoi de l'adresse d'écoute réussi%s\n", AC_GREEN, AC_NORMAL);

  int number;
  int color = 0;
  fd_set set;
  fd_set settmp;
  FD_ZERO(&set); //initialisation à 0 des booléens de scrutation
  FD_SET(ds, &set); //ajout de la socket client au tableau de scrutation
  FD_SET(dsServ, &set); //ajout de la socket client au tableau de scrutation
  int maxDesc = (sock_list > dsServ ) ? sock_list : dsServ;
  //printf("[Client] : attente de connexion du serveur.\n");
  while(1){
    settmp = set;
    if (select(maxDesc+1, &settmp, NULL, NULL, NULL) == -1) {
      printf("%s[CLIENT] Problème lors du select%s\n", AC_RED, AC_WHITE);
    }
    for(int df = 2; df <= maxDesc; df++){
      if(!FD_ISSET(df, &settmp)) continue;

      if(df == dsServ){
        //étape 1 : réception du nombre de noeuds auxquels se connecter
        //printf("[Client/Reception] Réception du nombre de voisins\n");
        int neighbors;
        int res = recvTCP(dsServ, &neighbors, sizeof(int));
        if (res == -1 || res == 0) {
            printf("%s[Client] Erreur lors de la reception du nombre de noeuds voisins%s\n", AC_RED, AC_WHITE);
            exit(0);
        }
        printf("[Client] Nombre de voisins en attente : %i voisins\n", neighbors);
        //étape 2 : boucle avec for et i<nombreNoeud reception un part un de chaque adresses de noeuds et stockage dans la struct + création socket
        struct paquet* voisinsAdr = (struct paquet*)malloc(neighbors * sizeof(struct paquet));
        if(neighbors != 0){
          //printf("[Client/Reception] Reception et stockage des adresses voisines\n");
          for(int i = 0; i < neighbors; i++){
            struct paquet adr;
            int reception = recvTCP(dsServ, &adr, sizeof(adr));
            if(reception == -1 || reception == 0){
              printf("%s[Client] Erreur lors de la reception de l'adresse d'un voisin%s\n", AC_RED, AC_WHITE);
              exit(0);
            }
            voisinsAdr[i].adresse = adr.adresse;
            //printf("[Client] Une adresse voisine est %s:%i.\n", inet_ntoa(voisinsAdr[i].adresse.sin_addr), ntohs(voisinsAdr[i].adresse.sin_port));
          }
          printf("%s[Client] Attente de l'ordre pour démarrer les connexions%s\n", AC_MAGENTA, AC_WHITE);
          //Reception de l'ordre de démarrer les connexions
          int ordre;
          int ordreReception = recvTCP(dsServ, &ordre, sizeof(ordre));
          if(ordreReception == -1 || ordreReception == 0){
            printf("%s[Client] Erreur lors de la reception de l'ordre de connexion%s\n", AC_RED, AC_WHITE);
            exit(0);
          }

          //Reception du numéro de noeud
          int nodenumberReception = recvTCP(dsServ, &number, sizeof(number));
          color = ((number%6)+1);
          if(color == 1) {
            color = 7;
          }
          color = color + '0';
          //printf("Couleur : %i", color);
          if(nodenumberReception == -1 || nodenumberReception == 0){
            printf("%s[Client] Erreur lors de la reception du numéro d'identification%s\n", AC_RED, AC_WHITE);
            exit(0);
          }

          sleep(2);

          printf("\x1B[3%cm[Client] %i) Ordre de connexion reçu, je démarre les connexions%s\n", color, number, AC_WHITE);

          //étape 3 : boucle de connexion
          printf("\x1B[3%cm[Client/Connexions] Le noeuds %i démarre les connexions aux voisins%s\n",color, number, AC_WHITE);
          for(int j = 0; j < neighbors; j++){
            printf("\x1B[3%cm[Client/Connexions] Tentative de connexion au noeud %i %s\n",color, j, AC_WHITE);
            struct sockaddr_in sock_voisin = voisinsAdr[j].adresse;
            
            socklen_t lgAdr = sizeof(struct sockaddr_in);
            int dsVoisins = socket(PF_INET, SOCK_STREAM, 0);
            setsockopt(dsVoisins, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
            if (dsVoisins == -1){
                printf("%s[Client] Problème lors de la creation de la socket pour la connexion voisine%s\n", AC_RED, AC_WHITE);
                exit(1);
            }
            int co = connect(dsVoisins, (struct sockaddr *)&sock_voisin, lgAdr);
            if(co == -1){
              printf("%s[Client/Connexion] Erreur lors de la connexion au voisin%s\n", AC_RED, AC_WHITE);
              exit(0);
            }
            voisinsAdr[j].socket = dsVoisins;
            printf("\x1B[3%cm[Client/Connexion] Connexion au voisin %i (%s:%i) réussie%s\n", color, j, inet_ntoa(voisinsAdr[j].adresse.sin_addr), ntohs(voisinsAdr[j].adresse.sin_port), AC_WHITE);
          }
          printf("\e[0;100m\x1B[3%cm[Client/Connexion] Noeud %i, toutes les connexions sont réussies\e[0m%s\n", color, number, AC_WHITE);
          continue;
        }

        FD_CLR(dsServ, &set);
        printf("Fermeture de la socket serveur\n");
        
      } else {
        //Acceptation de la connexion d'un autre client
        int dsC = accept(ds, (struct sockaddr *)&sock_clt, &size);
        setsockopt(dsC, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        FD_SET(dsC, &set);
        if(maxDesc < dsC) maxDesc = dsC;
      }
    }
  }

  printf("[Travail] terminé le client s'arrête\n");
 
  // fermeture socket
  if(close(ds) == -1) {
    printf("[Client] : Problème lors de la fermeture socket\n");
    exit(1);
  }
  printf("[Client] : Socket fermée !\n");

  return 0;
}
