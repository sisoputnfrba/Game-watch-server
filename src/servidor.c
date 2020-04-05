/*
 * servidor.c
 *
 *  Created on: 3 mar. 2019
 *      Author: utnso
 */

#include "servidor.h"

int main(void)
{
	logger = log_create("servidor.log","servidor",true,0);
	log_info(logger,"Log inicializado");
	iniciar_servidor();

	return EXIT_SUCCESS;
}
