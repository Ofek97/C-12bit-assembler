#include "trie.h"
#include <stdio.h>

int main() {
    int i;
    Trie t;
    const char *f;
    const char *strings[4] = { 
        "yarin",
        "hello",
        "hellow",
        "hellno"
    };

    t = trie();
    /* Insertion to the Trie our strings */
    for (i=0;i<4;i++) {
        trie_insert(t, strings[i], (char*)strings[i]);
    }

    for (i=0;i<4;i++) {
        f = trie_exists(t, strings[i]);
        if(f) {
            printf("%s", f);
        }
        else {
            printf("string:'%s' could not be found,\n", strings[i]);
        }
    }

    /*Deleting hello fron the Trie*/
    trie_delete(t, strings[1]);

    /* checking which string from our strings is exists in our Trie*/
     for (i=0;i<4;i++) {
        f = trie_exists(t, strings[i]);
        if(f) {
            printf("%s", f);
        }
        else {
            printf("string:'%s' could not be found,\n", strings[i]);
        }
    }
    return 0;
}  