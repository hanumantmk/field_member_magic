#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <type_traits>

#define NVP(x) makeNvp(&wrapBase::x, #x)

#define ADAPT(Base, ...)                     \
    using wrapBase = Base;                   \
    constexpr static auto fields() {         \
        return std::make_tuple(__VA_ARGS__); \
    }

#define SAFEWRAP(name, member) \
    hasCallIfFieldIsPresent<decltype(&name::member), &name::member>::call()

#define SAFEWRAPTYPE(value) hasCallIfFieldIsPresent<decltype(&value), &value>::call()

template <typename Base, typename T>
struct Nvp {
    constexpr Nvp(T Base::*t, const char* name) : t(t), name(name) {}

    T Base::*t;
    const char* name;
};

template <typename Base, typename T>
struct Expr {
    constexpr Expr(const Nvp<Base, T>& nvp, T field) : nvp(nvp), field(std::move(field)) {}

    const Nvp<Base, T>& nvp;
    T field;

    friend std::ostream& operator<<(std::ostream& os, const Expr& expr) {
        os << expr.nvp.name << " == " << expr.field;
        return os;
    }
};

template <typename Base,
          typename T,
          typename U,
          typename = typename std::enable_if_t<!std::is_same<T, bool>::value>>
Expr<Base, T> operator==(const Nvp<Base, T>& lhs, const U& rhs) {
    return Expr<Base, T>(lhs, rhs);
}

template <typename Base,
          typename T,
          typename = typename std::enable_if_t<std::is_same<T, bool>::value>>
Expr<Base, T> operator==(const Nvp<Base, T>& lhs, const T& rhs) {
    return Expr<Base, T>(lhs, rhs);
}

template <typename Base, typename T>
Nvp<Base, T> constexpr makeNvp(T Base::*t, const char* name) {
    return Nvp<Base, T>(t, name);
}

struct Foo {
    Foo* v;
    long w;
    int x;
    bool y;
    std::string z;
    char missing;

    ADAPT(Foo, NVP(v), NVP(w), NVP(x), NVP(y), NVP(z))
};

template <typename Base,
          typename T,
          size_t N,
          size_t M,
          bool = N<M> struct hasField : public std::false_type {};

template <typename Base, typename T, size_t N, size_t M>
struct hasField<Base, T, N, M, true>
    : public std::is_same<T Base::*, decltype(std::get<N>(Base::fields()).t)> {};

template <typename Base, typename T, size_t N, size_t M>
    constexpr std::enable_if_t < N<M && hasField<Base, T, N, M>::value, bool> wrapbool(T Base::*t) {
    if (std::get<N>(Base::fields()).t == t) {
        return true;
    } else {
        return wrapbool<Base, T, N + 1, M>(t);
    }
}

template <typename Base, typename T, size_t N, size_t M>
    constexpr std::enable_if_t <
    N<M && !hasField<Base, T, N, M>::value, bool> wrapbool(T Base::*t) {
    return wrapbool<Base, T, N + 1, M>(t);
}

template <typename Base, typename T, size_t N, size_t M>
constexpr std::enable_if_t<N == M, bool> wrapbool(T Base::*t) {
    return false;
}

template <typename Base, typename T, size_t N, size_t M>
    constexpr std::enable_if_t <
    N<M && hasField<Base, T, N, M>::value, const Nvp<Base, T>> wrapimpl(T Base::*t) {
    if (std::get<N>(Base::fields()).t == t) {
        return std::get<N>(Base::fields());
    } else {
        return wrapimpl<Base, T, N + 1, M>(t);
    }
}

template <typename Base, typename T, size_t N, size_t M>
    constexpr std::enable_if_t <
    N<M && !hasField<Base, T, N, M>::value, const Nvp<Base, T>> wrapimpl(T Base::*t) {
    return wrapimpl<Base, T, N + 1, M>(t);
}

template <typename Base, typename T, size_t N, size_t M>
constexpr std::enable_if_t<N == M, const Nvp<Base, T>> wrapimpl(T Base::*t) {
    return Nvp<Base, T>(nullptr, nullptr);
}

template <typename Base, typename T>
constexpr const Nvp<Base, T> wrap(T Base::*t) {
    return wrapimpl<Base, T, 0, std::tuple_size<decltype(Base::fields())>::value>(t);
}

template <typename T, T, typename = void>
struct hasCallIfFieldIsPresent {};

template <typename Base, typename T, T Base::*ptr>
struct hasCallIfFieldIsPresent<
    T Base::*,
    ptr,
    std::enable_if_t<wrapbool<Base, T, 0, std::tuple_size<decltype(Base::fields())>::value>(ptr)>> {
    static constexpr const Nvp<Base, T> call() {
        return wrap(ptr);
    }
};

int main() {
    std::cout << "Wrap: " << wrap(&Foo::v).name << std::endl;
    std::cout << "Wrap: " << SAFEWRAP(Foo, w).name << std::endl;
    std::cout << "Wrap: " << SAFEWRAPTYPE(Foo::x).name << std::endl;
    std::cout << "Wrap: " << (SAFEWRAPTYPE(Foo::y) == true) << std::endl;
    //    std::cout << "Wrap: " << (SAFEWRAPTYPE(Foo::y) == "hello") << std::endl;
    std::cout << "Wrap: " << (SAFEWRAPTYPE(Foo::z) == "hello") << std::endl;
    //    std::cout << "Wrap: " << SAFEWRAPTYPE(Foo::missing)->name << std::endl;
}
