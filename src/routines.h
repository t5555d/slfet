#pragma once

#include <stddef.h>
#include <stdint.h>

#define ADD_VEC_ARGS const int16_t *a, const int16_t *b, int16_t *c, size_t n
#define VEC_SUM_ARGS const int16_t *a, size_t n

typedef void (*add_vec_fn)(ADD_VEC_ARGS);

void add_vec_c(ADD_VEC_ARGS);
void add_vec_sse41(ADD_VEC_ARGS);

typedef int(*vec_sum_fn)(VEC_SUM_ARGS);

int vec_sum_c(VEC_SUM_ARGS);
int vec_sum_sse41(VEC_SUM_ARGS);
