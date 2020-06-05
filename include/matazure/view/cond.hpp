#pragma once

#include <matazure/view/map.hpp>

namespace matazure {
namespace view {

template <typename _T1, typename _T2, typename _T3>
struct cond_functor {
    // MATAZURE_GENERAL bool operator()(_Reference v) const { return _Fun(v); }
    _T1 ts1;
    _T2 ts2;

    MATAZURE_GENERAL typename _T2::value_type operator()(pointi<_T1::rank> idx) const {
        if (ts1(idx)) {
            return ts2(idx);
        } else {
            return ts3(idx);
        }
    }
};

template <typename _T1, typename _T2, typename _T3>
inline auto cond(_T1 ts1, _T2 ts2, _T3 ts3)
    -> decltype(make_lambda(ts1.shape(), cond_functor<_T1, _T2, _T3>{ts1, ts2, ts2})) {
    static_assert(_T1::rank == _T2::rank, "the ranks are not matched");
    static_assert(_T1::rank == _T3::rank, "the ranks are not matched");
    static_assert(std::is_same<value_t<_T2>, value_t<_T3>>::value,
                  "the value types are not matched");
    static_assert(std::is_convertible<value_t<_T1>, bool>::value,
                  "the value types are not matched");
    static_assert(std::is_same<value_t<_T2>, value_t<_T3>>::value,
                  "the runtime types are not matched");
    static_assert(std::is_same<runtime_t<_T1>, runtime_t<_T2>>::value,
                  "the runtime types are not matched");
    static_assert(std::is_same<runtime_t<_T1>, runtime_t<_T2>>::value,
                  "the runtime types are not matched");

    MATAZURE_ASSERT(equal(ts1.shape(), ts2.shape()), "the shapes are not matched");
    MATAZURE_ASSERT(equal(ts1.shape(), ts3.shape()), "the shapes are not matched");

    return make_lambda(ts1.shape(), cond_functor<_T1, _T2, _T3>{ts1, ts2, ts3});
}

}  // namespace view
}  // namespace matazure
