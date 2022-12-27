# HAI721-Projet

## COSSU Arnaud - SAID Adam

Mise en place d'un réseau de plusieurs clients interconnectés à partir d'une structure de graph.
Il s'agit de créer un serveur afin de récupérer le schéma du réseau et de se placer en tant qu'annuaire afin de fournir les adresses des clients à tous les autres. La structure du graph peut être trouvée à [cette adresse](http://cedric.cnam.fr/~porumbed/graphs/). Une fois les noeuds connectés, un algorithme distribué de coloration de graphe va permettre d'attribuer à chaque noeuds une couleur différente de celles de ses voisins en essayant d'utiliser un nombre minimal de couleur.

## Utilisation

### Démarrage rapide

- Extrayez tous les dossiers et fichiers de l'archive
- Ouvrez un terminal dans le dossier courant contenant tous les fichiers du projet

Pour lancer le réseau il est nécessaire d'avoir :
- `Python3`
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
A son lancement le script vous demande un fichier à parser, il doit être contenu dans le dossier `Files` dans le dossier courant du projet. Mais trois fichiers de test bien formés sont déjà présents par défaut, "fileTest10.txt", "dsjc250.5.txt" et "dsjc1000.1.txt". Il faut **taper le nom sans les guillemets**. Mais si vous souhaitez en créer un c’est possible mais il faut garder la structure de ce par défaut.

En suite, le programme demande un numéro de port. Vous pouvez mettre celui que vous voulez mais garder à l’esprit que les ports des clients seront celui du serveur plus le numéro du client, il est donc nécessaire que les ports d’après soient égalements disponibles.

En dernier étape le script demande si vous souhaitez activer le mode verbeux ou non (y/n) afin d'avoir une trace de l'exécution ou non.

Vous pouvez voir alors dans l’ordre et dans le terminal :
- le parsing du fichier
- l’envoi du nombre de voisins à chaque client
- la réception sur chaque client
- l’envoi et réception des adresses
- la connexion des clients à leurs voisins
- le démarrage de la coloration
- le choix d'une nouvelle couleur et l'envoi aux autres clients
- la deuxième phase de coloration avec :
- l'attente de réception de couleur
- le choix d'une couleur
- le renvoi de la nouvelle couleur aux voisins moins prioritaires
- l'envoi au serveur de la couleur finale

Une fois l'exécution terminé vous pouvez voir dans le terminal qu'un fichier `SGV` a été généré avec un aperçu graphique du graph, comme seulement 20 couleurs sont disponibles il est possible que 2 noeuds aient la même couleur alors qu'en réalité ce n'est pas le cas.

Une fois terminé, vous pouvez lancer la commande
```bash
python3 killer.py
```
dans le terminal afin de nettoyer le projet et fermer les programmes.

### Uilisation avancée

Nous avons créé d'autres scripts `Python` pour une utilisation plus poussée.
Il y'a un script `serverRunner.py` qui permet de compiler et lancer uniquement le serveur.
Il y'a le script `clientRunner.py` qui sert à lancer un nombre x de clients fournis en paramètre. Ce qui permet d'en lancer une partie sur une machine et une autre sur une seconde machine.
Il y'a également le script `multiRun.py` pour lancer plusieurs fois le serveur et les clients afin d'avoir plusieurs résultats de coloration.
Et le script `color.py` qui permet de générer le `SVG` à partir du fichier texte généré par le serveur.

Des fichiers de lancements du script principal `runner.py` ont été fait afin d'accélérer les tests et ne pas avoir à entrer tous les paramètres à chaque fois. Il s'agit de `param10.txt` et `param250.txt` qui permettent respectivement de lancer le fichier de test de taille 10 en mode verbose et le fichier de test 250 sans verbose.

Pour s'en servir il suffit de faire :
```bash
python3 runner < param10.txt
```

