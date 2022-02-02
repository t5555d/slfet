#include <emmintrin.h>

#include "routines.h"

void add_vec_c(ADD_VEC_ARGS)
{
    for (size_t i = 0; i < n; i++)
        c[i] = a[i] + b[i];
}

void add_vec_sse41(ADD_VEC_ARGS)
{
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m128i vi = _mm_load_si128(reinterpret_cast<const __m128i *>(a + i));
        vi = _mm_add_epi16(vi, *reinterpret_cast<const __m128i *>(b + i));
        _mm_store_si128(reinterpret_cast<__m128i *>(c + i), vi);
    }

    for (; i < n; i++)
        c[i] = a[i] + b[i];
}

int vec_sum_c(VEC_SUM_ARGS)
{
    int sum = 0;
    for (size_t i = 0; i < n; i++)
        sum += a[i];
    return sum;
}

int vec_sum_sse41(VEC_SUM_ARGS)
{
    __m128i zero = _mm_setzero_si128();
    __m128i vsum = zero;

    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m128i vi = _mm_load_si128(reinterpret_cast<const __m128i *>(a + i));
        __m128i sn = _mm_srai_epi16(vi, 15);
        __m128i vl = _mm_unpacklo_epi16(vi, sn);
        __m128i vh = _mm_unpackhi_epi16(vi, sn);

        vsum = _mm_add_epi32(vsum, vl);
        vsum = _mm_add_epi32(vsum, vh);
    }

    int32_t sum = _mm_cvtsi128_si32(vsum);
    vsum = _mm_srli_si128(vsum, 4);
    sum += _mm_cvtsi128_si32(vsum);
    vsum = _mm_srli_si128(vsum, 4);
    sum += _mm_cvtsi128_si32(vsum);
    vsum = _mm_srli_si128(vsum, 4);
    sum += _mm_cvtsi128_si32(vsum);    

    for (; i < n; i++)
        sum += a[i];

    return sum;
}

