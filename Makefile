########################################
#~ définitions
########################################

BIN=bin/client  bin/server bin/serverMultiplex


# liste des fichiers sources 
SRCS0=client.c
SRCS1=serverMultiplex.c
SRCS2=server.c

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

bin/server: $(SRCS2:%.c=obj/%.o)
	gcc -o $@ $+ -lpthread



clean:
	rm -f $(BIN) obj/*.o *~
