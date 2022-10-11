#include <netinet/in.h>
#include <stdio.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include<arpa/inet.h>
#include<string.h>
#include <sys/stat.h>


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
  struct sockaddr_in sock_srv;
  sock_srv.sin_family = AF_INET;
  sock_srv.sin_addr.s_addr = inet_addr(argv[1]);
  sock_srv.sin_port = htons(atoi(argv[2]));

  /* etape 3 : demander une connexion */
  int dsConnect = connect(ds, (struct sockaddr*)&sock_srv, sizeof(sock_srv));
  if (dsConnect == -1){
      perror("[Client] : pb connexion serveur :\n");
      exit(1);
  }
  printf("[Client] : connexion réussie\n");


    sleep(100);
 
  // fermeture socket
  if (close(ds) == -1) {
    printf("[Client] : pb fermeture socket\n");
    exit(1);
  }
  printf("Client : socket fermée !\n");
  printf("Client : c'est fini\n");

  return 0;
}
