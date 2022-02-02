#ifndef __SLFET__
#define __SLFET__

/***
* Stateless functions equivalence testing (SLFET).
*/

#include <array>
#include <algorithm>
#include <limits>
#include <iostream>
#include <string>
#include <vector>

#define SLFET_OPEN_NS namespace slfet {
#define SLFET_CLOSE_NS }


SLFET_OPEN_NS;

using std::to_string;

//
// sample initializer of numeric inputs
//

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

template<typename Int, int Min = std::numeric_limits<Int>::min(), int Max = std::numeric_limits<Int>::max()>
struct num_init_t
{
    std::string name;
    Int value;
    Int inc;
    Int mul;
    Int min;
    Int max;

    num_init_t(const std::string& n, Int v, Int i = 0, Int m = 1, Int _min = Min, Int _max = Max) :
        name(n), value(v), inc(i), mul(1), min(_min), max(_max) {}

    static Int in_range(Int val, Int min, Int max) {
        if (val < min)
            return max - (min - val) % (max + 1 - min);
        else
            return min + (val - min) % (max + 1 - min);
    }

    template<typename T>
    void init_array(T *list, size_t size) const {
        const Int _min = std::max(min, (Int)std::numeric_limits<T>::min());
        const Int _max = std::min(max, (Int)std::numeric_limits<T>::max());
        Int val = value;
        for (size_t i = 0; i < size; i++) {
            list[i] = static_cast<T>(in_range(val, _min, _max));
            val = val * mul + inc; // next number
        }
    }
};

template<typename Int, int Min, int Max>
const std::string& to_string(const num_init_t<Int, Min, Max>& init)
{
    return init.name;
}

template<typename T, size_t SIZE, typename Int, int Min, int Max>
void init_test_data(T(&data)[SIZE], const num_init_t<Int, Min, Max>& init)
{
    init.init_array(data, SIZE);
}

template<typename T, size_t SIZE, typename Int, int Min, int Max>
void init_test_data(std::array<T, SIZE>& data, const num_init_t<Int, Min, Max>& init)
{
    init.init_array(&data[0], SIZE);
}

template<typename T, typename Int, int Min, int Max>
void init_test_data(std::vector<T>& data, const num_init_t<Int, Min, Max>& init)
{
    init.init_array(&data[0], data.size());
}

template<typename T>
void init_test_data(T& data, const T& init)
{
    data = init;
}

//
// input, output, in/out data markers
//

template<typename T, typename Init>
struct input_data_t
{
    T *data;
    const Init& initializers;

    void initialize(int& variant, std::string& desc) const
    {
        int num_variants = (int) initializers.size();
        auto& init = initializers[variant % num_variants];
        if (desc.empty())
            desc = to_string(init);
        else
            desc = desc + " vs " + to_string(init);
        init_test_data(*data, init);
        variant /= num_variants;
    }
};

template<typename T>
struct output_data_t
{
    T *data[2];
};

template<typename T, typename Init>
struct inout_data_t
{
    T *data[2];
    const Init& initializers;

    void initialize(int& variant, std::string& desc) const {
        int num_variants = (int) initializers.size();
        auto init = initializers[variant % num_variants];
        auto name = to_string(init);
        if (name.empty())
            ; // nothing to do
        else if (desc.empty())
            desc = name;
        else
            desc = desc + " vs " + name;
        init_test_data(*data[0], init);
        init_test_data(*data[1], init);
        variant /= num_variants;
    }
};


template<typename T, typename Init>
input_data_t<T, Init> in(T& x, const Init& init)
{
    return { &x, init };
}

template<typename T, int N>
input_data_t<T, std::array<T, N>> in(T& x, const T(&init)[N])
{
    return { &x, reinterpret_cast<const std::array<T, N>&>(init) };
}

template<typename T>
output_data_t<T> out(T& a, T& b)
{
    return { &a, &b };
}

template<typename T, typename Init>
inout_data_t<T, Init> inout(T& a, T& b, const Init& init)
{
    return { &a, &b, init };
}

//
// sample runtime, printing to std::cout
//

struct cout_runtime_t
{
    template<typename name_t>
    void set_test_name(const name_t& name) {
        this->testname = name;
    }

    template<typename T>
    void assert_equal(const T& a, const T& b, const char *name) {
        assert_true(a == b, name);
    }

    void assert_true(bool requirement, const char *name) {
        std::cout << testname << " - " << name << ": " << requirement << std::endl;
    }

private:
    std::string testname;
};

//
// implementation details
//

namespace
{
    //
    // basic assertion functions
    //

    template<typename Runtime, typename T>
    void assert_equal(Runtime r, const T& a, const T& b, const char *name) {
        r.assert_equal(a, b, name);
    }

    template<typename Runtime, typename T, size_t SIZE>
    void assert_equal(Runtime r, const T(&a)[SIZE], const T(&b)[SIZE], const char *name) {
        auto& _a = reinterpret_cast<const std::array<T, SIZE>&>(a);
        auto& _b = reinterpret_cast<const std::array<T, SIZE>&>(b);
        assert_equal(r, _a, _b, name);
    }

    // init args:

    template<typename T>
    void init_arg(int& variant, std::string& desc, const T& x) {}

    template<typename T, typename Init>
    void init_arg(int& variant, std::string& desc, const input_data_t<T, Init>& input) {
        input.initialize(variant, desc);
    }

    template<typename T, typename Init>
    void init_arg(int& variant, std::string& desc, const inout_data_t<T, Init>& input) {
        input.initialize(variant, desc);
    }

    template<typename... Args>
    int init_args(int variant, std::string& desc, Args&&... args) {
        desc = "";
        auto dummy = { (init_arg(variant, desc, std::forward<Args>(args)), 0)... };
        return variant;
    }

    // get args:

    template<typename T>
    T get_arg(int, T x) {
        return x;
    }

    template<typename T, typename Init>
    T& get_arg(int, const input_data_t<T, Init>& in) {
        return *in.data;
    }

    template<typename T>
    T& get_arg(int func_index, const output_data_t<T>& out) {
        return *out.data[func_index];
    }

    template<typename T, typename Init>
    T& get_arg(int func_index, const inout_data_t<T, Init>& out) {
        return *out.data[func_index];
    }

    // return data and execution:

    template<typename T>
    struct return_data_t
    {
        T data[2];

        template<typename Func, typename... Args>
        void execute(int idx, Func& func, Args&&... args) {
            data[idx] = func(get_arg(idx, std::forward<Args>(args))...);
        }
    };

    template<>
    struct return_data_t<void>
    {
        template<typename Func, typename... Args>
        void execute(int idx, Func& func, Args&&... args) {
            func(get_arg(idx, std::forward<Args>(args))...);
        }
    };

    // check equal output:

    template<typename R, typename T>
    void check_out(R& r, const T& x) {}

    template<typename R, typename T>
    void check_out(R& r, const output_data_t<T>& out) {
        assert_equal(r, *out.data[0], *out.data[1], "output");
    }

    template<typename R, typename T, typename Init>
    void check_out(R& r, const inout_data_t<T, Init>& out) {
        assert_equal(r, *out.data[0], *out.data[1], "output");
    }

    template<typename R, typename T>
    void check_out(R& r, const return_data_t<T>& out) {
        assert_equal(r, out.data[0], out.data[1], "return");
    }

    template<typename R>
    void check_out(R& r, const return_data_t<void>& out) {}
};

//
// test running templates
//

// test running template for functions, returning something
template<typename Runtime, typename Func0, typename Func1, typename... Args>
void run_tests(Runtime& r, Func0& f0, Func1& f1, Args&&... args)
{
    std::string desc;
    for (int variant = 0; init_args(variant, desc, args...) == 0; variant++) {
        r.set_test_name(desc);

        using result_type = decltype(f0(get_arg(0, args)...));
        return_data_t<result_type> retn;
        retn.execute(0, f0, std::forward<Args>(args)...);
        retn.execute(1, f1, std::forward<Args>(args)...);

        auto dummy = {
            (check_out(r, retn), 0),
            (check_out(r, std::forward<Args>(args)), 0)...
        };
    }
}

SLFET_CLOSE_NS;

#endif // __SLFET__
