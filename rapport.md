# Rapport Projet Systèmes et Réseaux
##### Adel BENNOUAR 12003494

## Ajouts au protocole WCP
PAS D'AJOUTS.

## Fonctionnalités mises en oeuvre

### Multithreadage
Le serveur est multithreadé afin d'avoir plusieurs clients en même temps.
La structure `work` a été nécessaire afin d'y faire passer les informations nécessaires à l'échange entre le client et le serveur.

### Fichier de log
Le serveur enregistre les connexions dans un fichier log. Un mutex a été nécessaire pour éviter les effets de compétition entre threads. Chaque ligne du fichier log contient l'heure de connexion, l'IP de connexion, le répertoire de comptines, le numéro de comptine demandé ainsi que son titre.

## Ce qui ne fonctionne pas
Je pense que dans ce code, tout marche comme prévu, mais j'aurais aimé faire plus que ce que j'ai fait. Malheuresement j'ai manqué de temps.

## Ressources externes au cours ?
J'ai eu besoin de consulter des pages de manuel pour faire la fonction `getdate`.

## Aide d'un autre étudiant ?
Je n'ai pas eu d'aide d'un autre étudiant.

## Conclusion

Ce projet était une bonne préparation à la partie réseau du Partiel 2. Néanmoins j'aurais aimé faire plus que ce que j'ai fait.

