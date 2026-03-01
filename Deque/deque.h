#ifndef DEQUE_H
#define DEQUE_H

#include <cstddef>
#include <cstdio>
#include <initializer_list>
#include <iterator>
#include <type_traits>
namespace bauyr {
template <typename T>
class Deque {
 public:
  using size_type = size_t;
  using reference = T&;
  using const_reference = const T&;
  using pointer = T*;

  template <bool IsConst>
  class base_iterator {};

  using iterator = base_iterator<false>;
  using const_iterator = base_iterator<true>;

  Deque();
  Deque(const Deque&);
  Deque(const int size_, const T& value);
  Deque(std::initializer_list<T>);

  ~Deque();
  Deque& operator=(const Deque&);
  Deque& operator=(std::initializer_list<T>);

  iterator begin() noexcept;
  const_iterator cbegin() const noexcept;
  iterator end() noexcept;
  const_iterator cend() const noexcept;

  bool empty() const noexcept;
  size_type size();
  reference operator[](size_type index);
  const_reference operator[](size_type index) const;
  reference at(size_type index);
  const_reference at(size_type index) const;
  reference front();
  const_reference front() const;
  reference back();
  const_reference backI() const;

  void push_front(const T& x);
  void push_back(const T& x);
  iterator insert(const_iterator position, const T& x);
  void pop_back();
  void pop_front();
  void clear();
};

}  // namespace bauyr
#endif  // end of DEQUE_H