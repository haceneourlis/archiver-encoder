**Description du Projet :**

Ce projet se divise en trois dossiers. Le premier contient deux fichiers, `encode.c` et `decode.c`. `encode.c` chiffre les fichiers selon un motif sécurisé, tandis que `decode.c` décrypte les fichiers chiffrés. Pour utiliser ces fonctionnalités, veuillez suivre ces étapes :

En bash :
```bash
gcc encodeur.c -o encodeur
./encodeur [argument]

gcc decodeur.c -o decodeur
./decodeur [argument]
```

NB : Une image (voir `figure_1`) explique comment les bits sont manipulés, offrant une compréhension rapide de l'algorithme implémenté.

**Nouvelle Fonctionnalité : Filtrage de l'Encodage et du Décodage**

Dans le deuxième dossier, deux nouveaux fichiers, `my_sae_encoding.c` et `my_sae_decoding.c`, permettent un encodage et un décodage personnalisés directement depuis la ligne de commande :

En bash :
```bash
./my_sae_encoding << tag
Ce matin il fait beau
tag
# Sortie sur le terminal (stdout)
```

Cette fonctionnalité est également disponible pour `my_sae_decoding`.

Maintenant, l'encodage et le décodage peuvent être filtrés grâce aux fichiers du deuxième dossier.

**Archiveur :**

L'archiveur a pour objectif de regrouper tous les fichiers passés en paramètres dans un fichier nommé `my-ball.sh`. Pour réaliser cela, exécutez la commande :
```bash
./create-ball-6.sh file1 directoryX ......
```

Ces fichiers sont ensuite supprimés. Pour restaurer les fichiers initiaux, exécutez simplement la commande :
```bash
./my-ball.sh
```

**Avertissement :**
Ce projet est destiné à des fins éducatives, visant à démontrer des techniques innovantes en manipulation de fichiers et en sécurité. Utilisez-le de manière responsable et respectez les considérations éthiques lors de l'application de ces méthodes.

**Licence :** Licence MIT.
