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
#define SAFEWRAPVALUE(value) hasCallIfFieldIsPresent<decltype(value), value>::call()

#define PP_NARG(...) PP_NARG_(__VA_ARGS__, PP_RSEQ_N())
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N(_1,  \
                 _2,  \
                 _3,  \
                 _4,  \
                 _5,  \
                 _6,  \
                 _7,  \
                 _8,  \
                 _9,  \
                 _10, \
                 _11, \
                 _12, \
                 _13, \
                 _14, \
                 _15, \
                 _16, \
                 _17, \
                 _18, \
                 _19, \
                 _20, \
                 _21, \
                 _22, \
                 _23, \
                 _24, \
                 _25, \
                 _26, \
                 _27, \
                 _28, \
                 _29, \
                 _30, \
                 _31, \
                 _32, \
                 _33, \
                 _34, \
                 _35, \
                 _36, \
                 _37, \
                 _38, \
                 _39, \
                 _40, \
                 _41, \
                 _42, \
                 _43, \
                 _44, \
                 _45, \
                 _46, \
                 _47, \
                 _48, \
                 _49, \
                 _50, \
                 _51, \
                 _52, \
                 _53, \
                 _54, \
                 _55, \
                 _56, \
                 _57, \
                 _58, \
                 _59, \
                 _60, \
                 _61, \
                 _62, \
                 _63, \
                 N,   \
                 ...) \
    N
#define PP_RSEQ_N()                                                                             \
    63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, \
        40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, \
        18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define NEST1(base, field1) SAFEWRAPVALUE(&base::field1)

#define NEST2(base, field1, field2)                                                         \
    makeNvpWithParent(                                                                      \
        SAFEWRAPVALUE(&std::decay_t<typename decltype(NEST1(base, field1))::type>::field2), \
        NEST1(base, field1))

#define NEST3(base, field1, field2, field3)                                               \
    makeNvpWithParent(                                                                    \
        SAFEWRAPVALUE(                                                                    \
            &std::decay_t<typename decltype(NEST2(base, field1, field2))::type>::field3), \
        NEST2(base, field1, field2))

#define PASTE_IMPL(s1, s2) s1##s2
#define PASTE(s1, s2) PASTE_IMPL(s1, s2)

#define NEST(type, ...) PASTE(NEST, PP_NARG(__VA_ARGS__))(type, __VA_ARGS__)

template <typename Base, typename T>
struct Nvp {
    constexpr Nvp(T Base::*t, const char* name) : t(t), name(name) {}

    using type = T;

    friend std::ostream& operator<<(std::ostream& os, const Nvp& nvp) {
        os << nvp.name;
        return os;
    }

    T Base::*t;
    const char* name;
};

template <typename Base, typename T, typename Parent>
struct NvpWithParent : public Nvp<Base, T> {
    constexpr NvpWithParent(T Base::*t, const char* name, Parent parent)
        : Nvp<Base, T>(t, name), parent(parent) {}

    using type = T;

    friend std::ostream& operator<<(std::ostream& os, const NvpWithParent& nvp) {
        os << nvp.parent << "." << nvp.name;
        return os;
    }

    Parent parent;
};

template <typename Nvp>
struct Expr {
    constexpr Expr(const Nvp& nvp, typename Nvp::type field) : nvp(nvp), field(std::move(field)) {}

    const Nvp& nvp;
    typename Nvp::type field;

    friend std::ostream& operator<<(std::ostream& os, const Expr& expr) {
        os << expr.nvp << " == " << expr.field;
        return os;
    }
};

template <typename Nvp,
          typename T,
          typename = typename std::enable_if_t<!std::is_same<T, bool>::value>>
Expr<Nvp> operator==(const Nvp& lhs, const T& rhs) {
    return Expr<Nvp>(lhs, rhs);
}

template <typename Nvp,
          typename = typename std::enable_if_t<std::is_same<typename Nvp::type, bool>::value>>
Expr<Nvp> operator==(const Nvp& lhs, const typename Nvp::type& rhs) {
    return Expr<Nvp>(lhs, rhs);
}

template <typename Base, typename T>
Nvp<Base, T> constexpr makeNvp(T Base::*t, const char* name) {
    return Nvp<Base, T>(t, name);
}

template <typename Base, typename T, typename Parent>
NvpWithParent<Base, T, Parent> constexpr makeNvpWithParent(const Nvp<Base, T>& nvp,
                                                           const Parent& parent) {
    return NvpWithParent<Base, T, Parent>(nvp.t, nvp.name, parent);
}

struct Baz {
    int x;

    ADAPT(Baz, NVP(x))
};

struct Bar {
    Baz baz;

    ADAPT(Bar, NVP(baz))
};

struct Foo {
    Foo* v;
    long w;
    int x;
    bool y;
    std::string z;
    char missing;
    Bar bar;

    ADAPT(Foo, NVP(v), NVP(w), NVP(x), NVP(y), NVP(z), NVP(bar))
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
    std::cout << "Wrap: " << (NEST(Foo, bar, baz, x) == 10) << std::endl;
    //    std::cout << "Wrap: " << SAFEWRAPTYPE(Foo::missing)->name << std::endl;
}
