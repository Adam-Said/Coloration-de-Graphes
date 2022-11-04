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

int main(int argc, char *argv[]) {

  if (argc != 4){
    printf("utilisation : client ip_serveur port_serveur port_client\n");
    exit(0);
  }

  /* etape 1 : créer une socket */   
  int ds = socket(PF_INET, SOCK_STREAM, 0);
  if (ds == -1){
    perror("[Client] : pb creation socket :\n");
    exit(1);
  }
  printf("[Client] : creation de la socket réussie \n");
  /* etape 1.2 : nommage de la socket client */
  struct sockaddr_in sock_clt;
  socklen_t size =sizeof(struct sockaddr_in);
  sock_clt.sin_family = AF_INET;
  sock_clt.sin_addr.s_addr = INADDR_ANY;
  sock_clt.sin_port = htons((short)atoi(argv[3]));
  int res = bind(ds, (struct sockaddr*) &sock_clt, size);

  if (res == -1){
      perror("[Client] : pb nommage socket :\n");
      exit(1);
  }
  /* etape 2 : designer la socket du serveur */

  int dsServ = socket(PF_INET, SOCK_STREAM, 0);
  if (dsServ == -1)
  {
      perror("Problème creation socket :");
      exit(1);
  }
  printf("Création de la socket réussie \n");

  struct sockaddr_in sock_srv;
  sock_srv.sin_family = AF_INET;
  sock_srv.sin_addr.s_addr = inet_addr(argv[1]);
  sock_srv.sin_port = htons(atoi(argv[2]));
  socklen_t lgAdr = sizeof(struct sockaddr_in);
  
  int resConnexion = connect(dsServ, (struct sockaddr *)&sock_srv, lgAdr);
  if( resConnexion == 0){
    printf("[Client] Première connexion au serveur réussie\n");
  } else {
    perror("Erreur lors de la connexion au serveur\n");
    exit(0);
  }

  struct paquet msg;
  msg.requete = 1;
  /*if (sendTCP(dsServ, &msg, sizeof(struct paquet)) <= 0)
  {
      printf("Problème lors de l'envoi de l'adresse d'écoute\n");
  }

  if (recvTCP(dsServ, &msg, sizeof(struct paquet)) <= 0)
  {
      printf("Probleme lors de la réception du numéro attribué\n");
      exit(0);
  }*/

  int sock_list = listen(ds, 1000);
    if (sock_list == -1)
    {
        perror("Client : problème lors de la mise en écoute de la socket");
        exit(1);
    }
    printf("Client : socket client sur écoute.\n");

  fd_set set, settmp;
  FD_ZERO(&set); //initialisation à 0 des booléens de scrutation
  FD_SET(sock_list, &set); //ajout de la socket client au tableau de scrutation
  FD_SET(dsServ, &set); //ajout de la socket client au tableau de scrutation
  int maxDesc = sock_list;

  while(1){
    settmp = set;
    if (select(maxDesc+1, &settmp, NULL, NULL, NULL) == -1) {
      printf("[CLIENT] Problème lors du select\n");
      continue;
    }
    select(maxDesc+1, &settmp, NULL, NULL, NULL);
    for(int df = 2; df <= maxDesc; df++){
      if(!FD_ISSET(df, &settmp)) continue;
      if(df == ds){
        int dsC = accept(sock_list, NULL, NULL);
        FD_SET(dsC, &set);
        if(maxDesc < dsC) maxDesc = dsC;
        continue;
      }
      char msg[4000];
      if(recv(df, msg, sizeof(msg), 0) <= 0){
        FD_CLR(df, &set); 
        printf("la socket se retire\n");
        close(df);
        continue;
      }
      char len[400];
      send(df, len, strlen(len) + 1, 0);
    }
  }


  /* etape 3 : demander une connexion 
  int dsConnect = connect(ds, (struct sockaddr*)&sock_srv, sizeof(sock_srv));
  if (dsConnect == -1){
      perror("[Client] : pb connexion serveur :\n");
      exit(1);
  }
  printf("[Client] : connexion réussie\n");*/


  printf("Travail terminé le client s'arrête\n");
 
  // fermeture socket
  if(close(ds) == -1) {
    printf("[Client] : pb fermeture socket\n");
    exit(1);
  }
  printf("Client : socket fermée !\n");
  printf("Client : c'est fini\n");

  return 0;
}