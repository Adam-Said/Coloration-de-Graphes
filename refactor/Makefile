########################################
#~ définitions
########################################

BIN=bin/client bin/server


# liste des fichiers sources 
SRCS0=client.c
SRCS1=server.c

default: $(BIN)

########################################
#~ regles pour l'executable
########################################

obj/%.o: %.c
	gcc -Wall -Iinclude -c $< -o $@

bin/client: $(SRCS0:%.c=obj/%.o)
	gcc -lpthread -o $@ $+ 

bin/server: $(SRCS1:%.c=obj/%.o)
	gcc -lpthread -o $@ $+ 


clean:
	(rm -rf bin ; rm -rf obj)
