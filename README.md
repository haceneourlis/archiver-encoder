
@Description du projet : 

Ce projet se compose de trois dossiers. Le premier dossier comprend deux fichiers, `encode.c` et `decode.c`. Le fichier `encode.c` chiffre les fichiers selon un motif sécurisé, tandis que le fichier `decode.c` décrypte le fichier chiffré. Pour utiliser ces fonctionnalités, veuillez suivre les étapes suivantes :

en bash: 
gcc encodeur.c -o encodeur  

./encodeur [argument]

gcc decodeur.c -o decodeur

./decodeur [argument]

NB : Une image (voir `figure_1`) explique comment les bits sont manipulés, offrant une compréhension rapide de l'algorithme implémenté.

@Nouvelle Fonctionnalité: Filtrage de l'Encodage et du Décodage

Dans le deuxième dossier, deux nouveaux fichiers, `my_sae_encoding.c` et `my_sae_decoding.c`, permettent désormais un encodage et un décodage personnalisés directement à partir de la ligne de commande :

en bash:
---------------------------
./my_sae_encoding << tag
Ce matin il fait beau
tag
# Sortie sur le terminal (stdout)
---------------------------

Cette fonctionnalité est également disponible pour `my_sae_decoding`.


Donc maintenant l'encodage et le décodage peuvent être filtrés grâce aux fichiers du deuxième dossier.

@ Archiveur:

L'archiveur a pour objectif de regrouper tous les fichiers passés en paramètres dans un fichier nommé `my-ball.sh`. pour faire cela il faut éxécuter la commande :
./create-ball-6.sh file1 directoryX ......
Ces fichiers sont ensuite supprimés. Pour restaurer les fichiers initiaux, il suffit d'exécuter la commande :
----------------
./my-ball.sh
----------------

@ Avertissement:
Ce projet est destiné à des fins éducatives et vise à démontrer des techniques innovantes en manipulation de fichiers et en sécurité. Utilisez-le de manière responsable et respectez les considérations éthiques lors de l'application de ces méthodes.

Licence:Licence MIT .
