#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <stdint.h>

#ifndef DDR
#define DDR(port) (*(&(port) - 1))
#endif

#ifndef PIN
#define PIN(port) (*(&(port) - 2))
#endif

namespace std {
template <bool, class T = void>
struct enable_if {};

template <class T>
struct enable_if<true, T> { typedef T type; };

template <bool B, class T = void>
using enable_if_t = typename enable_if<B, T>::type;

template <class T>
struct remove_reference {
    using type = T;
};

template <class T>
struct remove_reference<T&> {
    using type = T;
};

template <class T>
struct remove_reference<T&&> {
    using type = T;
};

template <class T>
using remove_reference_t = typename remove_reference<T>::type;

template <class T>
constexpr auto move(T&& arg) noexcept {
    return static_cast<remove_reference_t<T>&&>(arg);
}

template <class T>
void swap(T& a, T& b) {
    T tmp = a;
    a = b;
    b = tmp;
}

template <class T>
constexpr T abs(T v) {
    return v < 0 ? -v : v;
}
}

void lcd_hex(uint8_t value);

#endif