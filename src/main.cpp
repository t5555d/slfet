#include <array>
#include <iostream>
#include <slfet.h>
#include "routines.h"

template<typename T, size_t N>
struct array_t : std::array<T, N>
{
    operator T *() { return &std::array<T,N>::at(0); }
};

int main(int argc, const char *argv[])
{
    using namespace slfet;
    using bin_test_t = num_init_t<int>;

    constexpr size_t SIZE = 21;
    std::vector<bin_test_t> tests = {
        { "min", 0 },
        { "max", 255 },
        { "mid", 128 },
        { "inc", 100, 1 },
        { "dec", 100, -2 },
        { "rand1", 12345, 1664525L, 1013904223L },
        { "rand2", -1234567, 1664525L, 1013904223L }
    };
    cout_runtime_t runtime;

    alignas(16) int16_t aa[SIZE], ab[SIZE], ac0[SIZE], ac1[SIZE];
    run_tests(runtime, add_vec_c, add_vec_sse41, in(aa, tests), in(ab, tests), out(ac0, ac1), SIZE);
    run_tests(runtime, vec_sum_c, vec_sum_sse41, in(aa, tests), SIZE);

    alignas(16) array_t<short, SIZE> va, vb, vc0, vc1;
    run_tests(runtime, add_vec_c, add_vec_sse41, in(va, tests), in(vb, tests), out(vc0, vc1), SIZE);
    run_tests(runtime, vec_sum_c, vec_sum_sse41, in(va, tests), SIZE);

    return 0;
}

