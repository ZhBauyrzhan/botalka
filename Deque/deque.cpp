#include "deque.h"

#include <cassert>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <type_traits>

namespace bauyr {
template <typename T>
template <bool IsConst>
Deque<T>::base_iterator<IsConst>::base_iterator(typename base_iterator::pointer_to_chunk block_ptr,
                                                typename base_iterator::pointer_to_elem elem)
    : block_ptr(block_ptr), current(elem) {}

template <typename T>
template <bool IsConst>
Deque<T>::base_iterator<IsConst>& Deque<T>::base_iterator<IsConst>::operator++() {
  ++current;
  if (current == *block_ptr + CHUNK_SIZE) {
    ++block_ptr;
    current = *block_ptr;
  }
  return *this;
}

template <typename T>
template <bool IsConst>
Deque<T>::base_iterator<IsConst> Deque<T>::base_iterator<IsConst>::operator++(int) {
  base_iterator<IsConst> tmp = *this;
  ++(*this);
  return tmp;
}

template <typename T>
template <bool IsConst>
Deque<T>::base_iterator<IsConst>::reference Deque<T>::base_iterator<IsConst>::operator*() const {
  return *current;
}

template <typename T>
template <bool IsConst>
bool Deque<T>::base_iterator<IsConst>::operator==(const base_iterator& other) const {
  return current == other.current;
}

template <typename T>
template <bool IsConst>
bool Deque<T>::base_iterator<IsConst>::operator!=(const base_iterator& other) const {
  return !(*this == other);
}

template <typename T>
template <bool IsConst>
Deque<T>::base_iterator<IsConst>& Deque<T>::base_iterator<IsConst>::operator+=(difference_type n) {
  difference_type offset = current - *block_ptr;
  difference_type new_offset = offset + n;

  block_ptr = block_ptr + (new_offset / CHUNK_SIZE);
  current = block_ptr + new_offset % CHUNK_SIZE;
  return *this;
}

template <typename T>
Deque<T>::Deque() : Deque(0ul, 0ul) {}

template <typename T>
Deque<T>::Deque(size_type size_, size_type number_of_blocks)
    : size_(size_), number_of_blocks(number_of_blocks), blocks(new chunk_type[number_of_blocks]) {
  if (size_ == 0) {
    first_block_index = 0;
    first_elem_offset = 0;
    last_block_index = 0;
    last_elem_offset = 0;
  } else {
    first_block_index = number_of_blocks / 3;
    first_elem_offset = 0;
    last_block_index = first_block_index + size_ / CHUNK_SIZE;
    last_elem_offset = (size_ - 1) % CHUNK_SIZE;
    if (std::is_default_constructible_v<T>) {
      for (size_type i = 0; i < number_of_blocks; ++i) blocks[i] = nullptr;
      for (size_type i = first_block_index; i <= last_block_index; ++i) {
        blocks[i] = new T[CHUNK_SIZE];
      }
    } else {
      for (size_type i = first_block_index; i <= last_block_index; ++i) {
        blocks[i] = static_cast<T*>(operator new(sizeof(T) * CHUNK_SIZE));
      }
    }
  }
}
template <typename T>
Deque<T>::Deque(const Deque& other) : Deque(other.size_, other.number_of_blocks) {
  this->first_block_index = other.first_block_index;
  this->first_elem_offset = other.first_elem_offset;
  this->last_block_index = other.last_block_index;
  this->last_elem_offset = other.last_elem_offset;
  for (size_type block_n = first_block_index; block_n <= last_block_index; ++block_n) {
    size_type end_index = block_n == last_block_index ? last_elem_offset + 1 : CHUNK_SIZE;
    size_type start_index = block_n == first_block_index ? first_elem_offset : 0;
    for (size_t elem_offset = start_index; elem_offset < end_index; ++elem_offset)
      blocks[block_n][elem_offset] = other.blocks[block_n][elem_offset];
  }
}
template <typename T>
Deque<T>::Deque(size_type size_, const T& value) : Deque(size_, size_ / CHUNK_SIZE + 1) {
  std::cout << first_block_index << ' ' << first_elem_offset << '\n';
  std::cout << last_block_index << ' ' << last_elem_offset << '\n';
  for (auto& i : *this) {
    i = T{value};
  }
}

template <typename T>
bool Deque<T>::empty() const noexcept {
  return size_ == 0;
}

template <typename T>
Deque<T>::iterator Deque<T>::begin() noexcept {
  if (empty()) return iterator(nullptr, nullptr);
  return iterator(blocks + first_block_index, blocks[first_block_index] + first_elem_offset);
}

template <typename T>
Deque<T>::const_iterator Deque<T>::cbegin() const noexcept {
  if (empty()) return end();
  return const_iterator(blocks + first_block_index, blocks[first_block_index] + first_elem_offset);
}

template <typename T>
Deque<T>::iterator Deque<T>::end() noexcept {
  if (empty()) return begin();
  auto block_ptr = blocks + last_block_index;
  auto elem_ptr = blocks[last_block_index] + last_elem_offset + 1;
  if (elem_ptr == *block_ptr + CHUNK_SIZE) {
    ++block_ptr;
    elem_ptr = *block_ptr;
  }
  return iterator(block_ptr, elem_ptr);
}

template <typename T>
Deque<T>::~Deque() {
  for (size_type i = first_block_index; i <= last_block_index; ++i) {
    delete[] blocks[i];
  }
  delete[] blocks;
}

}  // namespace bauyr
template <typename T>
using Deque = bauyr::Deque<T>;
int main() {
  Deque<int> d(10, 2);
  std::cout << sizeof(d) << "\n";
  {
    Deque<int> d2(d);
    int cnt = 0;
    for (auto i : d) {
      std::cout << cnt++ << ' ' << i << '\n';
    }
  }
}