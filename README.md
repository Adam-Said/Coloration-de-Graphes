# HAI721-Projet

## Rendu mi-projet
### COSSU Arnaud - SAID Adam

Mise en place d'un réseau de plusieurs clients interconnectés à partir d'une structure de graph.
Il s'agit de créer un serveur afin de récupérer le schéma du réseau et de se placer en tant qu'annuaire afin de fournir les adresses des clients à tous les autres. La structure du graph peut être trouvée à [cette adresse](http://cedric.cnam.fr/~porumbed/graphs/).

### Utilisation

- Extrayez tous les dossiers et fichiers de l'archive
- Ouvrez un terminal dans le dossier courant contenant tous les fichiers du projet

Pour lancer le réseau il est nécessaire d'avoir :
- `Python`
- `Make`
- `gcc`

En suite vous pouvez lancer le script `Python` qui va s'occuper de tout compiler et de lancer le serveur et les clients.
```bash
python3 runner.py
```
ou
```bash
python runner.py
```
A son lancement le script vous demande un fichier à parser, il doit être contenu dans un dossier `Files` dans le dossier courant du projet. Mais trois fichiers de test bien placé sont déjà présents par défaut, "fileTest3.txt", "fileTest10.txt" et "dsjc250.5.txt". Il faut **taper le nom sans les guillemets**. Mais si vous souhaitez en créer un c’est possible mais il faut garder la structure de ce par défaut avec **le nombre de clients dans le nom du fichier**.

Enfin, le programme demande un numéro de port. Vous pouvez mettre celui que vous voulez mais garder à l’esprit que les ports des clients seront celui du serveur plus le numéro du client, il est donc nécessaire que les ports d’après soient égalements disponibles.

Vous pouvez voir alors dans l’ordre et dans le terminal :
- le parsing du fichier
- l’envoi du nombre de voisins à chaque client
- la réception sur chaque client
- l’envoi et réception des adresses
- la connexion des clients à leurs voisins

Une fois terminé, vous pouvez lancer la commande
```bash
python3 killer.py
```
dans le terminal afin de nettoyer le projet et fermer les programmes.
