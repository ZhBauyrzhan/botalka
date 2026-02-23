#ifndef DEQUE_H
#define DEQUE_H

#include <cstddef>
#include <cstdio>
#include <type_traits>
namespace bauyr {
template <typename T>
class Deque {
  using value_type = T;
  using size_type = size_t;
  using reference_type = value_type&;
  using pointer_type = value_type*;

  static_assert((std::is_copy_assignable_v<T> && std::is_copy_constructible_v<T>) &&
                "Deque support only CopyAssignable and CopyConstructible types");
  static const size_type chunck_size{32};

 private:
  Deque(const size_type size_, size_type chunk_number);

 public:
  // Iterators
  template <bool IsPrivate>
  class base_iterator {};

  // Constructors
  Deque();
  Deque(const size_type size_, const T& value);
  Deque(const Deque<T>& deque);
  ~Deque();

  // Operators
  Deque& operator=(const Deque& other);

  // Methods
  T& at(size_type pos);
  const T& at(size_type pos) const;

  size_type size() const;
  
  T& operator[](size_type pos);
  T& operator[](size_type pos) const;

 private:
  const size_type CHUNK_SIZE = 32;
  size_t size_;
  size_type chunk_number;
  T** blocks;
  T* first_chunk;
  T* last_chunk;

  bool check_range(size_t pos);
};
}  // namespace bauyr
#endif  // end of DEQUE_H