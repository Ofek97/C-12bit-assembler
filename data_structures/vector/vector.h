
#ifndef __VECTORR_H_
#define __VECTORR_H_
#include <stddef.h>

struct vector;

typedef struct vector * Vector;


Vector new_vector(void * (*ctor)(const void *copy),void (*dtor)(void *item));



void * vector_insert(Vector v,const void * copy_item);


void *const *vector_begin(const Vector v);

void *const *vector_end(const Vector v);


size_t vector_get_item_count(const Vector v);


void vector_destroy(Vector * v);



#define VECTOR_FOR_EACH(begin,end,v) for(begin   = vector_begin(v),end= vector_end(v);begin <= end;begin++)


#define VECTOR_FOR_EACH_R(begin,end,v) for(begin   = vector_begin(v),end= vector_end(v);begin <= end;end--)

#endif
