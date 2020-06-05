#pragma once

#include <matazure/view/map.hpp>

namespace matazure {
namespace view {

template <typename _T1, typename _T2>
struct mask_value {
    _T1 ts1;
    _T2 ts2;
    pointi<_T1::rank> idx;

    template <typename _V>
    MATAZURE_GENERAL mask_value& operator=(const _V& v) {
        if (ts2(idx)) {
            ts1(idx) = v;
        }

        return *this;
    }

    typename _T1::value_type opeartor() const { return ts1(idx); }
};

template <typename _T1, typename _T2>
struct mask_functor {
    // MATAZURE_GENERAL bool operator()(_Reference v) const { return _Fun(v); }
    _T1 ts1;
    _T2 ts2;

    MATAZURE_GENERAL mask_value<_T1, _T2> operator()(pointi<_T1::rank> idx) const {
        return mask_value<_T1, _T2>{ts1, ts2, idx};
    }
};

template <typename _T1, typename _T2>
inline auto mask(_T1 ts1, _T2 ts2)
    -> decltype(make_lambda(ts1.shape(), mask_functor<_T1, _T2>{ts1, ts2})) {
    static_assert(_T1::rank == _T2::rank, "the ranks are not matched");
    static_assert(std::is_same<runtime_t<_T1>, runtime_t<_T2>>::value,
                  "the runtime types are not matched");
    MATAZURE_ASSERT(equal(ts1.shape(), ts2.shape()), "the shapes are not matched");

    return make_lambda(ts1.shape(), mask_functor<_T1, _T2>{ts1, ts2});
}

}  // namespace view
}  // namespace matazure
