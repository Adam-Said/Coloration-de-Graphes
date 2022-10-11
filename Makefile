########################################
#~ définitions
########################################

BIN=bin/anneaux  bin/annuaire


# liste des fichiers sources 
SRCS0=anneaux.c
SRCS1=annuaire.c

default: $(BIN)

########################################
#~ regles pour l'executable
########################################

obj/%.o: %.c
	gcc -Wall -Iinclude -c $< -o $@

bin/anneaux: $(SRCS0:%.c=obj/%.o)
	gcc -o $@ $+

bin/annuaire: $(SRCS1:%.c=obj/%.o)
	gcc -o $@ $+



clean:
	rm -f $(BIN) obj/*.o *~
