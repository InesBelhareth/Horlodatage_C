# Variables
CC = gcc
CFLAGS = -Wall -std=c99
LDFLAGS = 
SRC_FILES = serveur.c horloge_scalaire.c horloge_matricielle.c horloge_vectorielle.c
EXEC_FILES = serveur horloge_scalaire horloge_matricielle horloge_vectorielle

# Cible par défaut
all: $(EXEC_FILES)

# Compilation des exécutables
serveur: serveur.c
	$(CC) $(CFLAGS) serveur.c -o serveur

horloge_scalaire: horloge_scalaire.c
	$(CC) $(CFLAGS) horloge_scalaire.c -o horloge_scalaire

horloge_matricielle: horloge_matricielle.c
	$(CC) $(CFLAGS) horloge_matricielle.c -o horloge_matricielle

horloge_vectorielle: horloge_vectorielle.c
	$(CC) $(CFLAGS) horloge_vectorielle.c -o horloge_vectorielle

# Nettoyage des fichiers générés
clean:
	rm -f $(EXEC_FILES)

.PHONY: all clean
