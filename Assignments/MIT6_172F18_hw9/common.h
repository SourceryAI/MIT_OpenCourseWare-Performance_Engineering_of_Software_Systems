/* Copyright (c) 2013 MIT License by 6.172 Staff
 *
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

// Atomic:
//   prev = *ptr;
//   if (*ptr == old) *ptr = _new;
//   return prev
static inline uint32_t InterlockedCompareExchange32(volatile uint32_t* ptr, uint32_t _new,
                                                    uint32_t old) {
  uint32_t prev;
  asm volatile("lock;"
               "cmpxchgl %1, %2;"
               : "=a"(prev)
               : "q"(_new), "m"(*ptr), "a"(old)
               : "memory");
  return prev;
}

static inline uint64_t InterlockedCompareExchange64(volatile uint64_t* ptr, uint64_t _new,
                                                    uint64_t old) {
  uint64_t prev;
  asm volatile("lock;"
               "cmpxchgq %1, %2;"
               : "=a"(prev)
               : "q"(_new), "m"(*ptr), "a"(old)
               : "memory");
  return prev;
}


#endif
