all: server
server: servidor.c utils.c
	gcc servidor.c utils.c -o Server -lcommons -lpthread 
