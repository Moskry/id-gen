#include "idgen.h"
#include <math.h>
#include <pthread.h>
#include <stdalign.h>
#include <stdint.h>
#include <threads.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

atomic_int ticket_count = 1;
atomic_int ticket_ready = 1;
uint64_t epoch_milliseconds;
uint64_t current_millisecond;
uint16_t user_count = 1;
char epoch_changed = 'n';

void get_ticket(Idgen_t *idgen) { idgen->ticket= ticket_count++; }

uint64_t get_time_difference(void) {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);

  uint64_t now_ms = ts.tv_sec * 1000LL + ts.tv_nsec / 1000000;
  return now_ms - epoch_milliseconds;
}

uint64_t generate_id(Idgen_t *idgen) {
  get_ticket(idgen);
  pthread_mutex_lock(&idgen->mutock->mutex);
  while (idgen->ticket != ticket_ready)
    pthread_cond_wait(&idgen->mutock->cond, &idgen->mutock->mutex);

  {
    uint64_t timediff = get_time_difference();
    if (current_millisecond != timediff) {
      current_millisecond = timediff;
      user_count = 1;
    }
    while (user_count == pow(2, 14)) {
      struct timespec nanosleeptime = {.tv_nsec = 1000, .tv_sec = 0};
      nanosleep(&nanosleeptime, NULL);
      timediff = get_time_difference();
      if (current_millisecond != timediff) {
        current_millisecond = timediff;
        user_count = 1;
      }
    }
  }

  uint64_t snowflake = (current_millisecond << 14) | user_count++;

  ticket_ready++;
  pthread_mutex_unlock(&idgen->mutock->mutex);

  pthread_cond_broadcast(&idgen->mutock->cond);
  return snowflake;
}

void set_epoch(int year) {
  if (year <= 1970) {
    perror("The year must be bigger than 1970");
    exit(1);
  }
  if (epoch_changed == 't') {
    fprintf(stderr, "Epoch already defined, ignoring the change...\n");
    fflush(stderr);
    return;
  }
  struct tm epoch = {0};
  epoch.tm_year = year - 1900;
  epoch_milliseconds = mktime(&epoch) * 1000LL;
  epoch_changed = 't';
}
