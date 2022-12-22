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
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>

#define AC_BLACK "\x1b[30m"
#define AC_RED "\x1b[31m"
#define AC_GREEN "\x1b[32m"
#define AC_YELLOW "\x1b[33m"
#define AC_BLUE "\x1b[34m"
#define AC_MAGENTA "\x1b[35m"
#define AC_CYAN "\x1b[36m"
#define AC_WHITE "\x1b[37m"
#define AC_NORMAL "\x1b[m"

#define MAX_LENGTH 10000
#define BINARY_LENGTH 30

struct nodeInfos {
    int socket;
    struct sockaddr_in address;
	int receiveColor;
	int sendColor;
    int state;
	int order;
};

int myPow(int x, int y) {
    int result = 1;
    for (int i = 0; i < y; i++) {
        result *= x;
    }
    return result;
}

int* generatePowerOfTwo(int* tab, int length){
    //generation des puissances de 2
    for(int i = 0; i < length; i++){
        tab[i] = myPow(2,i);
    }
    return tab;
}

int getBinaryNumber(char* str){
    int binaryNumber = 0;
    int* binaryColor = (int*)malloc(BINARY_LENGTH*sizeof(int));
    for (int i = 0; i < BINARY_LENGTH; i++){
      binaryColor[i] = 0;
    }
    binaryColor = generatePowerOfTwo(binaryColor, BINARY_LENGTH);

    for(int i = 0; i < strlen(str); i++){
        if(str[i] == '1'){
            binaryNumber += binaryColor[i];
        }
    }
    return binaryNumber;
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
	struct timespec finish;
	srand(clock_gettime(CLOCK_REALTIME, &finish));
	int rand_num = finish.tv_nsec % 2;
	char * num = int_to_string(rand_num);// Allouer de l'espace pour la nouvelle chaîne de caractères
	int len = strlen(str);
	//printf("Taille de la chaîne d'origine : %i \n", len);
	char *new_str = (char*)malloc((len+2)*sizeof(char));
	//char* new_str = malloc(MAX_LENGTH * sizeof(char)); // Copier la chaîne de caractères d'origine dans la nouvelle chaîne
	strcpy(new_str, str);  // Ajouter le nombre aléatoire à la fin de la nouvelle chaîne
	strcat(new_str, num);
	printf("Nouvelle chaîne : %s \n", new_str);
	// Retourner la nouvelle chaîne de caractères 
	return new_str;
}


void * recevoirCouleur (void * param){
		
	struct nodeInfos * args = (struct nodeInfos *) param;
	int newColor = 0;
	int dsVois = args->socket;

	int res = recvTCP(dsVois, &newColor, sizeof(int));
	if (res == -1 || res == 0) {
		perror("[Client/Thread] Erreur lors de la reception de la couleur\n");
		exit(0);
	}
	else
	{
		printf("[Client/Thread] Couleur reçue : %i\n", newColor);
	}
	
	args->receiveColor = newColor;
	pthread_exit(NULL);
}

void * envoyerCouleur (void * param){ 
	struct nodeInfos * args = (struct nodeInfos *) param;
	int newColor = args->sendColor;
	int ds = args->socket;

	if (sendTCP(ds, &newColor, sizeof(int)) == -1) {
		perror("[Client/Thread] Problème lors de l'envoi de la couleur\n");
	}
	else
	{
		printf("[Client/Thread] Couleur envoyée : %i\n", newColor);
	}
	pthread_exit(NULL);
}



int main(int argc, char *argv[]) {
	if (argc != 5){
		printf("utilisation : client ip_serveur port_serveur ip_client port_client\n");
		exit(0);
	}

	// Création socket d'écoute 
	int option = 1;
	int ds = socket(PF_INET, SOCK_STREAM, 0);
	setsockopt(ds, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	if (ds == -1) {
		perror("[Client] : pb creation socket :\n");
		exit(1);
	}

	struct sockaddr_in sock_clt;
	socklen_t size = sizeof(struct sockaddr_in);
	sock_clt.sin_family = AF_INET;
	sock_clt.sin_addr.s_addr = inet_addr(argv[3]);
	sock_clt.sin_port = htons((short)atoi(argv[4]));
	int res = bind(ds, (struct sockaddr*) &sock_clt, size);

	if (res == -1) {
		perror("[Client] : Problème nommage socket :\n");
		exit(1);
	}

	/* etape 2 : designer la socket du serveur */
	int dsServ = socket(PF_INET, SOCK_STREAM, 0);
	setsockopt(dsServ, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	if (dsServ == -1) {
		perror("Problème creation socket :");
		exit(1);
	}

	struct sockaddr_in sock_srv;
	sock_srv.sin_family = AF_INET;
	sock_srv.sin_addr.s_addr = inet_addr(argv[1]);
	sock_srv.sin_port = htons(atoi(argv[2]));
	socklen_t lgAdr = sizeof(struct sockaddr_in);

	// Connexion au serveur
	int resConnexion = connect(dsServ, (struct sockaddr *)&sock_srv, lgAdr);
	if(resConnexion != 0){
		perror("Erreur lors de la connexion au serveur\n");
		exit(0);
	}

	int sock_list = listen(ds, 1000);
	if (sock_list == -1){
		perror("[Client] : problème lors de la mise en écoute de la socket");
		exit(1);
	}

	struct nodeInfos myInfos;
	myInfos.address = sock_clt;
	if (sendTCP(dsServ, &myInfos, sizeof(struct nodeInfos)) <= 0){
		printf("%s[Client] Problème lors de l'envoi de l'adresse d'écoute%s\n", AC_RED, AC_WHITE);
	}
	printf("%s[Client] Envoi de l'adresse d'écoute réussi%s\n", AC_GREEN, AC_NORMAL);

	int myNumber = 0; //numéro d'identification du client
	fd_set set;
	fd_set settmp;
	FD_ZERO(&set); //initialisation à 0 des booléens de scrutation
	FD_SET(ds, &set); //ajout de la socket client au tableau de scrutation
	FD_SET(dsServ, &set); //ajout de la socket client au tableau de scrutation
	int maxDesc = (sock_list > dsServ ) ? sock_list : dsServ;

	int allNeighbors = 0; // Nombre de voisins total
	int connOut = 0; // Nombre de connexions sortantes
	int connIn = 0; // Nombre de connexions entrantes
	int receivedConn = 0; // Compteur de connexions entrantes reçues
	char * myColor = (char*)malloc(MAX_LENGTH*sizeof(char)); // Couleur du client

	struct nodeInfos* connInTab = (struct nodeInfos*)malloc(allNeighbors * sizeof(struct nodeInfos));
	struct nodeInfos* connOutTab = (struct nodeInfos*)malloc(allNeighbors * sizeof(struct nodeInfos));
	struct nodeInfos* allNodesTab = (struct nodeInfos*)malloc(allNeighbors * sizeof(struct nodeInfos));

	// Début multiplexage
	while(1){
		settmp = set;
		// Attente d'un évènement
		if (select(maxDesc+1, &settmp, NULL, NULL, NULL) == -1) {
			printf("%s[CLIENT] Problème lors du select%s\n", AC_RED, AC_WHITE);
		}
		for(int df = 2; df <= maxDesc; df++){
			if(!FD_ISSET(df, &settmp)) continue;

			// Evènement provenant du serveur
			if(df == dsServ){
				// Réception du nombre de connexions totales
				int res = recvTCP(dsServ, &allNeighbors, sizeof(int));
				if (res == -1 || res == 0) {
					printf("%s[Client] Erreur lors de la reception du nombre de connexions totales %s\n", AC_RED, AC_WHITE);
					exit(0);
				}
				
				// Réception du nombre de connexions sortantes
				res = recvTCP(dsServ, &connOut, sizeof(int));
				if (res == -1 || res == 0) {
					printf("%s[Client] Erreur lors de la reception du nombre de connexions sortantes %s\n", AC_RED, AC_WHITE);
					exit(0);
				}

				// Calcul du nombre de connexion entrantes
				connIn = allNeighbors - connOut;
				// printf("[Client] Nombre de voisins à connecter : %i voisins\n", connOut);
				// printf("[Client] Nombre de connexions en attente : %i voisins\n", connIn);

				//Reception du numéro de noeud
				res = recvTCP(dsServ, &myNumber, sizeof(myNumber));
				printf("[Client] Numéro de noeud reçu, je suis le noeud numéro %i\n", myNumber);

				// Si nombre de connexion sortantes différents de 0
				if(connOut != 0){
					for(int i = 0; i < connOut; i++){
						struct nodeInfos infosVoisin;
						// Réception de l'adresse d'un voisin
						res = recvTCP(dsServ, &infosVoisin, sizeof(infosVoisin));
						if(res == -1 || res == 0){
							printf("%s[Client] Erreur lors de la reception de l'adresse d'un voisin%s\n", AC_RED, AC_WHITE);
							exit(0);
						}
						// Stockage de l'adresse du voisin
						connOutTab[i].address = infosVoisin.address;
					}

					// Réception de l'ordre de connexion
					int ordre;
					res = recvTCP(dsServ, &ordre, sizeof(int));
					if(res == -1 || res == 0){
						printf("%s[Client] Erreur lors de la reception de l'ordre de connexion%s\n", AC_RED, AC_WHITE);
						exit(0);
					}
					else {
						printf("Démarrage des connexions\n");
					}

					// Connexions aux voisins sortants
					for(int i = 0; i < connOut; i++){
						struct sockaddr_in sock_voisin = connOutTab[i].address;
						socklen_t lgAdr = sizeof(struct sockaddr_in);
						int dsVoisin = socket(PF_INET, SOCK_STREAM, 0);
						setsockopt(dsVoisin, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
						if (dsVoisin == -1){
							printf("%s[Client] Problème lors de la creation de la socket pour la connexion voisine%s\n", AC_RED, AC_WHITE);
							exit(1);
						}
						int co = connect(dsVoisin, (struct sockaddr *)&sock_voisin, lgAdr);
						if(co == -1){
							printf("%s[Client/Connexion] Erreur lors de la connexion au voisin%s\n", AC_RED, AC_WHITE);
							exit(0);
						}
						connOutTab[i].socket = dsVoisin;

						struct nodeInfos msg;
			
						msg.address = sock_clt;
						if (sendTCP(dsVoisin, &msg, sizeof(struct nodeInfos)) <= 0){
							printf("%s[Client/Interconnexions] Problème lors de l'envoi de l'adresse d'écoute%s\n", AC_RED, AC_WHITE);
						}
						//printf("[Client/Connexion] Connexion au voisin %i (%s:%i) réussie\n", i, inet_ntoa(connOutTab[i].address.sin_addr), ntohs(connOutTab[i].address.sin_port));
					}
				}
			}

			// Réception d'un évènement ne provenant pas du serveur mais d'un autre client
			else {
				struct nodeInfos newClient;
				int dsClient = accept(ds, (struct sockaddr *)&sock_clt, &size);
				setsockopt(dsClient, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
				if (recvTCP(dsClient, &newClient, sizeof(struct nodeInfos)) <= 0) {
					printf("[Client/Interconnexions] Problème lors de la réception de l'adresse d'un voisin\n");
					exit(0);
				}
				// Ajout du nouveau voisin au voisins entrants connus
				connInTab[receivedConn] = newClient;
				connInTab[receivedConn].socket = dsClient;
				receivedConn++;
			}
		}

		if(receivedConn == connIn) {
			break;
		}
	}
	int cpt = 0;
	for (size_t i = 0; i < connOut ; i++) {
		if(&connOutTab[i] != NULL) {
			struct sockaddr_in sock_voisin = connOutTab[i].address;
			int socket = connOutTab[i].socket;
			struct nodeInfos newInfos;
			newInfos.address = sock_voisin;
			newInfos.sendColor = 0;
			newInfos.receiveColor = 0;
			newInfos.socket = socket;
			newInfos.state = 1;
			newInfos.order = 0;
			allNodesTab[cpt] = newInfos;
			cpt++;
		}
		else {
			break;
		}
	}

	for (size_t i = 0; i < connIn ; i++) {
		struct sockaddr_in sock_voisin = connInTab[i].address;
		int socket = connInTab[i].socket;
		struct nodeInfos newInfos;
		newInfos.address = sock_voisin;
		newInfos.sendColor = 0;
		newInfos.receiveColor = 0;
		newInfos.socket = socket;
		newInfos.state = 1;
		newInfos.order = 0;
		allNodesTab[cpt] = newInfos;
		cpt++;
	}
	pthread_t threads[2*allNeighbors];

	printf("Début de la coloration %i ..\n", myNumber);
	int check = 1;
	while(check == 1) {  
		myColor = nextBinary(myColor);
		printf("[Client %i] Couleur choisie : %s\n", myNumber, myColor);
		//int binColor = getBinaryNumber("01");
		//printf("RIEN DE SPECIAL");
		// for (size_t i = 0; i < allNeighbors; i++) {
		// 	if(allNodesTab[i].state == 1){
		// 		allNodesTab[i].sendColor = binColor;
		// 		if(pthread_create(&threads[i], NULL, envoyerCouleur, &allNodesTab[i]) != 0) {
		// 			printf("Erreur lors de la création du thread %li", i);
		// 		}
		// 	}
		// }

		// for (size_t i = 0; i < allNeighbors; i++) {
		// 	if(allNodesTab[i].state == 1){
		// 		if(pthread_create(&threads[i+ allNeighbors], NULL, recevoirCouleur, &allNodesTab[i]) != 0) {
		// 			printf("Erreur lors de la création du thread %li", i);
		// 		}
		// 	}
		// }
		// for (int i = 0; i < 2*allNeighbors; i++){
		// 	if(i < allNeighbors){
		// 		if(allNodesTab[i].state == 1) {
		// 			pthread_join(threads[i], NULL);
		// 		}
		// 	}
		// 	else {
		// 		if(allNodesTab[i-allNeighbors].state == 1) {
		// 			pthread_join(threads[i + allNeighbors], NULL);
		// 		}
		// 	}
		// }

		// printf("NODE %i MYCOLOR : %i\n", myNumber, binColor);
		// for (size_t i = 0; i < allNeighbors; i++)
		// {
		// 	printf("infos[%li] : %i : %i\n", i, allNodesTab[i].receiveColor, allNodesTab[i].state);
		// } 

		// int verif = 1;
		// for (size_t i = 0; i < allNeighbors; i++)
		// {
		// 	if(allNodesTab[i].receiveColor == binColor){
		// 		allNodesTab[i].state = 0;
		// 	}
		// 	else {
		// 		verif = 0;
		// 	}
		// }
		
		// if(verif == 1){
		check = 0;
		// }
	}
}