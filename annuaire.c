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


int main(int argc, char *argv[])
{
  /* etape 0 : gestion des paramètres si vous souhaitez en passer */
  if (argc > 2)
  {
    printf("Utilisation : [port_serveur]\n");
    exit(1);
  }
  int numberClient = 3;

  char *adresses[numberClient];
  char * ( *ptr )[numberClient] = &adresses;
  
  for (int i = 0; i < numberClient; i++)
         ( *ptr )[i] = ".";

  struct sockaddr_in socket_srv;
  socklen_t size = sizeof(struct sockaddr_in);
  /* etape 1 : creer une socket d'écoute des demandes de connexions*/
  int srv = socket(PF_INET, SOCK_STREAM, 0);
  if (srv == -1)
  {
    perror("Serveur : problème lors de la création de la socket");
    exit(1);
  }
  printf("Serveur : création de la socket réussi !\n");
  /* etape 2 : nommage de la socket */
  socket_srv.sin_family = AF_INET;
  socket_srv.sin_addr.s_addr = INADDR_ANY;
  socket_srv.sin_port = htons((short)atoi(argv[1]));
  int res = bind(srv, (struct sockaddr *)&socket_srv, sizeof(socket_srv));

  if (res == -1)
  {
    perror("Serveur : problème lors du nommage de la socket");
    exit(1);
  }

  /* etape 3 : mise en ecoute des demandes de connexions */
  int srvListen = listen(srv, 2);
  if (srvListen == -1)
  {
    perror("Serveur : problème lors de la mise en écoute de la socket");
    exit(1);
  }
  printf("Serveur : socket serveur sur écoute.\n");
  /* etape 4 : plus qu'a attendre la demande d'un client */

    for (size_t i = 0; i < numberClient; i++) {
    struct sockaddr_in sock_clt;
    int newConnection = accept(srv, (struct sockaddr *)&sock_clt, &size);

    if (newConnection == -1)
    {
      perror("[Serveur] : problème lors de la connexion d'un client");
      exit(1);
    }

      printf("[SERVEUR] Le client connecté est %s:%i.\n",      inet_ntoa(sock_clt.sin_addr), ntohs(sock_clt.sin_port));

      char adr[23];
      char port[5];
      sprintf(port, "%d",  ntohs(sock_clt.sin_port)); 
      strcat(adr,inet_ntoa(sock_clt.sin_addr));
      strcat(adr,port);
      
      ( *ptr )[1] = adr;
    
      close(newConnection);
  }
  printf("Serveur : c'est fini\n");

  for (int i = 0; i < numberClient; i++) {
    printf("String %i : %s\n", i+1, adresses[i] );
  }

  for (int i = 0; i < numberClient; i++) {
    if(i == numberClient - 1){
      printf("envoi ici du dernier élément avec le premier\n");
    }
    printf("Affectation des adresses aux clients : client %i : %s\n reçois %i : %s", i+1, adresses[i], i+2, adresses[i+1]);
  }

  close(srv);

}