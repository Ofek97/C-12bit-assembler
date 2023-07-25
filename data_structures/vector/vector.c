
#include "vectorr.h"
#include <stdlib.h>

#define VECTOR_BEGIN_SIZE 12

struct vector {
    void ** items;
    size_t  pointer_count;
    size_t  item_count;
    void *  (*ctor)(const void *copy);
    void    (*dtor)(void *item);
};

Vector new_vector(void * (*ctor)(const void *copy),void (*dtor)(void *item)) {
    Vector new = calloc(1,sizeof(struct vector));
    if(new == NULL)
        return NULL;
    new->pointer_count = VECTOR_BEGIN_SIZE;
    new->items = calloc(VECTOR_BEGIN_SIZE,sizeof(void*));
    if(new->items == NULL) {
        free(new);
        return NULL;
    }
    new->ctor = ctor;
    new->dtor = dtor;
    return new;
}

void * vector_insert(Vector v,const void * copy_item) {
    size_t it;
    void ** temp;
    if(v->item_count == v->pointer_count) {
        v->pointer_count *=2;
        temp = realloc(v->items,v->pointer_count * sizeof(void *));
        if(temp == NULL) {
            v->pointer_count /= 2;
            return NULL;
        }
        v->items = temp;
        v->items[v->item_count] = v->ctor(copy_item);
        if(v->items[v->item_count] ==NULL) {
            return NULL;
        }
        v->item_count++;
        /*
        memset(&v->items[v->item_count],0,(v->pointer_count - v->item_count) + 1);
        */
        for(it = v->item_count; it < v->pointer_count; it++) {
            v->items[it] = NULL;
        }
    }else {
        for(it = 0;it<v->pointer_count;it++) {
            if(v->items[it] == NULL) {
                v->items[it] = v->ctor(copy_item);
                if(v->items[it] != NULL) {
                    v->item_count++;
                    break;    
                }
                return NULL;
            }
        }
    }
    return v->items[v->item_count-1];
}




void *const *vector_begin(const Vector v) {
    return v->items;
}

void *const *vector_end(const Vector v) {
    return &v->items[v->pointer_count - 1];
}

size_t vector_get_item_count(const Vector v) {
    return v->item_count; 
}

void vector_destroy(Vector * v) {
    size_t it;
    if(*v != NULL) {
        if((*v)->dtor != NULL) {
            for(it = 0;it < (*v)->pointer_count; it++) {
                if((*v)->items[it] != NULL)
                    (*v)->dtor((*v)->items[it]);
            }
        }
        free((*v)->items);
        free(*v);
        *v = NULL;
    }
}
