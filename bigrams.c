#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include "hashtable.h"

bool read_word(FILE *fp, char *word) {
    char c;
    int i = 0;
    bool eof_flag = false;
    if (!fp) {
        fprintf(stderr, "Could not open file\n");
        exit(1);
    }
    while (true) {
        c = (char)fgetc(fp);
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            word[i] = c;
            i++;
        } else {
            if (c == EOF) {
                eof_flag = true;
            }
            word[i] = '\0';
            break;
        }
    }
    return eof_flag;
}

void print_bigram_count(hashtable_t *ht) {
    int max_entries = hashtable_probe_max(ht);
    char *key;
    int val;
    int counter = 0;
    for (int k = 0; k < max_entries; k++) {
        bool found = hashtable_probe(ht, k, &key, &val);
        if (found && val >= 200) {
            printf("Bigram '%s' has count of %d\n", key, val);
            counter += 1;
        }
    }
    if (counter == 0) {
        for (int i = 0; i < max_entries; i++) {
            if (hashtable_probe(ht, i, &key, &val)) {
                for (int j = 0; j < val; j++) {
                    printf("%s\n", key);
                }
                printf("Bigram '%s' has count of %d\n", key, val);
            }
        }
    }
    int num_entries = hashtable_size(ht);
    printf("Total of %d different bigrams recorded\n", num_entries);
}

int main(void) {
    hashtable_t *h_table = hashtable_create();
    FILE *fp = fopen("book.txt", "r");
    int buffer_len = 256;
    char *word1 = malloc(buffer_len);
    int word1_len = 0;
    while (word1_len == 0) {
        read_word(fp, word1);
        word1_len = (int)strlen(word1);
    }
    while (!(feof(fp))) {
        char *word2 = malloc(buffer_len);
        char *buffer = malloc(buffer_len);
        int word2_len = 0;
        while (word2_len == 0) {
            bool eof_flag = read_word(fp, word2);
            word2_len = (int)strlen(word2);
            if (eof_flag) {
                break;
            }
        }
        if (word2_len > 0) {
            snprintf(buffer, buffer_len, "%s %s", word1, word2);
            //printf("%s\n", buffer);
            int counter_val = 0;
            bool found = hashtable_get(h_table, buffer, &counter_val);
            if (found) {
                counter_val += 1;
            } else {
                counter_val = 1;
            }
            hashtable_set(h_table, buffer, counter_val);
            free(word1);
            word1 = strdup(word2);
        }
        free(word2);
        free(buffer);
    }
    print_bigram_count(h_table);
    free(word1);
    hashtable_destroy(h_table);
    return 0;
}
