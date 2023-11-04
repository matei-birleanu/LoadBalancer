/* Copyright 2023 <> */
#include <stdlib.h>
#include <string.h>
#define HMAX 5
#define NREP 3
#define AD 100000
#include "load_balancer.h"

struct load_balancer {
    hashtable_t *servers;
    int *hash_ring;
};

unsigned int hash_function_servers(void *a) {
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

unsigned int hash_function_key(void *a) {
    unsigned char *puchar_a = (unsigned char *)a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}

load_balancer *init_load_balancer() {
    load_balancer *balans;
    balans = malloc(sizeof(balans));
    balans->servers = ht_create(NREP * HMAX, hash_function_int, compare_function_ints,key_val_free_function);
    balans->hash_ring = malloc(HMAX * NREP);
    return balans;
}

int binary_search(int *v, int nr,  int val){
    if(val < hash_function_servers(&v[0]))
        return 0;
    if(val > hash_function_servers(&v[nr - 1]))
        return nr;    
    for(int i = 0; i < nr; i++){
        int a = hash_function_servers(&v[i]);
        int b = hash_function_servers(&v[i+1]);
        if(val> a && val < b)
            return i+1;   
    }
    return 0;

}

void loader_add_server(load_balancer *main, int server_id) {
    server_memory *server = init_server_memory();
    int nr = ht_get_size(main->servers);
    int rep1, rep2;
    rep1 = server_id + AD;
    rep2 = rep1 + AD;
    int hash = hash_function_servers(&server_id);///main->servers->hash_function(&server_id);
    int hash1 = hash_function_servers(&rep1);///main->servers->hash_function(&rep1);
    int hash2 = hash_function_servers(&rep2);///main->servers->hash_function(&rep2);
    int st = 0;
    int dr = nr - 1, mijloc;
    if(nr == 0) {
        main->hash_ring[0] = server_id;
        main->hash_ring[1] = rep1;
        main->hash_ring[2] = rep2;
    }
    else{
        int poz = binary_search(main->hash_ring, nr, hash);
        for(int i = nr - 1; i >= poz; i--)
            main->hash_ring[i + 1] = main->hash_ring[i];
        main->hash_ring[poz] = server_id;
        nr++;
        poz = binary_search(main->hash_ring, nr, hash1);
        for(int i = nr - 1; i >= poz; i--)
            main->hash_ring[i + 1] = main->hash_ring[i];
        main->hash_ring[poz] = rep1;
        poz = binary_search(main->hash_ring, nr, hash2);
        for(int i = nr - 1; i >= poz; i--)
            main->hash_ring[i + 1] = main->hash_ring[i];
        main->hash_ring[poz] = rep2;
    }
    ///cautare binara pe hashring
    ///lepun in hasg_ring in ordinee
    ht_put_modified(main->servers, &server_id, sizeof(int), server, sizeof(server_memory));//+replici
    ht_put_modified(main->servers, &rep1, sizeof(int), server, sizeof(server_memory));
    ht_put_modified(main->servers, &rep2, sizeof(int), server, sizeof(server_memory));
    /* TODO 2 */
}

linked_list_t *extract_objects(hashtable_t *server){
    int nr = ht_get_size(server);
    linked_list_t *lista;
    lista = ll_create(sizeof(info_t));
    linked_list_t **buckets = server->buckets;
    for(unsigned int i = 0; i < server->size; i++){
        ll_node_t *head;
        head = buckets[i]->head;
        ll_add_nth_node(lista, lista->size, head->data);
        head = head->next;
        while(head) {
            ll_add_nth_node(lista, lista->size, head->data);
            head = head->next;
        }
    }
    return lista;
}
void loader_remove_server(load_balancer *main, int server_id) {
    linked_list_t *elemente;
    server_memory * deleted_server = ht_get(main->servers, &server_id);
    elemente = extract_objects(deleted_server->hashtable);
    ll_node_t *head = elemente->head;
    int nr = ht_get_size(main->servers);
    int poz = binary_search(main->hash_ring, nr, hash_function_servers(&server_id));
    int new_server_id = main->hash_ring[poz + 1];
    server_memory *next_server = ht_get(main->servers, &new_server_id);
    while(head) {
        char *key = ((info_t *)head->data)->key;
        char * value = ((info_t *)head->data)->value;
        server_store(next_server, key, value);
        head = head->next;
    }
    free_server_memory(deleted_server);
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id) {
    int hash = hash_function_key(key);
    int nr = ht_get_size(main->servers);
    int poz = binary_search(main->hash_ring,nr , hash);
    *server_id = main->hash_ring[poz];
    server_memory *server = malloc(sizeof(server_memory));
    server->hashtable = ht_get(main->servers, &server_id);
    server_store(server, key, value);
    free(server);
    /* TODO 4 */
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id) {
    server_memory *server = malloc(sizeof(server_memory));
    int nr = ht_get_size(main->servers);
    int hash = hash_function_key(key);
    int poz = binary_search(main->hash_ring, nr, hash);
    *server_id = main->hash_ring[poz];
    server->hashtable = ht_get(main->servers, server_id);
    if(ht_has_key(server->hashtable, key) == 0)
        return NULL;
    return ht_get(server->hashtable, key);    
}

void free_load_balancer(load_balancer *main) {
   ht_free(main->servers);
   free(main->hash_ring);
   free(main); 
}
