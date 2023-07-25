#ifndef __TRIE_H_
#define __TRIE_H_ 


typedef struct trie * Trie;

Trie trie();

/*Insertion accept ( a certain trie, a pointer to a string, a pointer )*/
const char * trie_insert(Trie trie, const char *string, void *end);


/* checks if a string is in a trie recives (trie, pointer to a string) and return int (boolean 0,1)*/
void * trie_exists(Trie trie, const char *string);

/* Deleting the string from the trie by setting the ending pointer to NULL*/
void trie_delete(Trie trie,const char *string);

/* Recvive a pointer to a pointer to trie to realese the memorey*/
void trie_destroy(Trie * trie);



#endif


/* After finidng the word we send a pointer that point to the actual word or set of data */
/* We can also set a pointer that just gonna tells us True or False if the string actually located in the trie*/
/* To delete a string we just declare its ending pointer to a NULL */