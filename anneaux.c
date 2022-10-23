#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#define ADRESSE_ECOUTE 1
#define ATTRIBUTION_NUMERO 2
#define ADRESSE_VOISIN 3
#define ELECTION 4
#define TAILLE_RESEAU 5
#define DESTRUCTION_ANNEAU 6

#define TRUE 1
#define FALSE 0
#define MAX_BUFFER_SIZE 1000

#define MAX_STOCKAGE 100
#define RESETCOLOR "\033[0m"

struct paquet
{
    int requete;
    int information;
    int information2;
    struct sockaddr_in adresse;
};

void errorDisplay()
{
    printf("errno: %d , %s\n", errno, strerror(errno));
}

void contentSeparator()
{
    printf("--------------------------------\n");
}

int sockCreate()
{
    int option = 1;
    int ds = socket(PF_INET, SOCK_STREAM, 0);
    setsockopt(ds, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if (ds == -1)
    {
        perror("Problème creation socket :");
        errorDisplay();
        exit(1);
    }
    printf("Création de la socket réussie \n");
    return ds;
}

struct sockaddr_in nameSocket(char *port, int ds)
{
    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    if (atoi(port) != -1)
    {
        ad.sin_port = htons((short)atoi(port));
    }
    printf("Port: %hu\n", htons(ad.sin_port));
    int res;
    res = bind(ds, (struct sockaddr *)&ad, sizeof(ad));
    if (res == 0)
    {
        printf("Socket nommée avec succès\n");
    }
    else
    {
        printf("Erreur lors du nommage de la socket : %i \n", res);
        errorDisplay();
        exit(1);
    }
    return ad;
}

struct sockaddr_in searchSocket(char *port, char *ip)
{
    struct sockaddr_in sock;
    sock.sin_family = AF_INET;
    int resConvertAddr = inet_pton(AF_INET, ip, &(sock.sin_addr));
    if (resConvertAddr == 1)
    {
        printf("Adresse IP correctement convertie\n");
    }
    else
    {
        printf("Problème lors de la conversion de l'adresse IP\n");
        errorDisplay();
        exit(1);
    }
    sock.sin_port = htons((short)atoi(port));
    return sock;
}

int connectTCP(int ds, struct sockaddr_in *sock)
{
    socklen_t lgAdr = sizeof(struct sockaddr_in);
    int resConnexion = connect(ds, (struct sockaddr *)sock, lgAdr);
    if (resConnexion == 0)
    {
        printf("Connexion réussie\n");
    }
    else
    {
        printf("Connexion impossible\n");
        errorDisplay();
        exit(-1);
    }
    return resConnexion;
}

int listenTCP(int ds, int maxWaiting)
{
    int resListen = listen(ds, maxWaiting);
    if (resListen == -1)
    {
        printf("Problème lors de l'écoute\n");
        errorDisplay();
        exit(1);
    }
    else
    {
        printf("En écoute\n");
    }
    return resListen;
}

int sendTCP(int sock, void *msg, int sizeMsg)
{
    int res;
    int sent = 0;
    while (sent < sizeMsg)
    {
        res = send(sock, msg + sent, sizeMsg - sent, 0);
        sent += res;
        if (res == -1)
        {
            printf("Problème lors de l'envoi du message\n");
            errorDisplay();
            return -1;
        }
    }
    return sent;
}

int recvTCP(int sock, void *msg, int sizeMsg)
{
    int res;
    int received = 0;
    while (received < sizeMsg)
    {
        res = recv(sock, msg + received, sizeMsg - received, 0);
        received += res;
        if (res == -1)
        {
            printf("Problème lors de la réception du message\n");
            return -1;
        }
        else if (res == 0)
        {
            return 0;
        }
    }
    return received;
}

int recv2TCP(int sock, void *msg, int sizeMsg)
{
    int taille;
    recvTCP(sock, &taille, sizeof(int));
    if (taille > sizeMsg)
    {
        printf("Problème, buffer trop petit! (taille attendue %i, taille réelle %i)\n", taille, sizeMsg);
        return -1;
    }
    return recvTCP(sock, msg, taille);
}

int send2TCP(int sock, void *msg, int sizeMsg)
{
    sendTCP(sock, &sizeMsg, sizeof(int));
    return sendTCP(sock, msg, sizeMsg);
}

char getColorFromNum(int num)
{
    return (((num % 6) + 1) + '0');
}

void printInColor(char msg[], int num)
{
    printf("\x1B[3%cm[SA%i]: %s%s\n", getColorFromNum(num), num, msg, RESETCOLOR);
}

int stockerMessage(struct paquet stockage[], int *indice, struct paquet msg, int numeroSousAnneau)
{
    if ((*indice) == MAX_STOCKAGE)
    {
        printInColor("Problème, dépassement du stockage maximum de messages", numeroSousAnneau);
        return -1;
    }
    stockage[(*indice)] = msg;
    (*indice)++;
    return 0;
}

int sendMessages(struct paquet stockage[], int *ind, int desc, int numSousAnneau)
{
    if ((*ind) == 0)
    {
        return 1;
    }
    printInColor("Envoi des messages", numSousAnneau);
    if (send2TCP(desc, ind, sizeof(int)) <= 0)
    {
        printInColor("Problème lors de l'envoi du nombre de message stockés", numSousAnneau);
        return -1;
    }
    for (int i = 0; i < (*ind); i++)
    {
        printInColor("Envoi d'un message", numSousAnneau);
        if (send2TCP(desc, &stockage[i], sizeof(struct paquet)) <= 0)
        {
            printInColor("Problème lors de l'envoi d'un message", numSousAnneau);
            return -1;
        }
    }
    (*ind) = 0;
    printInColor("Stockage de message vidé", numSousAnneau);
    return 0;
}

int startElection(int numSousAnneau, struct paquet stockage[], int *ind)
{
    struct paquet msg;
    msg.requete = ELECTION;
    msg.information = numSousAnneau;
    msg.information2 = 1;
    stockerMessage(stockage, ind, msg, numSousAnneau);
    return 0;
}

void autoDestructAnneau(int dsNextAnneau, int dsPrecAnneau, fd_set *set)
{
    if (dsNextAnneau != -1 && close(dsNextAnneau) == -1)
    {
        printf("Problème lors de la fermeture du descripteur\n");
    }
    if (close(dsPrecAnneau) == -1)
    {
        printf("Problème lors de la fermeture du descripteur\n");
    }
    FD_CLR(dsNextAnneau, set);
    FD_CLR(dsPrecAnneau, set);
    printf("Auto-destruction de l'anneau...\n");
    exit(0);
}

int receveMessages(int dsPrecAnneau, int dsNextAnneau, int numSousAnneau, int *chief, int *tailleReseau, struct paquet stockage[], int *ind)
{
    struct paquet msg;
    int nbReception;
    int numeroProvenance;
    int calculDeTaille;
    char str[100];

    if (recv2TCP(dsPrecAnneau, &nbReception, sizeof(int)) <= 0)
    {
        printInColor("Problème lors de la réception du nombre de messages à recevoir", numSousAnneau);
        return -1;
    }
    for (int i = 0; i < nbReception; i++)
    {
        if (recv2TCP(dsPrecAnneau, &msg, sizeof(struct paquet)) <= 0)
        {
            printInColor("Problème lors de la réception d'un message du sous-anneau précédent", numSousAnneau);
            return -1;
        }
        switch (msg.requete)
        {
        case ELECTION:
            numeroProvenance = msg.information;
            calculDeTaille = msg.information2;
            sprintf(str, "J'ai reçu un message d'élection du P°%i, le calcul de taille en cours est %i", numeroProvenance, calculDeTaille);
            printInColor(str, numSousAnneau);
            if (numeroProvenance < numSousAnneau)
            {
                printInColor("Je suis un meilleur candidat, je ne renvoie pas", numSousAnneau);
            }
            else if (numeroProvenance == numSousAnneau)
            {
                printInColor("J'ai reçu mon propre message, je suis donc l'élu, je renvoie la taille du réseau à tous", numSousAnneau);
                (*chief) = TRUE;
                (*tailleReseau) = calculDeTaille;
                msg.information = calculDeTaille;
                msg.requete = TAILLE_RESEAU;
                stockerMessage(stockage, ind, msg, numSousAnneau);
            }
            else
            {
                printInColor("C'est un meilleur candidat que moi, je renvoie", numSousAnneau);
                msg.information2++;
                stockerMessage(stockage, ind, msg, numSousAnneau);
            }
            break;
        case TAILLE_RESEAU:
            if ((*chief))
            {
                printInColor("Le message de taille du reseau a bien fait le tour de l'anneau", numSousAnneau);
            }
            else
            {
                (*tailleReseau) = msg.information;
                sprintf(str, "La taille du reseau est: %i", (*tailleReseau));
                printInColor(str, numSousAnneau);
                stockerMessage(stockage, ind, msg, numSousAnneau);
            }
            break;
        default:
            printInColor("Problème: requête non reconnue", numSousAnneau);
            break;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("utilisation : %s ip_serveur port_serveur port_client\n", argv[0]);
        exit(0);
    }

    char str[100];

    int dsServeur = sockCreate();
    struct sockaddr_in sockServ = searchSocket(argv[2], argv[1]);
    connectTCP(dsServeur, &sockServ);

    int dsPrecAnneau = sockCreate();
    printf("dsPrecAnneau: %i\n", dsPrecAnneau);
    struct sockaddr_in sockPrec = nameSocket(argv[3], dsPrecAnneau);
    int nbMaxAttente = 10;
    listenTCP(dsPrecAnneau, nbMaxAttente);

    struct paquet msg;
    msg.requete = ADRESSE_ECOUTE;
    msg.adresse = sockPrec;
    if (send2TCP(dsServeur, &msg, sizeof(struct paquet)) <= 0)
    {
        printf("Problème lors de l'envoi de l'adresse d'écoute\n");
    }

    if (recv2TCP(dsServeur, &msg, sizeof(struct paquet)) <= 0)
    {
        printf("Probleme lors de la réception du numéro attribué\n");
        exit(0);
    }
    int numSousAnneau = msg.information;
    printInColor("J'ai reçu mon numéro de sous anneau", numSousAnneau);

    fd_set set, settmp;
    FD_ZERO(&set);
    FD_SET(dsServeur, &set);
    FD_SET(dsPrecAnneau, &set);
    struct sockaddr_in sockNext;
    socklen_t lgAdr;
    int maxDescripteur = dsServeur;
    if (dsPrecAnneau > dsServeur)
        maxDescripteur = dsPrecAnneau;
    int dsNextAnneau = -1;

    int connecteAuPrecedent = FALSE;
    int tailleReseau = -1;
    int chief = FALSE;

    struct paquet stockage[MAX_STOCKAGE];
    int indiceStockage = 0;

    while (TRUE)
    {
        if (indiceStockage > 0 && dsNextAnneau != -1)
        {
            if (sendMessages(stockage, &indiceStockage, dsNextAnneau, numSousAnneau) == -1)
            {
                autoDestructAnneau(dsNextAnneau, dsPrecAnneau, &set);
            }
        }

        settmp = set;
        if (select(maxDescripteur + 1, &settmp, NULL, NULL, NULL) == -1)
        {
            printInColor("Problème lors du select", numSousAnneau);
            errorDisplay();
            continue;
        }

        for (int df = 2; df <= maxDescripteur; df++)
        {
            if (!FD_ISSET(df, &settmp))
            {
                continue;
            }

            if (df == dsPrecAnneau)
            {
                if (connecteAuPrecedent)
                {
                    if (receveMessages(dsPrecAnneau, dsNextAnneau, numSousAnneau, &chief, &tailleReseau, stockage, &indiceStockage) == -1)
                    {
                        autoDestructAnneau(dsNextAnneau, dsPrecAnneau, &set);
                    }
                }
                else
                {
                    dsPrecAnneau = accept(dsPrecAnneau, (struct sockaddr *)&sockPrec, &lgAdr);
                    char adresse[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &sockPrec.sin_addr, adresse, INET_ADDRSTRLEN);
                    sprintf(str, "Connecté à l'anneau précédent: %s", adresse);
                    printInColor(str, numSousAnneau);
                    FD_SET(dsPrecAnneau, &set);
                    if (maxDescripteur < dsPrecAnneau)
                        maxDescripteur = dsPrecAnneau;
                    connecteAuPrecedent = TRUE;
                    continue;
                }
            }

            if (df == dsServeur)
            {
                struct paquet msg;
                if (recv2TCP(dsServeur, &msg, sizeof(struct paquet)) <= 0)
                {
                    printInColor("Problème lors de la réception d'un message du serveur", numSousAnneau);
                    exit(1);
                }
                printInColor("Deconnexion du serveur", numSousAnneau);
                if (close(dsServeur) == -1)
                {
                    printInColor("Problème lors de la fermeture du descripteur", numSousAnneau);
                    errorDisplay();
                }
                FD_CLR(dsServeur, &set);
                switch (msg.requete)
                {
                case ADRESSE_VOISIN:
                    sockNext = msg.adresse;
                    char adresse[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &sockNext.sin_addr, adresse, INET_ADDRSTRLEN);
                    int port = htons(sockNext.sin_port);
                    sprintf(str, "Tentative de se connecter à l'anneau suivant: %s:%i\n", adresse, port);
                    printInColor(str, numSousAnneau);
                    dsNextAnneau = sockCreate();
                    connectTCP(dsNextAnneau, &sockNext);
                    printInColor("Connexion au prochain sous-anneau réussie", numSousAnneau);
                    startElection(numSousAnneau, stockage, &indiceStockage);
                    break;
                default:
                    printInColor("Problème: requête non reconnue", numSousAnneau);
                    break;
                }
            }
        }
    }
}