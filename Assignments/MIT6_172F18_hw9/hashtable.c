/* Copyright (c) 2013 MIT License by 6.172 Staff
 *
 * DON'T USE THE FOLLOWING SOFTWARE, IT HAS KNOWN BUGS, AND POSSIBLY
 * UNKNOWN BUGS AS WELL.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include "./hashlock.h"
#include <pthread.h>

#include "./common.h"

#define HASHBITS 8
#define TABLESIZE (1<<HASHBITS)
#define MAX_ENTRIES (TABLESIZE-1)

typedef struct entry {
  void* ptr;
  size_t size;
} entry_t;

typedef struct hashtable_t {
  entry_t hashtable[TABLESIZE];
  int entries;
  pthread_mutex_t lock;
} hashtable_t;

hashtable_t ht = {{}, 0, PTHREAD_MUTEX_INITIALIZER};

/* golden ratio (sqrt(5)-1)/2 * (2^64) */
#define PHI_2_64  11400714819323198485U

int hash_func(void* p) {
  long x = (long)p;
  /* multiplicative hashing */
  return (x * PHI_2_64) >> (64 - HASHBITS);
}

void hashtable_insert(void* p, int size) {
  assert(ht.entries < TABLESIZE);
  ht.entries++;

  int s = hash_func(p);
  /* open addressing with linear probing */
  do {
    if (!ht.hashtable[s].ptr) {
      ht.hashtable[s].ptr = p;
      ht.hashtable[s].size = size;
      break;
    }
    /* conflict, look for next item */
    s++;
  } while (1);
}

void hashtable_lock() {
  pthread_mutex_lock(&ht.lock);
}

void hashtable_unlock() {
  pthread_mutex_unlock(&ht.lock);
}

void hashtable_insert_locked(void* p, int size) {
  int s = hash_func(p);
  /* open addressing with linear probing */
  hashtable_lock();
  assert(ht.entries < TABLESIZE);
  ht.entries++;

  do {
    if (!ht.hashtable[s].ptr) {
      ht.hashtable[s].ptr = p;
      ht.hashtable[s].size = size;
      break;
    }
    /* conflict, look for next item */
    s++;
  } while (1);
  hashtable_unlock();
}

void hashtable_insert_fair(void* p, int size) {
  hashtable_lock();
  assert(ht.entries < TABLESIZE);
  ht.entries++;
  hashtable_unlock();

  int s = hash_func(p);
  /* open addressing with linear probing */
  hashlock_lock(s);
  do {
    if (!ht.hashtable[s].ptr) {
      ht.hashtable[s].ptr = p;
      ht.hashtable[s].size = size;
      break;
    }
    int olds = s;
    /* conflict, look for next item */
    s++;
    /* fair lock, hold the old lock, before grabbing the new one */
    hashlock_lock(s);
    hashlock_unlock(olds);
  } while (1);
  hashlock_unlock(s);
}

void hashtable_insert_lockless(void* p, int size) {
  hashtable_lock();
  assert(ht.entries < TABLESIZE);
  ht.entries++;
  hashtable_unlock();

  int s = hash_func(p);
  /* open addressing with linear probing */
  do {
    if (!ht.hashtable[s].ptr) {
      ht.hashtable[s].ptr = p;
      ht.hashtable[s].size = size;
      break;
    }
    /* conflict, look for next item */
    s++;
  } while (1);
}

entry_t* hashtable_lookup(void* p) {
  size_t s = hash_func(p);
  do {
    if (p == ht.hashtable[s].ptr) {
      return &ht.hashtable[s];
    }
    if (ht.hashtable[s].ptr == NULL) {
      /* not found */
      return NULL;
    }
    if (ht.hashtable[s].size == 0) {
      /* race on incomplete size, pretend not found */
      return NULL;
    }
    /* check next */
    s++;
#ifndef STUDENT_VERSION
    s %= TABLESIZE;
#endif
  } while (1);
}

void hashtable_dump(void) {
  int i;
  for (i = 0; i < TABLESIZE; i++)
    if (ht.hashtable[i].ptr) {
      printf("%d %p %ld\n", i, ht.hashtable[i].ptr, ht.hashtable[i].size);
    }
}

void hashtable_entries(void) {
  int i;
  int count = 0;
  for (i = 0; i < TABLESIZE; i++) {
    if (ht.hashtable[i].ptr) {
      count++;
    }
  }
}

void hashtable_test(int n) {
  int i;
  for (i = 0; i < n; i++) {
    int s = random() & (TABLESIZE - 1);
    void* p = ht.hashtable[s].ptr;
    void* pl = hashtable_lookup(p);
    assert(p == pl);
  }
}

void hashtable_free(void) {
  int i;
  for (i = 0; i < TABLESIZE; i++) {
    if (ht.hashtable[i].ptr) {
      free(ht.hashtable[i].ptr);
      ht.entries--;
    }
  }
}

void hashtable_fill(int n) {
  int i;
  printf("fill %d\n", n);
  for (i = 0; i < n; i++) {
    int sz = random() % 1000;
    char* p = malloc(sz);
    // test one of these below
    hashtable_insert(p, sz);
    // hashtable_insert_locked(p, sz);
    // hashtable_insert_fair(p, sz);
    // hashtable_insert_lockless(p, sz);
  }
}

int main(int argc, char* argv[]) {
  int n = MAX_ENTRIES / 2;     /* 50 % fill ratio */

  // GCC sometimes gets confused by Cilk
//  #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

  int threads = 1;
  if (argc > 1) {
    threads = atoi(argv[1]);
  }

  unsigned seed = time(NULL);
  if (argc > 2) {
    seed = atoi(argv[2]);
  }

  /* verify hashtable entries are aligned, otherwise even 64bit
   * writes/reads won't be guaranteed to be atomic
   */
  assert((uintptr_t)ht.hashtable % sizeof(long) == 0);

  srandom(seed);
#ifdef VERBOSE
  printf("seed = %d\n", seed);
  printf("sizeof entry = %ld\n", sizeof(entry_t));
  printf("&entries = %p &last=%p\n", &ht.entries, &ht.hashtable[TABLESIZE]);
#endif

#ifdef CILK
  int i;
  for (i = 1; i < threads; i++) {
    cilk_spawn hashtable_fill(n / threads);
  }
#endif
  hashtable_fill(n / threads);

#ifdef CILK
  cilk_sync;
#endif

#ifdef VERY_VERBOSE
  hashtable_dump();
#endif
  printf("%d entries\n", ht.entries);
  hashtable_free();
  printf("%d entries\n", ht.entries);
  assert(ht.entries == 0);
  return 0;
}
