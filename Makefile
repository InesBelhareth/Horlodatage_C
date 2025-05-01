# Variables
CC = gcc
CFLAGS = -Wall -pthread
TARGETS = horloge_scalaire horloge_vectorielle horloge_matricielle ricart_agrawala

# Cible par défaut
all: $(TARGETS)

# Compilation des exécutables
horloge_scalaire: horloge_scalaire.c
	$(CC) $(CFLAGS) -o $@ $<

horloge_vectorielle: horloge_vectorielle.c
	$(CC) $(CFLAGS) -o $@ $<

horloge_matricielle: horloge_matricielle.c
	$(CC) $(CFLAGS) -o $@ $<

ricart_agrawala: ricart_agrawala.c
	$(CC) $(CFLAGS) -o $@ $<

# Nettoyage des fichiers générés
clean:
	rm -f $(TARGETS)

.PHONY: all clean
