#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

#include "hashtable.h"

typedef unsigned int uint;

#define MAX_BUCKET_SIZE 64
#define RESIZE_CONST 0.75

void key_val_free_function(void *data) {
	if(!data) 
        return;
    info_t *hash = (info_t *)data;

    if(hash->key)
        free(hash->key);
    if(hash->value)
        free(hash->value);

    free(hash);
}
/*
 * Functii de comparare a cheilor:
 */
int
compare_function_ints(void *a, void *b)
{
	int int_a = *((int *)a);
	int int_b = *((int *)b);

	if (int_a == int_b) {
		return 0;
	} else if (int_a < int_b) {
		return -1;
	} else {
		return 1;
	}
}

int
compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

/*
 * Functii de hashing:
 */
unsigned int
hash_function_int(void *a)
{
	/*
	 * Credits: https://stackoverflow.com/a/12996028/7883884
	 */
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

unsigned int
hash_function_string(void *a)
{
	/*
	 * Credits: http://www.cse.yorku.ca/~oz/hash.html
	 */
	unsigned char *puchar_a = (unsigned char*) a;
	unsigned long hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c; /* hash * 33 + c */

	return hash;
}


/*
 * Functie apelata dupa alocarea unui hashtable pentru a-l initializa.
 * Trebuie alocate si initializate si listele inlantuite.
 */
hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*),
		void (*key_val_free_function)(void*))
{
	hashtable_t *ht = malloc(sizeof(*ht));
	DIE(ht == NULL, "malloc failed");

	ht->buckets = malloc(hmax * sizeof(*ht->buckets));
	DIE(ht->buckets == NULL, "malloc failed");

	for (uint i = 0; i < hmax; i++)
		ht->buckets[i] = ll_create(sizeof(info_t));
	
	ht->hmax = hmax;
	ht->hash_function = hash_function;
	ht->compare_function = compare_function;
	ht->key_val_free_function = key_val_free_function;
	return ht;
}

static ll_node_t *find_key(linked_list_t *bucket, void *key,
	int (*compare_function)(void*, void*), unsigned int *pos)
{
	ll_node_t *current = bucket->head;
	for (uint i = 0; i < bucket->size; ++i) {
		if (compare_function(key, ((info_t *)current->data)->key) == 0) {
			*pos = i;
			return current;
		}
		current = current->next;
	}
	
	return NULL;
}	

void ht_resize(hashtable_t *ht, uint key_size, uint value_size)
{
	int n = 2 * ht->hmax;
	hashtable_t *new_ht = ht_create(n, ht->hash_function, ht->compare_function, ht->key_val_free_function);


	for (int i = 0; i < ht->hmax; i++) {
		linked_list_t *bucket = ht->buckets[i];
		ll_node_t *current = bucket->head;

		for (int j = 0; j < bucket->size; j++) {
			ht_put(new_ht, ((info_t *)current->data)->key, key_size, ((info_t *)current->data)->value, value_size);
			current = current->next;
		}
		
	}

	for (int i = 0; i < ht->hmax; i++) {

		while (ht->buckets[i]->head) {
			ll_node_t *removed = ll_remove_nth_node(ht->buckets[i], 0);
			free(((info_t *)removed->data)->value);
			free(((info_t *)removed->data)->key);
			free(removed->data);
			free(removed);
		}

		free(ht->buckets[i]);		
	}

	free(ht->buckets);

	ht->buckets = new_ht->buckets;
	ht->hmax = n;
	printf("salutik\n");
	printf("ala nou %d \n", new_ht->hmax);
	printf("%d \n", ht->hmax);
	
	free(new_ht);
}

/*
 * Atentie! Desi cheia este trimisa ca un void pointer (deoarece nu se impune tipul ei), in momentul in care
 * se creeaza o noua intrare in hashtable (in cazul in care cheia nu se gaseste deja in ht), trebuie creata o copie
 * a valorii la care pointeaza key si adresa acestei copii trebuie salvata in structura info asociata intrarii din ht.
 * Pentru a sti cati octeti trebuie alocati si copiati, folositi parametrul key_size_bytes.
 *
 * Motivatie:
 * Este nevoie sa copiem valoarea la care pointeaza key deoarece dupa un apel put(ht, key_actual, value_actual),
 * valoarea la care pointeaza key_actual poate fi alterata (de ex: *key_actual++). Daca am folosi direct adresa
 * pointerului key_actual, practic s-ar modifica din afara hashtable-ului cheia unei intrari din hashtable.
 * Nu ne dorim acest lucru, fiindca exista riscul sa ajungem in situatia in care nu mai stim la ce cheie este
 * inregistrata o anumita valoare.
 */
void
ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	info_t data;
	uint idx = ht->hash_function(key) % ht->hmax;
	linked_list_t *bucket = ht->buckets[idx];
	uint pos;
	ll_node_t *node = find_key(bucket, key, ht->compare_function, &pos);

	if (!node) {
		data.key = malloc(key_size);
		DIE(NULL == data.key, "malloc failed");
		memcpy(data.key, key, key_size);

		data.value = malloc(value_size);
		DIE(NULL == data.value, "malloc failed");
		memcpy(data.value, value, value_size);///data.value = value;

		ll_add_nth_node(bucket, 0, &data);
	} else {
		memcpy(((info_t *)node->data)->value, value, value_size);
	}
	
	ht->size++;
	
	if ((double) ht->size / ht->hmax > RESIZE_CONST)
		ht_resize(ht, key_size, value_size);
}
void
ht_put_modified(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	info_t data;
	uint idx = ht->hash_function(key) % ht->hmax;
	linked_list_t *bucket = ht->buckets[idx];
	uint pos;
	ll_node_t *node = find_key(bucket, key, ht->compare_function, &pos);

	if (!node) {
		data.key = malloc(key_size);
		DIE(NULL == data.key, "malloc failed");
		memcpy(data.key, key, key_size);

		data.value = malloc(value_size);
		DIE(NULL == data.value, "malloc failed");
		data.value = value;///data.value = value;

		ll_add_nth_node(bucket, 0, &data);
	} else {
		memcpy(((info_t *)node->data)->value, value, value_size);
	}
	
	ht->size++;
	
	if ((double) ht->size / ht->hmax > RESIZE_CONST)
		ht_resize(ht, key_size, value_size);
}

void *
ht_get(hashtable_t *ht, void *key)
{
	uint idx = ht->hash_function(key) % ht->hmax;
	linked_list_t *bucket = ht->buckets[idx];
	uint pos;
	ll_node_t *node = find_key(bucket, key, ht->compare_function, &pos);

	if (node)
		return ((info_t *)node->data)->value;

	return 0;
}

/*
 * Functie care intoarce:
 * 1, daca pentru cheia key a fost asociata anterior o valoare in hashtable folosind functia put
 * 0, altfel.
 */
int
ht_has_key(hashtable_t *ht, void *key)
{
	uint idx = ht->hash_function(key) % ht->hmax;
	linked_list_t *bucket = ht->buckets[idx];
	uint pos;
	ll_node_t *node = find_key(bucket, key, ht->compare_function, &pos);

	if (node)
		return 1;

	return 0;
}

/*
 * Procedura care elimina din hashtable intrarea asociata cheii key.
 * Atentie! Trebuie avuta grija la eliberarea intregii memorii folosite pentru o intrare din hashtable (adica memoria
 * pentru copia lui key --vezi observatia de la procedura put--, pentru structura info si pentru structura Node din
 * lista inlantuita).
 */
void
ht_remove_entry(hashtable_t *ht, void *key)
{
	uint idx = ht->hash_function(key) % ht->hmax;
	linked_list_t *bucket = ht->buckets[idx];
	uint pos;
	ll_node_t *node = find_key(bucket, key, ht->compare_function, &pos);

	if (node) {
		ll_node_t *removed = ll_remove_nth_node(bucket, pos);
		free(((info_t *)removed->data)->value);
		free(((info_t *)removed->data)->key);
		free(removed->data);
		free(removed);
		ht->size--;
	}
}

/*
 * Procedura care elibereaza memoria folosita de toate intrarile din hashtable, dupa care elibereaza si memoria folosita
 * pentru a stoca structura hashtable.
 */
void
ht_free(hashtable_t *ht)
{
	ll_node_t *current;
	for (uint i = 0; i < ht->hmax; i++) {
		if (ht->buckets[i]->head != NULL) {
			current = ht->buckets[i]->head;
			while (current != NULL) {
				free(((info_t *)current->data)->value);
				free(((info_t *)current->data)->key);
				current = current->next;
			}
		}
		ll_free(&ht->buckets[i]);
	}

	free(ht->buckets);
	free(ht);
}

unsigned int
ht_get_size(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->size;
}

unsigned int
ht_get_hmax(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->hmax;
}