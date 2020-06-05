#pragma once

#include "ut_foundation.hpp"

int_t get_tmp(point2i idx) { return idx[0] + idx[1]; }

TEST(LambdaTensorTests, CFunction) {
    // static_assert(is_prime_function<get_tmp>::value, "");
    // std::function<int_t(point2i)>::argument_type ts = &get_tmp;
    // auto t = (&get_tmp)(point2i{10, 1});
    auto ts_re = make_lambda(point2i{100, 100}, get_tmp);
}
