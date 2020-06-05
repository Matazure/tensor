#pragma once

namespace matazure {

// template <typename _Fun>
// struct is_prime_function {
//     static const bool value = false;
// };

template <typename _ReturnType, typename... _Args>
struct is_prime_function {
    static const bool value = true;
};

/// a type traits to get the argument type and result type of a functor
template <typename _Fun>
struct function_traits;

template <class R, class... Args>
struct function_traits<R (*)(Args...)> : public function_traits<R(Args...)> {};

/// implements
template <typename _ReturnType, typename... _Args>
struct function_traits<_ReturnType(_Args...)> {
    enum { arguments_size = sizeof...(_Args) };

    typedef _ReturnType result_type;

    template <int_t _index>
    struct arguments {
        typedef typename std::tuple_element<_index, std::tuple<_Args...>>::type type;
    };
};

template <typename _Fun>
struct function_traits : public function_traits<decltype(&_Fun::operator())> {};

/// implements
template <typename _ClassType, typename _ReturnType, typename... _Args>
struct function_traits<_ReturnType (_ClassType::*)(_Args...) const> {
    enum { arguments_size = sizeof...(_Args) };

    typedef _ReturnType result_type;

    template <int_t _index>
    struct arguments {
        typedef typename std::tuple_element<_index, std::tuple<_Args...>>::type type;
    };
};

/// implements
template <typename _ClassType, typename _ReturnType, typename... _Args>
struct function_traits<_ReturnType (_ClassType::*)(_Args...)> {
    enum { arguments_size = sizeof...(_Args) };

    typedef _ReturnType result_type;

    template <int_t _index>
    struct arguments {
        typedef typename std::tuple_element<_index, std::tuple<_Args...>>::type type;
    };
};

}  // namespace matazure
