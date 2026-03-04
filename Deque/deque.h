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
  using chunk_type = T*;

  template <bool IsConst>
  class base_iterator {
   public:
    using pointer_to_chunk = chunk_type*;
    using difference_type = std::ptrdiff_t;
    using pointer_to_elem = T*;
    using value_type = T;
    using pointer = std::conditional_t<IsConst, const T*, T*>;
    using reference = std::conditional_t<IsConst, const T&, T&>;
    using iterator_category = std::random_access_iterator_tag;

    base_iterator(pointer_to_chunk chunk_ptr, pointer_to_elem elem_ptr);
    base_iterator& operator++();
    base_iterator& operator+=(difference_type n);
    base_iterator operator++(int);
    bool operator==(const base_iterator& other) const;
    bool operator!=(const base_iterator& other) const;
    reference operator*() const;

   private:
    pointer_to_chunk block_ptr;
    pointer_to_elem current;
  };

  using iterator = base_iterator<false>;
  using const_iterator = base_iterator<true>;
  Deque();
  Deque(const Deque& other);
  Deque(const size_type size_, const T& value);
  Deque(const size_type size_);
  Deque(std::initializer_list<T>);

  ~Deque();
  Deque& operator=(const Deque& other);
  Deque& operator=(std::initializer_list<T>);

  iterator begin() noexcept;
  const_iterator cbegin() const noexcept;
  iterator end() noexcept;
  const_iterator cend() const noexcept;

  bool empty() const noexcept;
  size_type size() const;
  reference operator[](size_type index);
  const_reference operator[](size_type index) const;
  reference at(size_type index);
  const_reference at(size_type index) const;
  reference front();
  const_reference front() const;
  reference back();
  const_reference back() const;

  void push_front(const T& x);
  void push_back(const T& x);
  iterator insert(const_iterator position, const T& x);
  void pop_back();
  void pop_front();
  void clear();

  // TODO: Remove print blocks ???
  void print_blocks();

 private:
  static const size_type CHUNK_SIZE{32};
  static const size_type increase_coefficient{3};
  size_type size_;
  size_type number_of_blocks;
  chunk_type* blocks;
  size_type first_block_index;
  size_type first_elem_offset;
  size_type last_block_index;
  size_type last_elem_offset;

  Deque(size_type size_, size_type number_of_blocks);
  void reallocate_blocks(size_type new_number_of_blocks);
  void swap(Deque<T>& other);
  std::pair<size_type, size_type> next_position(size_type cur_block_index,
                                                size_type cur_elem_offset);
  std::pair<size_type, size_type> previous_position(size_type cur_block_index,
                                                    size_type cur_elem_offset);

  chunk_type allocate_new_block();
};

}  // namespace bauyr
#endif  // end of DEQUE_H