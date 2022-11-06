########################################
#~ définitions
########################################

BIN=bin/client bin/serverMultiplex


# liste des fichiers sources 
SRCS0=client.c
SRCS1=serverMultiplex.c

default: $(BIN)

########################################
#~ regles pour l'executable
########################################

obj/%.o: %.c
	gcc -Wall -Iinclude -c $< -o $@

bin/client: $(SRCS0:%.c=obj/%.o)
	gcc -o $@ $+ -lpthread

bin/serverMultiplex: $(SRCS1:%.c=obj/%.o)
	gcc -o $@ $+ -lpthread


clean:
	(rm -rf bin ; rm -rf obj)
