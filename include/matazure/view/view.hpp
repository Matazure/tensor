#pragma once

#include <matazure/view/binary.hpp>
#include <matazure/view/broadcast.hpp>
#include <matazure/view/cast.hpp>
#include <matazure/view/clamp_zero.hpp>
#include <matazure/view/cond.hpp>
#include <matazure/view/conv.hpp>
#include <matazure/view/eye.hpp>
#include <matazure/view/gather.hpp>
#include <matazure/view/map.hpp>
#include <matazure/view/mask.hpp>
#include <matazure/view/ones.hpp>
#include <matazure/view/pad.hpp>
#include <matazure/view/permute.hpp>
#include <matazure/view/shift.hpp>
#include <matazure/view/slice.hpp>
#include <matazure/view/stride.hpp>
#include <matazure/view/unary.hpp>
#include <matazure/view/zeros.hpp>
#ifndef MATAZURE_CUDA
#include <matazure/view/meshgrid.hpp>
#include <matazure/view/zip.hpp>
#else
// #ifndef MATAZURE_CUDA
// #include <matazure/view/zip.hpp>
// #endif
#endif
