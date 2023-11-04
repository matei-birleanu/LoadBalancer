/* Copyright 2023 <> */
#include <stdlib.h>
#include <string.h>

#include "server.h"

server_memory *init_server_memory()
{
	server_memory *server;
	server = malloc(sizeof(server));
	/*server->rep_id = -1;
	server->serv_id = -1;*/
	server->hashtable = ht_create(10, hash_function_string, compare_function_strings,key_val_free_function);
	return server;
}

void server_store(server_memory *server, char *key, char *value) {
	ht_put(server->hashtable, key, sizeof(char), value, sizeof(char));
}

char *server_retrieve(server_memory *server, char *key) {
	return (ht_get(server->hashtable, key));
}

void server_remove(server_memory *server, char *key) {
	ht_remove_entry(server->hashtable, key);
}

void free_server_memory(server_memory *server) {
	ht_free(server->hashtable);
}
