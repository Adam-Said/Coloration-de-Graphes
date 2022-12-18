#include <asm-generic/socket.h>
#include <pthread.h>
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

#define MAX_LENGTH 100

struct paquet {
    int socket;
    struct sockaddr_in adresse;
};

struct infosColor {
    char * sendColor;
		char * receiveColor;
    int socket;
    struct sockaddr_in adresse;
    int state;
};

char* int_to_string(int n) {
  // Allouer de l'espace pour la chaîne de caractères
  char* str = malloc(sizeof(char) * 12); // 12 est la taille maximale d'un entier en C
  // Convertir l'entier en chaîne de caractères
  sprintf(str, "%d", n);
  // Retourner la chaîne de caractères
  return str;
}

char* nextBinary(char* str) {
  // Générer un nombre aléatoire compris entre 0 et 1
  printf("%sChaîne d'origine : %s %s\n", AC_CYAN, str, AC_NORMAL);
  srand(time(NULL));
  int rand_num = rand() %2;
  char * num = int_to_string(rand_num);// Allouer de l'espace pour la nouvelle chaîne de caractères
  int len = strlen(str);
	printf("Taille de la chaîne d'origine : %i \n", len);
  char *new_str = (char*)malloc((len+2)*sizeof(char));
  //char* new_str = malloc(MAX_LENGTH * sizeof(char)); // Copier la chaîne de caractères d'origine dans la nouvelle chaîne
  strcpy(new_str, str);  // Ajouter le nombre aléatoire à la fin de la nouvelle chaîne
  strcat(new_str, num);
	printf("Nouvelle chaîne : %s \n", new_str);
  // Retourner la nouvelle chaîne de caractères 
  return new_str;
}

int sendTCP(int sock, void* msg, int sizeMsg) {
    int res;
    int sent = 0;
    while(sent < sizeMsg) {
        res = send(sock, msg+sent, sizeMsg-sent, 0);
        sent += res;
        if (res == -1) {
            perror("Problème lors de l'envoi du message\n");
            return -1;
        }
				if (res == 0) {
						perror("Socket fermée\n");
						return 0;
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
            perror("Problème lors de la réception du message\n");
            return -1;
        } else if (res == 0) {
						perror("Socket fermée\n");
            return 0;
        }
    }
		
    return received;
}

void * recevoirCouleur (void * param){
	
  struct infosColor * args = (struct infosColor *) param;
  char newColor[MAX_LENGTH];
  int dsVois = args->socket;

	struct sockaddr_in adresse = args->adresse;

	char castAdresse[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &adresse.sin_addr, castAdresse, INET_ADDRSTRLEN);
	//int port = htons(args->adresse.sin_port);

	//printf("[Client/Thread] Réception de %s:%i\n", castAdresse, port);
	size_t colorSize = 0;
  int res = recvTCP(dsVois, &colorSize, sizeof(size_t));
  if (res == -1) {
      perror("[Client/Thread] Erreur lors de la reception de la taille couleur\n");
      exit(0);
  }
	else
	{
		printf("[Client/Thread] Taille Couleur reçue : %zu\n", colorSize);
	}
	
  res = recvTCP(dsVois, &newColor, colorSize*sizeof(char));
  if (res == -1 || res == 0) {
      perror("[Client/Thread] Erreur lors de la reception de la couleur\n");
      exit(0);
  }
	else
	{
		printf("[Client/Thread] Couleur reçue : %s\n", newColor);
	}
  newColor[colorSize] = '\0';
	sprintf(args->receiveColor, "%s", newColor);
  //args->color = newColor;
  pthread_exit(NULL);
}

void * envoyerCouleur (void * param){ 
  struct infosColor * args = (struct infosColor *) param;
  char * newColor = args->sendColor;
  int ds = args->socket;
	struct sockaddr_in adresse = args->adresse;

	char castAdresse[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &adresse.sin_addr, castAdresse, INET_ADDRSTRLEN);
	//int port = htons(args->adresse.sin_port);

	//printf("[Client/Interconnexions] Envoi à %s:%i\n", castAdresse, port);
	size_t colorSize = strlen(newColor);
	if (sendTCP(ds, &colorSize, sizeof(size_t)) == -1) {
    perror("[Client/Thread] Problème lors de l'envoi de la taille de la couleur \n");
  }
	else
	{
		printf("[Client/Thread] Taille couleur envoyée : %zu\n", colorSize);
	}

  if (sendTCP(ds, newColor, colorSize*sizeof(char)) == -1) {
    perror("[Client/Thread] Problème lors de l'envoi de la couleur\n");
  }
	else
	{
		printf("[Client/Thread] Couleur envoyée : %s\n", newColor);
	}
  pthread_exit(NULL);
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

  struct paquet msg;
  
  msg.adresse = sock_clt;
  if (sendTCP(dsServ, &msg, sizeof(struct paquet)) <= 0){
      printf("%s[Client] Problème lors de l'envoi de l'adresse d'écoute%s\n", AC_RED, AC_WHITE);
  }
  printf("%s[Client] Envoi de l'adresse d'écoute réussi%s\n", AC_GREEN, AC_NORMAL);
  
  int allNeighbors = 0; //nombres de Connexion en attente reçu du serveur
  res = recvTCP(dsServ, &allNeighbors, sizeof(int));
  if (res == -1 || res == 0) {
      printf("%s[Client] Erreur lors de la reception du nombre de connexions totales %s\n", AC_RED, AC_WHITE);
      exit(0);
  }

  int number = 0; //numéro d'identification du client
  srand(time(NULL));
  int color = 0;
  fd_set set;
  fd_set settmp;
  FD_ZERO(&set); //initialisation à 0 des booléens de scrutation
  FD_SET(ds, &set); //ajout de la socket client au tableau de scrutation
  FD_SET(dsServ, &set); //ajout de la socket client au tableau de scrutation
  int maxDesc = (sock_list > dsServ ) ? sock_list : dsServ;
  //printf("[Client] : attente de connexion du serveur.\n");
  int incomingConnexions = 0; //nombre de connexions entrantes total
  int incoming = 0; //nombre de connexions entrantes reçues
  char * myColor = (char*)malloc(MAX_LENGTH*sizeof(char));
  struct paquet* incomingConnexionsInfos = (struct paquet*)malloc(allNeighbors * sizeof(struct paquet));
  struct paquet* voisinsAdr = (struct paquet*)malloc(allNeighbors * sizeof(struct paquet));

  struct infosColor* infos = (struct infosColor*)malloc(allNeighbors * sizeof(struct infosColor));

	int toConnectNeighbors = 0;
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
        int res = recvTCP(dsServ, &toConnectNeighbors, sizeof(int));
        if (res == -1 || res == 0) {
            printf("%s[Client] Erreur lors de la reception du nombre de noeuds voisins%s\n", AC_RED, AC_WHITE);
            exit(0);
        }
        printf("[Client] Nombre de voisins à connecter : %i voisins\n", toConnectNeighbors);
        incoming = allNeighbors - toConnectNeighbors;
        printf("[Client] Nombre de connexions en attente : %i voisins\n", incoming);
        //Reception du numéro de noeud
        int nodenumberReception = recvTCP(dsServ, &number, sizeof(number));
        printf("[Client] Numéro de noeud reçu, je suis le noeud numéro %i\n", number);
        color = ((number%6)+1);
        if(color == 1) {
          color = 7;
        }
        color = color + '0';
        //étape 2 : boucle avec for et i<nombreNoeud reception un part un de chaque adresses de noeuds et stockage dans la struct + création socket
        if(toConnectNeighbors != 0){
          //printf("[Client/Reception] Reception et stockage des adresses voisines\n");
          for(int i = 0; i < toConnectNeighbors; i++){
            struct paquet adr;
            int reception = recvTCP(dsServ, &adr, sizeof(adr));
            if(reception == -1 || reception == 0){
              printf("%s[Client] Erreur lors de la reception de l'adresse d'un voisin%s\n", AC_RED, AC_WHITE);
              exit(0);
            }
            voisinsAdr[i].adresse = adr.adresse;
            //printf("[Client] Une adresse voisine est %s:%i.\n", inet_ntoa(voisinsAdr[i].adresse.sin_addr), ntohs(voisinsAdr[i].adresse.sin_port));
          }
          //printf("%s[Client] Attente de l'ordre pour démarrer les connexions%s\n", AC_MAGENTA, AC_WHITE);
          //Reception de l'ordre de démarrer les connexions
          int ordre;
          int ordreReception = recvTCP(dsServ, &ordre, sizeof(ordre));
          if(ordreReception == -1 || ordreReception == 0){
            printf("%s[Client] Erreur lors de la reception de l'ordre de connexion%s\n", AC_RED, AC_WHITE);
            exit(0);
          }

          //printf("Couleur : %i", color);
          if(nodenumberReception == -1 || nodenumberReception == 0){
            printf("%s[Client] Erreur lors de la reception du numéro d'identification%s\n", AC_RED, AC_WHITE);
            exit(0);
          }

          sleep(2);

          //printf("\x1B[3%cm[Client] %i) Ordre de connexion reçu, je démarre les connexions%s\n", color, number, AC_WHITE);

          //étape 3 : boucle de connexion
          //printf("\x1B[3%cm[Client/Connexions] Le noeuds %i démarre les connexions aux voisins%s\n",color, number, AC_WHITE);
          for(int j = 0; j < toConnectNeighbors; j++){
            //printf("\x1B[3%cm[Client/Connexions] Tentative de connexion au noeud %i %s\n",color, j, AC_WHITE);
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

            struct paquet msg;
  
            msg.adresse = sock_clt;
            if (sendTCP(dsVoisins, &msg, sizeof(struct paquet)) <= 0){
                printf("%s[Client/Interconnexions] Problème lors de l'envoi de l'adresse d'écoute%s\n", AC_RED, AC_WHITE);
            }
            //printf("%s[Client n°%i] Envoi de l'adresse d'écoute au voisin réussi%s\n", AC_GREEN, number, AC_NORMAL);
            //printf("\x1B[3%cm[Client/Connexion] Connexion au voisin %i (%s:%i) réussie%s\n", color, j, inet_ntoa(voisinsAdr[j].adresse.sin_addr), ntohs(voisinsAdr[j].adresse.sin_port), AC_WHITE);
          }
          //printf("\e[0;100m\x1B[3%cm[Client/Connexion] Noeud %i, toutes les connexions sont réussies\e[0m%s\n", color, number, AC_WHITE);
          //break;
        }

        /*         printf("Fermeture de la socket serveur\n");
        FD_CLR(dsServ, &set); */
        
      } else {
        // TODO Ajouter un compteur pour incrémenter le nombre de personne reçues et arrêter la boucle quand on a reçu tout le monde et commencer la coloration
        //Acceptation de la connexion d'un autre client
        struct paquet newClient;
        int dsC = accept(ds, (struct sockaddr *)&sock_clt, &size);
        setsockopt(dsC, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        if (recvTCP(dsC, &newClient, sizeof(struct paquet)) <= 0) {
          printf("[Client/Interconnexions] Problème lors de la réception de l'adresse d'un voisin\n");
          exit(0);
        }     
        incomingConnexionsInfos[incomingConnexions] = newClient;
        //char adresse[INET_ADDRSTRLEN];
        //inet_ntop(AF_INET, &incomingConnexionsInfos[incomingConnexions].adresse.sin_addr, adresse, INET_ADDRSTRLEN);
        //int port = htons(incomingConnexionsInfos[incomingConnexions].adresse.sin_port);
      
        //printf("[Client/Interconnexions] %i) %s:%i\n", incomingConnexions, adresse, port);
        
        incomingConnexionsInfos[incomingConnexions].socket = dsC;
        incomingConnexions++;
        
        // FD_SET(dsC, &set);
        // if(maxDesc < dsC) maxDesc = dsC;
      }

    }
      sleep(1);
      if(incomingConnexions == incoming){
				break;
      }
  }
	int cpt = 0;
	for (size_t i = 0; i < toConnectNeighbors ; i++)
	{
		if(&voisinsAdr[i] != NULL) {
			struct sockaddr_in sock_voisin = voisinsAdr[i].adresse;
			int socket = voisinsAdr[i].socket;
			struct infosColor newInfos;
			newInfos.adresse = sock_voisin;
			newInfos.sendColor = (char*)malloc(MAX_LENGTH*sizeof(char));
			newInfos.receiveColor = (char*)malloc(MAX_LENGTH*sizeof(char));
			newInfos.socket = socket;
			newInfos.state = 1;
			infos[cpt] = newInfos;
			cpt++;
		}
		else {
			break;
		}
	}
	for (size_t i = 0; i < incoming; i++) {
		struct sockaddr_in sock_voisin = incomingConnexionsInfos[i].adresse;
		int socket = incomingConnexionsInfos[i].socket;
		struct infosColor newInfos;
		newInfos.adresse = sock_voisin;
		newInfos.sendColor = (char*)malloc(MAX_LENGTH*sizeof(char));
		newInfos.receiveColor = (char*)malloc(MAX_LENGTH*sizeof(char));
		newInfos.socket = socket;
		newInfos.state = 1;
		infos[cpt] = newInfos;
		cpt++;
	}
	pthread_t threads[2*(toConnectNeighbors + incoming)];

	printf("Début de la coloration %i ..\n", number);
	int check = 1;
	while(check == 1) {  
		myColor = nextBinary(myColor);
		printf("[Client %i] Couleur choisie : %s\n", number, myColor);

		for (size_t i = 0; i < allNeighbors; i++)
		{
			if(infos[i].state == 1){
        printf("%sNODE %i Boucle d'envoi, thread n°%li %s\n", AC_MAGENTA, number, i, AC_NORMAL);
				//sprintf(infos[i].color, "%s", myColor);
				strcpy(infos[i].sendColor, myColor);
        //infos[i].color = myColor;
				if(pthread_create(&threads[i], NULL, envoyerCouleur, &infos[i]) != 0) {
          printf("Erreur lors de la création du thread %li", i);
        }
			}
		}
		for (size_t i = 0; i < allNeighbors; i++)
		{
			if(infos[i].state == 1){
        //struct infosColor newInfo = infos[i];
        printf("%sNODE %i Boucle de réception, thread n°%li %s\n", AC_YELLOW,number, i, AC_NORMAL);
				if(pthread_create(&threads[i+ allNeighbors], NULL, recevoirCouleur, &infos[i]) != 0) {
          printf("Erreur lors de la création du thread %li", i);
        }
        //infos[i] = newInfo;
			}
		}

		for (int i = 0; i < 2*allNeighbors; i++){
			pthread_join(threads[i], NULL);
		}

		int verif = 1;
		for (size_t i = 0; i < allNeighbors; i++)
		{
			if(strcmp(infos[i].receiveColor, myColor) != 0){
				infos[i].state = 0;
			}
      else {
        verif = 0;
      }
		}
		
		if(verif == 1){
			check = 0;
		}

		// print infos array and its content
		printf("NODE %i MYCOLOR : %s\n", number,myColor);
		for (size_t i = 0; i < toConnectNeighbors + incoming; i++)
		{
			printf("infos[%li] : %s : %i\n", i, infos[i].receiveColor, infos[i].state);
		} 
	}
	printf("%s[Client %i] Couleur finale : %s %s\n", AC_RED, number, myColor, AC_NORMAL);

  //printf("[Travail] terminé le client s'arrête\n");
 
  // fermeture socket
  // if(close(ds) == -1) {
  //   printf("[Client] : Problème lors de la fermeture socket\n");
  //   exit(1);
  // }
  //printf("[Client] : Socket fermée !\n");

  return 0;
}
