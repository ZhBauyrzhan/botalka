#include "deque.h"
namespace bauyr {
template <typename T> Deque<T>::Deque()  {}
template <typename T> Deque<T>::~Deque()  {}
}
int main() {
    bauyr::Deque<int> d;
}