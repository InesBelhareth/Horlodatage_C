# Implémentation d'Horloges Logiques en C

Ce projet implémente trois types d'horloges logiques pour la synchronisation dans les systèmes distribués :
1. Horloge scalaire (Lamport)
2. Horloge vectorielle
3. Horloge matricielle

De plus, une implémentation de l'algorithme d'exclusion mutuelle de Ricart-Agrawala utilisant des horloges vectorielles est également fournie.

## Compilation

Pour compiler tous les programmes :

```bash
make
```

Cela générera les exécutables suivants :
- `horloge_scalaire`
- `horloge_vectorielle`
- `horloge_matricielle`
- `ricart_agrawala`

## Utilisation

### Configuration

1. Lancez plusieurs instances des programmes sur la même machine, chacune avec un ID différent (0 à 3)
2. Chaque programme communique avec les autres via les ports TCP (8080 + ID du processus)

### Exécution des démonstrations

Ouvrez 4 terminaux et exécutez un type d'horloge dans chaque terminal avec un ID unique :

**Horloge scalaire :**
```bash
./horloge_scalaire
# Saisissez un ID entre 0 et 3
```

**Horloge vectorielle :**
```bash
./horloge_vectorielle
# Saisissez un ID entre 0 et 3
```

**Horloge matricielle :**
```bash
./horloge_matricielle
# Saisissez un ID entre 0 et 3
```

**Algorithme Ricart-Agrawala :**
```bash
./ricart_agrawala
# Saisissez un ID entre 0 et 3
```

### Fonctionnalités

Chaque programme d'horloge offre un menu interactif permettant de :
1. Générer un événement local
2. Envoyer un message à tous les autres processus

La démonstration exécute automatiquement :
- 5 événements locaux
- 1 envoi de messages à tous les autres processus

L'implémentation de Ricart-Agrawala fonctionne en deux phases :
1. **Phase automatique** :
   - Exécution de 3 événements locaux initiaux
   - Simulation automatique avec 3 demandes d'accès à la section critique par processus
   - Affichage en temps réel des mises à jour d'horloge après chaque événement
   - Génération automatique d'un fichier de visualisation HTML à la fin

2. **Phase interactive** (après la simulation automatique) :
   - Demander l'accès à la section critique
   - Afficher l'horloge vectorielle
   - Générer une visualisation HTML de l'exécution

## Description des types d'horloge

### Horloge Scalaire (Lamport)
- Maintient un compteur unique
- Incrémente le compteur à chaque événement local
- Lors de la réception d'un message : prend le maximum entre l'horloge locale et l'horloge reçue, puis incrémente de 1

### Horloge Vectorielle
- Maintient un vecteur d'entiers (un compteur pour chaque processus)
- Incrémente sa propre position à chaque événement local
- Lors de la réception d'un message : prend le maximum pour chaque position entre les vecteurs, puis incrémente sa propre position

### Horloge Matricielle
- Maintient une matrice d'entiers (un vecteur pour chaque processus)
- Incrémente sa propre diagonale à chaque événement local
- Lors de la réception d'un message : fusionne les matrices en prenant le maximum pour chaque position, puis incrémente sa propre diagonale

### Algorithme d'exclusion mutuelle de Ricart-Agrawala
- Utilise des horloges vectorielles pour ordonner les requêtes
- Implémente l'algorithme complet avec requêtes, réponses et libérations
- Exécute une simulation automatique avec plusieurs demandes d'accès à la section critique
- Génère une visualisation HTML avec code couleur pour identifier les différents types d'événements
- Affiche en temps réel les mises à jour d'horloge après chaque événement

## Exemple de sortie d'horloge matricielle

```
[Matriciel 1] Événement local.
[Matriciel 1] Horloge :
1 0 0 0
0 1 0 0
0 0 0 0
0 0 0 0

[Matriciel 1] Message envoyé à 0
[Matriciel 1] Horloge :
1 0 0 0
0 2 0 0
0 0 0 0
0 0 0 0

[Matriciel 1] Message reçu: 1,0,0,0;0,0,0,0;0,0,1,0;0,0,0,0;
[Matriciel 1] Matrice extraite:
1 0 0 0
0 0 0 0
0 0 1 0
0 0 0 0

[Matriciel 1] Mise à jour horloge après réception
[Matriciel 1] Horloge :
1 0 0 0
0 3 0 0
0 0 1 0
0 0 0 0
```

## Visualisation de l'algorithme Ricart-Agrawala

L'implémentation de Ricart-Agrawala génère un fichier HTML pour chaque processus, permettant de visualiser la séquence d'événements et les états des horloges vectorielles. La visualisation utilise un code couleur pour différencier les types d'événements :

- Rouge : Demandes d'accès à la section critique
- Vert : Entrée en section critique
- Bleu : Sortie de section critique
- Orange : Envoi/réception de réponses
- Cyan : Événement local

Le fichier génère automatiquement après la simulation automatique et peut également être généré manuellement via le menu interactif. Le fichier (`ricart_agrawala_process_X.html`) peut être ouvert dans n'importe quel navigateur web. 