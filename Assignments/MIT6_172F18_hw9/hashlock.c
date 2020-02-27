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

#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>

#define HLOCKS 16
pthread_mutex_t hashlock[HLOCKS];
#define HASHLOCK_SLOT(l) &hashlock[(l) & (HLOCKS - 1)]

void hashlock_init() {
  int i;
  pthread_mutexattr_t recursive;
  pthread_mutexattr_init(&recursive);
  pthread_mutexattr_settype(&recursive, PTHREAD_MUTEX_RECURSIVE);
  for (i = 0; i < HLOCKS; i++) {
    pthread_mutex_init(&hashlock[i], &recursive);
  }
}

void hashlock_lock(int l) {
  pthread_mutex_lock(HASHLOCK_SLOT(l));
  #ifdef UNIT_TEST
  printf("lock %d\n", l);
  #endif
}

void hashlock_unlock(int l) {
  pthread_mutex_unlock(HASHLOCK_SLOT(l));
  #ifdef UNIT_TEST
  printf("unlock %d\n", l);
  #endif
}

#ifdef UNIT_TEST
int main() {
  hashlock_init();
  printf("lock size %ld\n", sizeof(hashlock[0]));

  hashlock_lock(2);
  hashlock_lock(2);
  hashlock_lock(5);
  hashlock_unlock(2);
  hashlock_unlock(5);
  return 0;
}
#endif
