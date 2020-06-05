#pragma once
#include <matazure/algorithm.hpp>
#include <matazure/lambda_tensor.hpp>
#include <matazure/tensor.hpp>

#ifdef MATAZURE_CUDA
#include <matazure/cuda/tensor.hpp>
#endif

namespace matazure {
namespace view {

template <typename _Tensor, typename _Kernel, bool _IsLocalTensor>
struct conv_functor;

template <typename _Tensor, typename _Kernel>
struct conv_functor<_Tensor, _Kernel, false> {
   private:
    typedef typename _Tensor::value_type value_type;
    static const int_t rank = _Tensor::rank;

    _Tensor ts_;
    _Kernel kernel_;
    pointi<rank> kernel_shape_;

   public:
    conv_functor(_Tensor ts, _Kernel kernel)
        : ts_(ts), kernel_(kernel), kernel_shape_(kernel.shape()) {}

    MATAZURE_GENERAL value_type operator()(pointi<_Tensor::rank> idx) const {
        auto re = matazure::zero<value_type>::value();
        for_index(kernel_shape_, [&](pointi<rank> neigbor_idx) {
            re += kernel_(neigbor_idx) * ts_(idx + neigbor_idx - kernel_shape_ / 2);
        });

        return re;
    }
};

template <typename _Tensor, typename _Kernel>
struct conv_functor<_Tensor, _Kernel, true> {
   private:
    typedef typename _Tensor::value_type value_type;
    static const int_t rank = _Tensor::rank;

    const _Tensor ts_;
    const _Kernel kernel_;
    const pointi<rank> kernel_shape_;
    const pointi<rank> kernel_radius_;

   public:
    conv_functor(_Tensor ts, _Kernel kernel)
        : ts_(ts),
          kernel_(kernel),
          kernel_shape_(kernel.shape()),
          kernel_radius_(kernel.shape() / 2) {}

    MATAZURE_GENERAL value_type operator()(pointi<_Tensor::rank> idx) const {
        auto re = matazure::zero<value_type>::value();
        for_index(kernel_shape_, [&](pointi<rank> neigbor_idx) {
            re += kernel_(neigbor_idx) * ts_(idx + neigbor_idx - kernel_radius_);
        });

        return re;
    }
};

// add 3x3 unroll special,
template <typename _Tensor, typename _ValueType, typename _Layout>
struct conv_functor<_Tensor, local_tensor<_ValueType, dim<3, 3>, _Layout>, true> {
   private:
    typedef typename _Tensor::value_type value_type;
    static const int_t rank = _Tensor::rank;

    typedef local_tensor<_ValueType, dim<3, 3>, _Layout> kernel_type;

    _Tensor ts_;
    kernel_type kernel_;
    pointi<rank> kernel_shape_;
    pointi<rank> kernel_radius_;

   public:
    conv_functor(_Tensor ts, kernel_type kernel)
        : ts_(ts),
          kernel_(kernel),
          kernel_shape_(kernel.shape()),
          kernel_radius_(kernel.shape() / 2) {}

    MATAZURE_GENERAL value_type operator()(pointi<_Tensor::rank> idx) const {
        // clang-format off
        auto re = zero<value_type>::value();
        re += kernel_(pointi<2>{0, 0}) * ts_(idx + pointi<2>{-1, -1});
        re += kernel_(pointi<2>{1, 0}) * ts_(idx + pointi<2>{ 0, -1});
        re += kernel_(pointi<2>{2, 0}) * ts_(idx + pointi<2>{ 1, -1});
        re += kernel_(pointi<2>{0, 1}) * ts_(idx + pointi<2>{-1,  0});
        re += kernel_(pointi<2>{1, 1}) * ts_(idx + pointi<2>{ 0,  0});
        re += kernel_(pointi<2>{2, 1}) * ts_(idx + pointi<2>{ 1,  0});
        re += kernel_(pointi<2>{0, 2}) * ts_(idx + pointi<2>{-1,  1});
        re += kernel_(pointi<2>{1, 2}) * ts_(idx + pointi<2>{ 0,  1});
        re += kernel_(pointi<2>{2, 2}) * ts_(idx + pointi<2>{ 1,  1});
        // clang-format on

        return re;
    }
};

template <typename _Tensor>
struct conv_neighbors_weights_functor {
   private:
    typedef typename _Tensor::value_type value_type;
    static const int_t rank = _Tensor::rank;
    typedef tensor<tuple<pointi<_Tensor::rank>, typename _Tensor::value_type>, 1> weights_type;

    _Tensor ts_;
    weights_type neighbors_weights_;

   public:
    conv_neighbors_weights_functor(_Tensor ts, weights_type neighbors_weights)
        : ts_(ts), neighbors_weights_(neighbors_weights) {}

    MATAZURE_GENERAL value_type operator()(pointi<_Tensor::rank> idx) const {
        auto re = matazure::zero<value_type>::value();
        for (int_t i = 0; i < neighbors_weights_.size(); ++i) {
            auto nw = neighbors_weights_[i];
            auto local_idx = get<0>(nw);
            auto weights = get<1>(nw);
            re += ts_(idx + local_idx) * weights;
        }

        return re;
    }
};

template <typename _Tensor, typename _Kernel>
inline auto conv(_Tensor ts, _Kernel kernel) -> decltype(make_lambda(
    ts.shape(), conv_functor<_Tensor, _Kernel, is_local_tensor<_Kernel>::value>(ts, kernel),
    runtime_t<_Tensor>{}, layout_t<_Tensor>{})) {
    static_assert(is_same<typename _Tensor::value_type, typename _Kernel::value_type>::value,
                  "the value types are not matched");
    return make_lambda(ts.shape(),
                       conv_functor<_Tensor, _Kernel, is_local_tensor<_Kernel>::value>(ts, kernel),
                       runtime_t<_Tensor>{}, layout_t<_Tensor>{});
}

template <typename _Tensor>
[[deprecated]] inline auto conv(
    _Tensor ts,
    tensor<tuple<pointi<_Tensor::rank>, typename _Tensor::value_type>, 1> neighbors_weights)
    -> decltype(make_lambda(ts.shape(),
                            conv_neighbors_weights_functor<_Tensor>(ts, neighbors_weights),
                            runtime_t<_Tensor>{}, layout_t<_Tensor>{})) {
    return make_lambda(ts.shape(), conv_neighbors_weights_functor<_Tensor>(ts, neighbors_weights),
                       runtime_t<_Tensor>{}, layout_t<_Tensor>{});
}

}  // namespace view
}  // namespace matazure
