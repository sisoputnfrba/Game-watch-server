/*
 * conexiones.c
 *
 *  Created on: 3 mar. 2019
 *      Author: utnso
 */

#include"utils.h"

void iniciar_servidor(void)
{
	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(IP, PUERTO, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);
    log_info(logger,"Servidor levantdo.");
    while(1)
    	esperar_cliente(socket_servidor);
}

void esperar_cliente(int socket_servidor)
{
	struct sockaddr_in dir_cliente;

	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	pthread_create(&thread,NULL,(void*)serve_client,&socket_cliente);
	pthread_detach(thread);

}

void serve_client(int* socket)
{
	int cod_op;
	if(recv(*socket, &cod_op, sizeof(int), MSG_WAITALL) == -1)
		cod_op = -1;
	log_info(logger,"Se conecto un cliente con socket: %d",*socket);
	process_request(cod_op, *socket);
}

void process_request(int cod_op, int cliente_fd) {
	int size;
	void* msg;
	log_info(logger,"Codigo de operacion %d",cod_op);
	switch (cod_op) {
		case MENSAJE:
			msg = recibir_mensaje(cliente_fd, &size);
			devolver_mensaje(msg, size, cliente_fd);
			free(msg);
			break;
		case 0:
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
	}
}

void* recibir_mensaje(int socket_cliente, int* size)
{
	void * buffer;
	log_info(logger,"Recibiendo mensaje.");
	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	log_info(logger,"Tamaño de paquete recibido: %d",*size);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);
	log_info(logger,"Mensaje recibido.");
	return buffer;
}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	log_info(logger,"Iniciando serializando del paquete.");
	void * magic = malloc(bytes);
	int desplazamiento = 0;
	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	log_info(logger,"Codigo de operacion: %d",paquete->codigo_operacion);
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	log_info(logger,"Tamaño del buffer del paquete: %d",paquete->buffer->size);
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;
	log_info(logger,"Finalizo la serializacion del paquete.");
	return magic;
}

void devolver_mensaje(void* payload, int size, int socket_cliente)
{
	log_info(logger,"Devolviendo mensaje");
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = size;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, payload, paquete->buffer->size);
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);
	send(socket_cliente, a_enviar, bytes, 0);
	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	log_info(logger,"Mensaje devuelto");
}
