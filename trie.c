#include "trie.h"
#include <stdio.h>
#include <stdlib.h>

#define TRIE_BASE_CHAR ' '

/* We're gonna have a pointer per char. We're using the chars between(ASCII) 32->126 = 96 Pointers */
/* current -> the current char */
/* end -> a pointer from the end of a string*/
/* Beacuse we're using only certain chars from ASCII (36->132) our base char is 36 in ascii which is SPACE*/
/* so we will set the base to space */
struct trie_node {
    char current;
    void *end;
    struct trie_node *next[95];
};

/* The Trie */
struct trie {
    struct trie_node *next[95]; /* This is the head of trie -> pointer to all the other nodes */
};

Trie trie() {
    return calloc(1, sizeof(struct trie)); /* return a Trie */
}

/* This function recursively searches for a given string in a trie structure. */
/* If we have reached the end of the string and the current node has a non-NULL end value,
   it means the string is found in the trie, so we return the current node. */
static struct trie_node *internal_trie_exist(struct trie_node *node_i, const char *string) {
    if (node_i == NULL)
        return NULL;
    if (*string == '\0' && node_i->end != NULL)
        return node_i;
    return internal_trie_exist(node_i->next[(*string) - TRIE_BASE_CHAR], string + 1);
}

/* trie_exists function:
   Returns a pointer to the end value associated with the given string in the trie if it exists,
   otherwise returns NULL. */
void *trie_exists(Trie trie, const char *string) {
    struct trie_node *find_node = internal_trie_exist(trie->next[(*string) - TRIE_BASE_CHAR], string + 1);
    return find_node == NULL ? NULL : find_node->end;
}

/* trie_delete function:
   Deletes the end value associated with the given string in the trie if it exists. */
void trie_delete(Trie trie, const char *string) {
    struct trie_node *find_node = internal_trie_exist(trie->next[(*string) - TRIE_BASE_CHAR], string + 1);
    if (find_node != NULL)
        find_node->end = NULL;
}

/* trie_insert function:
   Inserts the given string into the trie and associates it with the provided end value.
   Returns the pointer to the remaining portion of the string after insertion. */
const char *trie_insert(Trie trie, const char *string, void *end) {
    if (*string == '\0')
        return NULL;

    struct trie_node **iterator = &trie->next[(*string) - TRIE_BASE_CHAR];
    while (1) {
        if (*iterator == NULL) {
            *iterator = calloc(1, sizeof(struct trie_node));
            if (*iterator == NULL)
                return NULL;
        }
        string++;
        if (*string == '\0') {
            (*iterator)->end = end;
            break;
        }
        iterator = &(*iterator)->next[(*string) - TRIE_BASE_CHAR];
    }
    return string;
}
