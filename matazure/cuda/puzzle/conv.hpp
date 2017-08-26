﻿#pragma once

#include <matazure/cuda/tensor.hpp>

namespace matazure {
namespace cuda {

namespace puzzle {

template <typename _Func>
inline __device__ void corner_index(pointi<1> origin, pointi<1> extent, _Func func) {
	func(origin);
	func(extent-1);
}

template <typename _Func>
inline __device__ void corner_index(pointi<2> origin, pointi<2> extent, _Func func) {
	func(pointi<2>{origin[0], origin[1]});
	func(pointi<2>{origin[0], extent[1]-1});
	func(pointi<2>{extent[0]-1, origin[1]});
	func(pointi<2>{extent[0]-1, extent[1]-1});
}

template <typename _Func>
inline __device__ void corner_index(pointi<3> origin, pointi<3> extent, _Func func) {
	func(pointi<3>{origin[0], origin[1], origin[2]});
	func(pointi<3>{origin[0], origin[1], extent[2]-1});
	func(pointi<3>{origin[0], extent[1]-1, origin[2]});
	func(pointi<3>{origin[0], extent[1]-1, extent[2]-1});
	func(pointi<3>{extent[0]-1, origin[1], origin[2]});
	func(pointi<3>{extent[0]-1, origin[1], extent[2]-1});
	func(pointi<3>{extent[0]-1, extent[1]-1, origin[2]});
	func(pointi<3>{extent[0]-1, extent[1]-1, extent[2]-1});
}

template <typename _Func>
inline __device__ void corner_index(pointi<4> origin, pointi<4> extent, _Func func) {
	func(pointi<4>{origin[0], origin[1], origin[2], origin[3]});
	func(pointi<4>{origin[0], origin[1], origin[2], extent[3]-1});
	func(pointi<4>{origin[0], origin[1], extent[2]-1, origin[3]});
	func(pointi<4>{origin[0], origin[1], extent[2]-1, extent[3]-1});
	func(pointi<4>{origin[0], extent[1]-1, origin[2], origin[3]});
	func(pointi<4>{origin[0], extent[1]-1, origin[2], extent[3]-1});
	func(pointi<4>{origin[0], extent[1]-1, extent[2]-1, origin[3]});
	func(pointi<4>{origin[0], extent[1]-1, extent[2]-1, extent[3]-1});
	func(pointi<4>{extent[0]-1, origin[1], origin[2], origin[3]});
	func(pointi<4>{extent[0]-1, origin[1], origin[2], extent[3]-1});
	func(pointi<4>{extent[0]-1, origin[1], extent[2]-1, origin[3]});
	func(pointi<4>{extent[0]-1, origin[1], extent[2]-1, extent[3]-1});
	func(pointi<4>{extent[0]-1, extent[1]-1, origin[2], origin[3]});
	func(pointi<4>{extent[0]-1, extent[1]-1, origin[2], extent[3]-1});
	func(pointi<4>{extent[0]-1, extent[1]-1, extent[2]-1, origin[3]});
	func(pointi<4>{extent[0]-1, extent[1]-1, extent[2]-1, extent[3]-1});
}

} //puzzle

} //cuda
} //matazure

#define MATAZURE_CUDA_PUZZEL_CONV_GLOBAL(conv_global, mask)													\
namespace matazure{namespace cuda{ namespace puzzle {														\
																											\
namespace internal {																						\
																											\
template <typename _Tensor>																					\
struct conv_op {																							\
private:																									\
	_Tensor ts_;																							\
public:																										\
	conv_op(_Tensor ts) :																					\
		ts_(ts)																								\
	{}																										\
																											\
	MATAZURE_GENERAL typename _Tensor::value_type operator()(const pointi<_Tensor::rank> &idx) const {		\
		auto mask_radius = mask.shape() / 2;																\
		auto sum = zero<typename _Tensor::value_type>::value();												\
		device::for_index(pointi<_Tensor::rank>::zeros(), mask.shape(), [&] (const pointi<2> &mask_idx) {	\
			sum += ts_[idx + mask_idx - mask_radius] * mask[mask_idx];										\
		});																									\
		return sum;																							\
	}																										\
};																											\
																											\
}																											\
																											\
template <typename _Tensor>																					\
inline auto conv_global(_Tensor ts)																			\
->decltype(make_lambda(ts.shape(), internal::conv_op<decay_t<_Tensor>>(ts), typename _Tensor::memory_type{})) {		\
	return make_lambda(ts.shape(), internal::conv_op<decay_t<_Tensor>>(ts), typename _Tensor::memory_type{});		\
}																											\
																											\
}}} //matazure/cuda/puzzle


#define MATAZURE_CUDA_PUZZEL_CONV_BLOCK(conv_block, mask)													\
namespace matazure { namespace cuda{ namespace puzzle{														\
																											\
template <typename _BlockDim, typename _Tensor, typename _TensorRe>											\
inline void conv_block(_Tensor ts, _TensorRe &ts_re) {														\
	MATAZURE_STATIC_ASSERT_DIM_MATCHED(_Tensor, decltype(mask));											\
	MATAZURE_STATIC_ASSERT_VALUE_TYPE_MATCHED(_Tensor, decltype(mask));										\
	typedef typename _Tensor::value_type value_type;														\
																											\
	constexpr auto block_ext = _BlockDim::value();															\
	auto grid_ext = (ts.shape() + block_ext - 1) / block_ext;												\
	MATAZURE_ASSERT(equal(ts.shape(), ts_re.shape()), "unmatched shape");									\
	auto shape_le = mask.shape() <= _BlockDim::value();														\
	for(int_t i = 0; i <shape_le.size(); ++i){																\
		MATAZURE_ASSERT(shape_le[i], "block dim could not be less than mask shape");						\
	}																										\
																											\
	block_for_index<_BlockDim>(grid_ext, [=] __device__ (block_index<_BlockDim> block_idx) {				\
		auto tmp_shape = meta::sub_c(																		\
			meta::add_c(_BlockDim{}, decltype(mask)::meta_shape()), meta::int_t_c<1>{});					\
		__shared__ static_tensor<value_type, decltype(tmp_shape)> sh_ts_block;								\
																											\
		auto is_valid = inside(block_idx.global, pointi<_Tensor::rank>::zeros(), ts.shape());				\
		if (is_valid) {																						\
			corner_index(pointi<_Tensor::rank>::zeros(), mask.shape(),										\
				[&](pointi<_Tensor::rank> corner_idx) {														\
				sh_ts_block[block_idx.local + corner_idx] = ts[block_idx.global - corner_idx / 2];			\
			}																								\
			);																								\
		}																									\
		device::barrier();																					\
																											\
		if (!is_valid) return;																				\
		auto sum = zero<value_type>::value();																\
		device::for_index(pointi<_Tensor::rank>::zeros(), mask.shape(), [&](const pointi<2> &idx) {			\
			sum += sh_ts_block[block_idx.local + idx] * mask[idx];											\
		});																									\
		ts_re[block_idx.global] = sum;																		\
	});																										\
}																											\
																											\
}}}  //end conv_block

#define MATAZURE_CUDA_PUZZEL_CONV_BLOCK_CRACK(conv_block_crack, mask)										\
namespace matazure{namespace cuda{namespace puzzle{															\
																											\
template <typename _BlockDim, typename _Tensor, typename _TensorRe>											\
inline void conv_block_crack(_Tensor ts, _TensorRe &ts_re) {												\
	MATAZURE_STATIC_ASSERT_DIM_MATCHED(_Tensor, decltype(mask));											\
	typedef typename _Tensor::value_type value_type;														\
																											\
	constexpr auto block_ext = _BlockDim::value();															\
	auto grid_ext = (ts.shape() + block_ext - 1) / block_ext;												\
	MATAZURE_ASSERT(equal(ts.shape(), ts_re.shape()), "unmatched shape");									\
	auto shape_le = mask.shape() <= _BlockDim::value();														\
	for(int_t i = 0; i <shape_le.size(); ++i){																\
		MATAZURE_ASSERT(shape_le[i], "block dim could not be less than mask shape");						\
	}																										\
																											\
	block_for_index<_BlockDim>(grid_ext, [=] __device__ (block_index<_BlockDim> block_idx) {				\
		__shared__ static_tensor<value_type, _BlockDim> sh_ts_block;										\
																											\
		auto is_valid = inside(block_idx.global, pointi<_Tensor::rank>::zeros(), ts.shape());				\
		if (is_valid) {																						\
			sh_ts_block[block_idx.local] = ts[block_idx.global];											\
		}																									\
		device::barrier();																					\
																											\
		auto mask_radius = mask.shape() / 2;																\
		if (inside(block_idx.local, mask_radius, block_idx.block_dim - mask_radius) 						\
			&& inside(block_idx.global, mask_radius, ts.shape() - mask_radius)) {							\
			auto sum = zero<value_type>::value();															\
			device::for_index(pointi<_Tensor::rank>::zeros(), mask.shape(), [&](const pointi<2> &idx) {		\
				sum += sh_ts_block[block_idx.local + idx - mask_radius] * mask[idx];						\
			});																								\
			ts_re[block_idx.global] = sum;																	\
		}																									\
	});																										\
}																											\
																											\
}}}	 //end conv_block_crack
																											\
#define MATAZURE_CUDA_PUZZEL_CONV_BLOCK_OVERLAP(conv_block_overlap, mask)									\
namespace matazure { namespace cuda { namespace puzzle {													\
																											\
template <typename _BlockDim, typename _Tensor, typename _TensorRe>											\
inline void conv_block_overlap(_Tensor ts, _TensorRe &ts_re) {												\
	MATAZURE_STATIC_ASSERT_DIM_MATCHED(_Tensor, decltype(mask));											\
	typedef typename _Tensor::value_type value_type;														\
																											\
	auto valid_block_dim = meta::array_to_pointi(															\
		meta::add_c(meta::sub_c(_BlockDim{}, decltype(mask)::meta_shape_type{}), meta::int_t_c<1>{})		\
	);																										\
	auto grid_ext = (ts.shape() + valid_block_dim - 1) / valid_block_dim;									\
	MATAZURE_ASSERT(equal(ts.shape(), ts_re.shape()), "unmatched shape");									\
	auto shape_le = mask.shape() <= _BlockDim::value();														\
	for(int_t i = 0; i <shape_le.size(); ++i){																\
		MATAZURE_ASSERT(shape_le[i], "block dim could not be less than mask shape");						\
	}																										\
																											\
	block_for_index<_BlockDim>(grid_ext, [=] __device__(block_index<_BlockDim> block_idx) {					\
		__shared__ static_tensor<value_type, _BlockDim> sh_ts_block;										\
		auto valid_block_dim = meta::array_to_pointi(														\
			meta::add_c(meta::sub_c(_BlockDim{}, decltype(mask)::meta_shape()), meta::int_t_c<1>{})			\
		);																									\
		auto mask_radius = mask.shape() / 2;																\
		auto valid_global_idx = valid_block_dim * block_idx.block + block_idx.local - mask_radius;			\
																											\
		if (inside(valid_global_idx, -mask_radius, ts.shape() + mask_radius)) {								\
			sh_ts_block[block_idx.local] = ts[valid_global_idx];											\
		}																									\
		device::barrier();																					\
																											\
		if (inside(block_idx.local, mask_radius, block_idx.block_dim - mask_radius)							\
			&& inside(valid_global_idx, pointi<_Tensor::rank>::zeros(), ts.shape())) {						\
			auto sum = zero<value_type>::value();															\
			device::for_index(pointi<_Tensor::rank>::zeros(), mask.shape(), [&](const pointi<2> &idx) {		\
				sum += sh_ts_block[block_idx.local + idx - mask_radius] * mask[idx];						\
			});																								\
			ts_re[valid_global_idx] = sum;																	\
		}																									\
	});																										\
}																											\
																											\
}}}	 // end conv_block_overlap


#define MATAZURE_CUDA_PUZZEL_CONV_BLOCK_ALIGNED(conv_block_aligned, mask)									\
namespace matazure { namespace cuda{ namespace puzzle{														\
																											\
template <typename _BlockDim, typename _Tensor, typename _TensorRe>											\
inline void conv_block_aligned(_Tensor ts, _TensorRe &ts_re) {												\
	MATAZURE_STATIC_ASSERT_DIM_MATCHED(_Tensor, decltype(mask));											\
	MATAZURE_STATIC_ASSERT_VALUE_TYPE_MATCHED(_Tensor, decltype(mask));										\
	typedef typename _Tensor::value_type value_type;														\
																											\
	constexpr auto block_ext = _BlockDim::value();															\
	auto grid_ext = ts.shape() / block_ext;																	\
	MATAZURE_ASSERT(equal(grid_ext * block_ext, ts.shape()), "unaligned shape");							\
	MATAZURE_ASSERT(equal(ts.shape(), ts_re.shape()), "unmatched shape");									\
	auto shape_le = mask.shape() <= _BlockDim::value();														\
	for(int_t i = 0; i <shape_le.size(); ++i){																\
		MATAZURE_ASSERT(shape_le[i], "block dim should be greater than mask shape");						\
	}																										\
																											\
	block_for_index<_BlockDim>(grid_ext, [=] __device__ (block_index<_BlockDim> block_idx) {				\
		auto tmp_shape = 																					\
			meta::sub_c(meta::add_c(_BlockDim{}, decltype(mask)::meta_shape()), meta::int_t_c<1>{});		\
		__shared__ static_tensor<value_type, decltype(tmp_shape)> sh_ts_block;								\
																											\
		corner_index(pointi<_Tensor::rank>::zeros(), mask.shape(),											\
			[&](pointi<_Tensor::rank> corner_idx){															\
				sh_ts_block[block_idx.local + corner_idx] = ts[block_idx.global - corner_idx / 2];			\
			}																								\
		);																									\
		device::barrier();																					\
																											\
		auto sum = zero<value_type>::value();																\
		device::for_index(pointi<_Tensor::rank>::zeros(), mask.shape(), [&](const pointi<2> &idx) {			\
			sum += sh_ts_block[block_idx.local + idx] * mask[idx];											\
		});																									\
		ts_re[block_idx.global] = sum;																		\
	});																										\
}																											\
																											\
}}}  //end conv_block

#define MATAZURE_CUDA_PUZZEL_CONV_BLOCK_CRACK_ALIGNED(conv_block_crack_aligned, mask)						\
namespace matazure{namespace cuda{namespace puzzle{															\
																											\
template <typename _BlockDim, typename _Tensor, typename _TensorRe>											\
inline void conv_block_crack_aligned(_Tensor ts, _TensorRe &ts_re) {										\
	MATAZURE_STATIC_ASSERT_DIM_MATCHED(_Tensor, decltype(mask));											\
	typedef typename _Tensor::value_type value_type;														\
																											\
	constexpr auto block_ext = _BlockDim::value();															\
	auto grid_ext = ts.shape() / block_ext;																	\
	MATAZURE_ASSERT(equal(grid_ext * block_ext, ts.shape()), "unaligned shape");							\
	MATAZURE_ASSERT(equal(ts.shape(), ts_re.shape()), "unmatched shape");									\
	auto shape_le = mask.shape() <= _BlockDim::value();														\
	for(int_t i = 0; i <shape_le.size(); ++i){																\
		MATAZURE_ASSERT(shape_le[i], "block dim could not be less than mask shape");						\
	}																										\
																											\
	block_for_index<_BlockDim>(grid_ext, [=] __device__ (block_index<_BlockDim> block_idx) {				\
		__shared__ static_tensor<value_type, _BlockDim> sh_ts_block;										\
		sh_ts_block[block_idx.local] = ts[block_idx.global];												\
		device::barrier();																					\
																											\
		auto mask_radius = mask.shape() / 2;																\
		if (inside(block_idx.local, mask_radius, block_idx.block_dim - mask_radius)) {						\
			auto sum = zero<value_type>::value();															\
			device::for_index(pointi<_Tensor::rank>::zeros(), mask.shape(), [&](const pointi<2> &idx) {		\
				sum += sh_ts_block[block_idx.local + idx - mask_radius] * mask[idx];						\
			});																								\
			ts_re[block_idx.global] = sum;																	\
		}																									\
	});																										\
}																											\
																											\
}}}	 //end conv_block_crack

#define MATAZURE_CUDA_PUZZEL_CONV_BLOCK_OVERLAP_ALIGNED(conv_block_overlap_aligned, mask)					\
namespace matazure { namespace cuda { namespace puzzle {													\
																											\
template <typename _BlockDim, typename _Tensor, typename _TensorRe>											\
inline void conv_block_overlap_aligned(_Tensor ts, _TensorRe &ts_re) {										\
	MATAZURE_STATIC_ASSERT_DIM_MATCHED(_Tensor, decltype(mask));											\
	typedef typename _Tensor::value_type value_type;														\
																											\
	auto valid_block_dim = meta::array_to_pointi(															\
		meta::add_c(meta::sub_c(_BlockDim{}, decltype(mask)::meta_shape_type{}), meta::int_t_c<1>{})		\
	);																										\
	auto grid_ext = ts.shape() / valid_block_dim;															\
	MATAZURE_ASSERT(equal(grid_ext * valid_block_dim, ts.shape()), "unaligned shape");						\
	MATAZURE_ASSERT(equal(ts.shape(), ts_re.shape()), "unmatched shape");									\
	auto shape_le = mask.shape() <= _BlockDim::value();														\
	for(int_t i = 0; i <shape_le.size(); ++i){																\
		MATAZURE_ASSERT(shape_le[i], "block dim could not be less than mask shape");						\
	}																										\
																											\
	block_for_index<_BlockDim>(grid_ext, [=] __device__ (block_index<_BlockDim> block_idx) {				\
		__shared__ static_tensor<value_type, _BlockDim> sh_ts_block;										\
		auto valid_block_dim = meta::array_to_pointi(														\
			meta::add_c(meta::sub_c(_BlockDim{}, decltype(mask)::meta_shape()), meta::int_t_c<1>{})			\
		);																									\
		auto mask_radius = mask.shape() / 2;																\
		auto valid_global_idx = valid_block_dim * block_idx.block + block_idx.local - mask_radius;			\
		sh_ts_block[block_idx.local] = ts[valid_global_idx];												\
		device::barrier();																					\
																											\
		if (inside(block_idx.local, mask_radius, block_idx.block_dim - mask_radius)) {						\
			auto sum = zero<value_type>::value();															\
			device::for_index(pointi<_Tensor::rank>::zeros(), mask.shape(), [&](const pointi<2> &idx) {		\
				sum += sh_ts_block[block_idx.local + idx - mask_radius] * mask[idx];						\
			});																								\
			ts_re[valid_global_idx] = sum;																	\
		}																									\
	});																										\
}																											\
																											\
}}}	 // end conv_block_overlap
