#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>

// for cuda
#if defined(__CUDACC__) && !defined(MATAZURE_DISABLE_CUDA)
#ifdef __clang__
#if __clang_major__ < 9
#error clang minimum version is 9 for cuda
#endif
#else
#if __CUDACC_VER_MAJOR__ < 9
#error CUDA minimum version is 10.0
#endif
#endif

#define MATAZURE_CUDA
#endif

#ifdef MATAZURE_CUDA
#define MATAZURE_GENERAL __host__ __device__
#define MATAZURE_DEVICE __device__
#define MATAZURE_GLOBAL __global__
#ifndef __clang__
#define MATAZURE_NV_EXE_CHECK_DISABLE #pragma nv_exec_check_disable
#else
#define MATAZURE_NV_EXE_CHECK_DISABLE
#endif
#else
#define MATAZURE_DEVICE
#define MATAZURE_GENERAL
#define MATAZURE_GLOBAL
#define MATAZURE_NV_EXE_CHECK_DISABLE
#endif

#ifdef _OPENMP
#define MATAZURE_OPENMP
#endif

#define MLAMBDA [=] MATAZURE_GENERAL

#if __cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1900)
#else
#error "use c++11 at least"
#endif

// for using
namespace matazure {

typedef int int_t;
typedef unsigned int uint_t;

using std::make_shared;
using std::move;
using std::shared_ptr;
using std::unique_ptr;

typedef unsigned char byte;

using std::decay;
using std::forward;
using std::remove_all_extents;
using std::remove_const;
using std::remove_cv;
using std::remove_reference;

using std::is_assignable;
using std::is_convertible;

using std::conditional;
using std::enable_if;
using std::integral_constant;
using std::is_integral;
using std::is_same;
using std::numeric_limits;

using std::get;
using std::make_tuple;
using std::tie;
using std::tuple;
using std::tuple_element;
using std::tuple_size;

using std::string;

template <bool _Val>
using bool_constant = integral_constant<bool, _Val>;

template <typename _Ty>
using decay_t = typename decay<_Ty>::type;

template <typename _Ty>
using remove_reference_t = typename remove_reference<_Ty>::type;

template <bool _Test, class _Ty = void>
using enable_if_t = typename enable_if<_Test, _Ty>::type;

template <bool _Test, class _Ty1, class _Ty2>
using conditional_t = typename conditional<_Test, _Ty1, _Ty2>::type;

template <typename _Ty>
using remove_const_t = typename remove_const<_Ty>::type;

template <typename _Ty>
using remove_cv_t = typename remove_cv<_Ty>::type;

struct blank_t {};

}  // namespace matazure

// for assert
#define MATAZURE_STATIC_ASSERT_DIM_MATCHED(T1, T2) \
    static_assert(T1::rank == T2::rank, "the rank are not matched")

#define MATAZURE_STATIC_ASSERT_VALUE_TYPE_MATCHED(T1, T2)                                \
    static_assert(std::is_same<typename T1::value_type, typename T2::value_type>::value, \
                  "the value type are not matched")

#define MATAZURE_STATIC_ASSERT_MEMORY_TYPE_MATCHED(T1, T2)           \
    static_assert(std::is_same<runtime_t<T1>, runtime_t<T2>>::value, \
                  "the memory type are not matched")

#define MATAZURE_STATIC_ASSERT_MATRIX_RANK(T) \
    static_assert(T::rank == 2, "the matrix rank should be 2")

#define MATAZURE_CURRENT_FUNCTION "(unknown)"

#if defined(__has_builtin)
#if __has_builtin(__builtin_expect)
#define MATAZURE_LIKELY(x) __builtin_expect(x, 1)
#define MATAZURE_UNLIKELY(x) __builtin_expect(x, 0)
#else
#define MATAZURE_LIKELY(x) x
#define MATAZURE_UNLIKELY(x) x
#endif
#else
#define MATAZURE_LIKELY(x) x
#define MATAZURE_UNLIKELY(x) x
#endif

#if defined(MATAZURE_DISABLE_ASSERTS)

#define MATAZURE_ASSERT(expr, msg) ((void)0)

#else

namespace matazure {

class assert_failed : public std::runtime_error {
   public:
    assert_failed(const std::string& msg) : std::runtime_error(msg) {}
};

inline void assertion_failed(char const* expr, char const* msg, char const* function,
                             char const* file, long line) {
    throw assert_failed(std::string(msg));
}

}  // namespace matazure

#define MATAZURE_ASSERT(expr, msg)                             \
    (MATAZURE_LIKELY(!!(expr)) ? ((void)0)                     \
                               : ::matazure::assertion_failed( \
                                     #expr, msg, MATAZURE_CURRENT_FUNCTION, __FILE__, __LINE__))

#endif
