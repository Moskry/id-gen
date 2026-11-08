#include "idgen.h"
#include <pthread.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void *single_thread(void *mutock);
pthread_t threads[12];

int main(void) {
  set_epoch(2000);

  printf("size of IdgenMutock: %zu\n", sizeof(IdgenMutock));
  printf("Alignment of IdgenMutock: %zu\n", alignof(IdgenMutock));

  IdgenMutock *mutock = malloc(sizeof(IdgenMutock));
  pthread_mutex_init(&mutock->mutex, NULL);
  pthread_cond_init(&mutock->cond, NULL);

  printf("Single-threaded solution: \n\n");
  single_thread(mutock);

  printf("\nmore threads!\n\n");
  for (int i = 0; i < 12; i++) {
    printf("\nThread %d spawned: \n\n", i + 1);
    if (pthread_create(&threads[i], NULL, single_thread, mutock) != 0) {
      perror("threads couldn't get spawned");

      pthread_mutex_destroy(&mutock->mutex);
      pthread_cond_destroy(&mutock->cond);
      free(mutock);
      exit(EXIT_FAILURE);
    }
  }

  for (int i = 0; i < 12; i++)
    pthread_join(threads[i], NULL);

  pthread_mutex_destroy(&mutock->mutex);
  pthread_cond_destroy(&mutock->cond);
  free(mutock);
  return EXIT_SUCCESS;
}

void *single_thread(void *mutock) {
  Idgen_t idgen = {.mutock = (IdgenMutock*)mutock, .ticket = 0};
  for (int i = 0; i < 10; i++) {
    uint64_t id = generate_id(&idgen);
    printf("ID: %lu \tticket: %d\n", id, idgen.ticket);
  }
  return NULL;
}
