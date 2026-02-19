#include <iostream>
#include <type_traits>

template <typename T>
struct S {
    static_assert(true);
    void f() {std::cout << std::is_copy_assignable_v<T>;}
};

int main() {
    S<int> s;
    s.f();
}