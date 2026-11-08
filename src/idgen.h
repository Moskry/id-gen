#ifndef IDGEN_H
#define IDGEN_H

#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>

// Interface structs
typedef struct {
  pthread_cond_t cond;
  pthread_mutex_t mutex;
  char padding[64 - ((sizeof(pthread_cond_t) + sizeof(pthread_mutex_t)) % 64)];
} IdgenMutock __attribute__((aligned(64)));

typedef struct {
  IdgenMutock *mutock;
  atomic_int ticket;
}Idgen_t;

// Interface methods
uint64_t generate_id(Idgen_t *mtk);
void set_epoch(int year);

#endif
