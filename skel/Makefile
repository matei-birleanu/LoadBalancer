# Copyright 2023 Vasile Alexandru-Gabriel <vasilealexandru37@gmail.com>
 
# compiler setup
CC=gcc
CFLAGS=-Wall -Wextra -std=c99 
 
# define targets
TARGETS=tema2
 
build:
	$(CC) $(CFLAGS) *.c *.h -g -o tema2
 
pack:
	zip -FSr 314CA_VasileAlexandruGabriel_Tema2.zip README.md Makefile *.c *.h
 
clean:
	rm -f $(TARGETS)
 
.PHONY: pack
