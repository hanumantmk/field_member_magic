#include <iostream>
#include <vector>
#include <tuple>

class Foo {
public:
    char a = 1;
    short b = 2;
    int c = 3;
};

template <typename T>
struct Widget {
};

template <typename... Ts>
struct Widget<std::tuple<Ts...>> {
    template <typename CB, typename T, size_t... idxs>
    static void callImpl(CB&& cb, T* thisv, const std::tuple<Ts...>& tup, std::index_sequence<idxs...>) {
        cb((*thisv).*std::get<idxs>(tup)...);
    }

    template <typename CB, typename T, typename Idxs = std::make_index_sequence<sizeof...(Ts)>>
    static void call(CB&& cb, T* thisv, const std::tuple<Ts...>& tup) {
        return callImpl(std::forward<CB>(cb), thisv, tup, Idxs());
    }
};

void pp(char a, short b, int c) {
    std::cout << "a: " << a << "\n";
    std::cout << "b: " << b << "\n";
    std::cout << "c: " << c << "\n";
}

int main() {
    std::tuple<char Foo::*, short Foo::*, int Foo::*> tup(&Foo::a, &Foo::b, &Foo::c);

    Foo foo;

    Widget<decltype(tup)>::call(pp, &foo, tup);
}
