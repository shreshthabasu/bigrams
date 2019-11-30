#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "hashtable.h"

typedef struct hashtable_entry {
    char *key;
    int value;
} hashtable_entry_t;

typedef struct hashtable {
    hashtable_entry_t *entries;
    int size;
    int num_entries;
} hashtable_t;

uint32_t rotate_left(uint32_t value, uint32_t count) {
    return value << count | value >> (32 - count);
}

uint32_t fxhash32_step(uint32_t hash, uint32_t value) {
    const uint32_t key = 0x27220a95;
    return (rotate_left(hash, 5) ^ value) * key;
}

uint32_t fxhash32_hash(uint8_t *data, int n) {
    uint32_t hash = 0;
    uint8_t block_size = 4;
    for (int i = 0; i < n / block_size; i++) {
        uint32_t number;
        memcpy(&number, data, sizeof(number));
        hash = fxhash32_step(hash, number);
        for (int k = 0; k < block_size; k++) {
            data++;
        }
    }
    for (int i = 0; i < n % block_size; i++) {
        hash = fxhash32_step(hash, *data);
        data++;
    }
    return hash;
}

uint32_t fibonacci32_reduce(uint32_t hash, int table_size) {
    uint32_t factor32 = 2654435769;
    return (hash * factor32) >> (32 - (int)log2(table_size));
}

uint32_t count_collisions(hashtable_t *ht) {
    uint32_t counter_table[ht->size];
    for (int k = 0; k < ht->size; k++) {
        counter_table[k] = 0;
    }
    for (int i = 0; i < ht->size; i++) {
        if (ht->entries[i].key) {
            uint8_t *data = (uint8_t *)ht->entries[i].key;
            int data_len = (int)strlen(ht->entries[i].key);
            uint32_t hash = fxhash32_hash(data, data_len);
            uint32_t red_hash = fibonacci32_reduce(hash, ht->size);
            counter_table[red_hash] += 1;
        }
    }
    uint32_t collision = 0;
    for (int i = 0; i < ht->size; i++) {
        if (counter_table[i] > 1) {
            collision += counter_table[i] - 1;
        }
    }
    return collision;
}

hashtable_t *hashtable_create(void) {
    hashtable_t *table = calloc(1, sizeof(hashtable_t));
    table->size = 128;
    table->entries = calloc(table->size, sizeof(hashtable_entry_t));
    return table;
}

void hashtable_grow(hashtable_t *ht) {
    hashtable_t *new_table = calloc(1, sizeof(hashtable_t));
    new_table->size = 2 * ht->size;
    new_table->entries = calloc(new_table->size, sizeof(hashtable_entry_t));
    for (int i = 0; i < ht->size; i++) {
        if (ht->entries[i].key) {
            hashtable_set(new_table, ht->entries[i].key, ht->entries[i].value);
            free(ht->entries[i].key);
        }
    }
    free(ht->entries);
    ht->entries = new_table->entries;
    ht->num_entries = new_table->num_entries;
    ht->size = new_table->size;
    free(new_table);
}

void hashtable_set(hashtable_t *ht, char *key, int value) {
    double load_factor = ht->num_entries / (double)ht->size;
    if (load_factor >= 0.5) {
        uint32_t coll_before = count_collisions(ht);
        hashtable_grow(ht);
        uint32_t coll_after = count_collisions(ht);
        printf("Rehashing reduced collisions from %d to %d\n", coll_before, coll_after);
    }
    int data_len = (int)strlen(key);
    uint8_t *data = (uint8_t *)key;
    uint32_t hash = fxhash32_hash(data, data_len);
    uint32_t red_hash = fibonacci32_reduce(hash, ht->size);
    while (1) {
        if (!(ht->entries[red_hash].key)) {
            ht->entries[red_hash].key = strdup(key);
            ht->entries[red_hash].value = value;
            ht->num_entries += 1;
            break;
        }
        if (strcmp(ht->entries[red_hash].key, key) == 0) {
            ht->entries[red_hash].value = value;
            break;
        }
        red_hash = (red_hash + 1) % ht->size;
    }
}

bool hashtable_get(hashtable_t *ht, char *key, int *value) {
    int data_len = (int)strlen(key);
    uint8_t *data = (uint8_t *)key;
    uint32_t hash = fxhash32_hash(data, data_len);
    uint32_t red_hash = fibonacci32_reduce(hash, ht->size);
    while (true) {
        if (!ht->entries[red_hash].key) {
            return false;
        }
        if (strcmp(ht->entries[red_hash].key, key) == 0) {
            *value = ht->entries[red_hash].value;
            return true;
        }
        red_hash = (red_hash + 1) % ht->size;
    }
}

void hashtable_destroy(hashtable_t *ht) {
    for (int i = 0; i < ht->size; i++) {
        if (ht->entries[i].key) {
            free(ht->entries[i].key);
        }
    }
    free(ht->entries);
    free(ht);
}

int hashtable_probe_max(hashtable_t *ht) {
    return ht->size;
}

int hashtable_size(hashtable_t *ht) {
    return ht->num_entries;
}

bool hashtable_probe(hashtable_t *ht, int i, char **key, int *val) {
    if (ht->entries[i].key) {
        *key = ht->entries[i].key;
        *val = ht->entries[i].value;
        return true;
    }
    return false;
}
