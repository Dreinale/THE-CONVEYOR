# THE CONVEYOR - Système de Tri Automatisé

Ce système automatise le processus de tri sur une bande transporteuse en dirigeant les colis en fonction des tags NFC.

Bonne chance à tous ceux qui utilisent le module M5Stack n'oubliez pas de bien installer M5Burner & les librairies affiliées pour détecter le module sur votre pc (com 4 la plupart du temps).

## Description

Le système de convoyeur s'intègre avec le système de gestion d'entrepôt (WMS) de Dolibarr pour acheminer automatiquement les colis en fonction de leurs références produits stockées sur les tags NFC. Il dirige les colis vers l'un des trois entrepôts de stockage tout en mettant à jour les mouvements de stock et le suivi en temps réel.

## Processus

1. La bande transporteuse démarre et avance le colis.
2. Un capteur (par exemple, infrarouge) détecte un colis entrant.
3. La bande transporteuse s'arrête.
4. Si le colis n'a pas de tag NFC, un servo moteur le dirige vers le stockage d'erreur "B".
5. Si un UID NFC est détecté, la bande s'arrête, et le servo moteur ajuste sa direction en fonction de l'UID, soit vers "C" ou "D".
6. La bande redémarre, acheminant le colis vers son entrepôt désigné.

## Fonctionnalités

- **Lecteur NFC** : Lit la référence produit des tags NFC.
- **Intégration avec Dolibarr WMS** : Interroge le WMS pour les détails et la destination du produit, et met à jour les mouvements de stock.
- **Tri Automatique** : Dirige les colis vers l'un des trois entrepôts en fonction des données NFC.
- **Gestion des Erreurs** : Dirige les colis sans NFC vers une zone de stockage d'erreur désignée.
- **Récupération Rapide** : Restaure rapidement les opérations en cas d'interruptions comme des pannes de courant, des arrêts manuels ou des pertes de connexion.

## Prérequis

- ArduinoIDE
- Dolibarr WMS avec module de stock (communication via son API REST).
- Tags NFC contenant les références produits.
- Trois conteneurs désignés A, B et C.

## Installation & Configuration

1. Cloner ce dépôt.
2. Connecter les composants matériels comme détaillé dans la documentation du contrôleur.
3. Configurer la connexion API à Dolibarr WMS.
4. Déployer le firmware sur le contrôleur.
