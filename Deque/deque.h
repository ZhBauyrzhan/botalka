#ifndef DEQUE_H
#define DEQUE_H

#include <cstdio>
#include <type_traits>
namespace bauyr {
template <typename T>
class Deque {
  static_assert((std::is_copy_assignable_v<T> && std::is_copy_constructible_v<T>) &&
                "Deque support only CopyAssignable and CopyConstructible types");
  static const size_t chunck_size{32};

 public:
  template <bool IsPrivate>
  class base_iterator {};
  Deque();
  Deque(const Deque<T>& deque);
  ~Deque();

  private:
};
}  // namespace bauyr
#endif  // end of DEQUE_H