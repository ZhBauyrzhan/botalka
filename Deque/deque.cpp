#include "deque.h"

#include <cassert>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <utility>

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
Deque<T>::Deque(size_t size_) : Deque(size_, (size_ / CHUNK_SIZE + 1)) {}

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
Deque<T>::Deque(const Deque& other)
    : size_(other.size_),
      number_of_blocks(other.number_of_blocks),
      blocks(new chunk_type[number_of_blocks]),
      first_block_index{other.first_block_index},
      first_elem_offset{other.first_elem_offset},
      last_block_index{other.last_block_index},
      last_elem_offset{other.last_elem_offset} {
  for (size_type i = 0; i < number_of_blocks; ++i) {
    blocks[i] = nullptr;
  }
  for (size_type block_n = first_block_index; block_n <= last_block_index; ++block_n) {
    blocks[block_n] = static_cast<T*>(operator new(sizeof(T) * CHUNK_SIZE));
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
Deque<T>::size_type Deque<T>::size() const {
  return size_;
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

template <typename T>
Deque<T>& Deque<T>::operator=(const Deque<T>& other) {
  if (this != &other) {
    Deque tmp(other);
    this->swap(tmp);
  }
  return *this;
}

template <typename T>
Deque<T>::reference Deque<T>::operator[](size_type index) {
  auto block_index = first_block_index + (index + first_elem_offset) / CHUNK_SIZE;
  auto elem_offset = (index + first_elem_offset) % CHUNK_SIZE;
  std::cout << block_index << ' ' << elem_offset << '\n';
  return blocks[block_index][elem_offset];
}

template <typename T>
Deque<T>::const_reference Deque<T>::operator[](size_type index) const {
  auto block_index = first_block_index + (index + first_elem_offset) / CHUNK_SIZE;
  auto elem_offset = (index + first_elem_offset) % CHUNK_SIZE;
  std::cout << block_index << ' ' << elem_offset << '\n';
  return blocks[block_index][elem_offset];
}

template <typename T>
Deque<T>::reference Deque<T>::at(size_type index) {
  if (index >= size_) throw std::out_of_range("Out of range in deque");
  return this->operator[](index);
}

template <typename T>
Deque<T>::const_reference Deque<T>::at(size_type index) const {
  if (index >= size_) throw std::out_of_range("Out of range in deque");
  return this->operator[](index);
}

template <typename T>
void Deque<T>::push_front(const T& x) {
  if (first_block_index == 0 && first_elem_offset == 0) {
    std::cout << "Trying to reallocate blocks" << std::endl;
    reallocate_blocks(size_ * increase_coefficient);
  }
  std::cout << "Trying to put element" << ' ' << first_block_index << ' ' << first_elem_offset
            << ' ' << number_of_blocks << std::endl;
  if (first_elem_offset != 0) {
    --first_elem_offset;
  } else if (first_block_index != 0) {
    --first_block_index;
    if (!blocks[first_block_index]) {
      blocks[first_block_index] = allocate_new_block();
    }
    std::cout << "M " << ' ' << first_block_index << ' ' << blocks[first_block_index] << std::endl;
    first_elem_offset = CHUNK_SIZE - 1;
  }
  blocks[first_block_index][first_elem_offset] = x;
  ++size_;
}

template <typename T>
void Deque<T>::push_back(const T& x) {
  // {number_of_blocks - 1, CHUNK_SIZE-1} is  for end() in the worst case
  if (last_block_index == number_of_blocks - 1 && last_elem_offset == CHUNK_SIZE - 2) {
    std::cout << "Trying to reallocate blocks" << std::endl;
    reallocate_blocks(size_ * increase_coefficient);
  }
  std::cout << "Trying to put element" << ' ' << last_block_index << ' ' << last_elem_offset << ' '
            << number_of_blocks << std::endl;
  if (last_elem_offset != CHUNK_SIZE - 1) {
    ++last_elem_offset;
  } else if (last_block_index != number_of_blocks) {
    ++last_block_index;
    if (!blocks[last_block_index]) {
      blocks[last_block_index] = allocate_new_block();
    }
    std::cout << "M " << ' ' << last_block_index << ' ' << blocks[last_block_index] << std::endl;
    last_elem_offset = 0;
  }
  blocks[last_block_index][last_elem_offset] = x;
  ++size_;
}

template <typename T>
void Deque<T>::pop_front() {
  std::pair<size_type, size_type> next_pos = next_position(first_block_index, first_elem_offset);
  first_block_index = next_pos.first;
  first_elem_offset = next_pos.second;
  --size_;
}

template <typename T>
void Deque<T>::pop_back() {
  std::pair<size_type, size_type> prev_pos = previous_position(last_block_index, last_elem_offset);
  last_block_index = prev_pos.first;
  last_elem_offset = prev_pos.second;
  --size_;
}

template <typename T>
void Deque<T>::reallocate_blocks(size_type new_number_of_blocks) {
  chunk_type* new_blocks = new chunk_type[new_number_of_blocks];
  for (size_type i = 0; i < new_number_of_blocks; ++i) {
    new_blocks[i] = nullptr;
  }
  size_type new_first_block_index = new_number_of_blocks / 3;
  std::cout << number_of_blocks << "\n";
  for (size_type i = 0; i < number_of_blocks; ++i) {
    std::cout << blocks[first_block_index + i] << std::endl;
    new_blocks[new_first_block_index + i] = blocks[first_block_index + i];
  }
  std::cout << "swapping" << std::endl;
  std::cout << new_blocks << " " << blocks << std::endl;
  last_block_index = new_first_block_index + number_of_blocks - 1;
  std::swap(new_blocks, blocks);
  std::swap(new_first_block_index, first_block_index);
  std::swap(new_number_of_blocks, number_of_blocks);
  delete[] new_blocks;
}

template <typename T>
std::pair<typename Deque<T>::size_type, typename Deque<T>::size_type> Deque<T>::next_position(
    size_type cur_block_index, size_type cur_elem_offset) {
  ++cur_elem_offset;
  if (cur_elem_offset == CHUNK_SIZE) {
    cur_elem_offset = 0;
    ++cur_block_index;
  }
  return {cur_block_index, cur_elem_offset};
}

template <typename T>
std::pair<typename Deque<T>::size_type, typename Deque<T>::size_type> Deque<T>::previous_position(
    size_type cur_block_index, size_type cur_elem_offset) {
  if (cur_elem_offset == 0) {
    cur_elem_offset = CHUNK_SIZE - 1;
    --cur_block_index;
  } else {
    --cur_elem_offset;
  }
  return {cur_block_index, cur_elem_offset};
}

template <typename T>
Deque<T>::chunk_type Deque<T>::allocate_new_block() {
  return reinterpret_cast<chunk_type>(new char[(CHUNK_SIZE * sizeof(T))]);
}

template <typename T>
void Deque<T>::swap(Deque<T>& other) {
  using std::swap;
  swap(size_, other.size_);
  swap(number_of_blocks, other.number_of_blocks);
  swap(blocks, other.blocks);
  swap(first_block_index, other.first_block_index);
  swap(first_elem_offset, other.first_elem_offset);
  swap(last_block_index, other.last_block_index);
  swap(last_elem_offset, other.last_elem_offset);
}

template <typename T>
void Deque<T>::print_blocks() {
  for (auto& i : *this) {
    std::cout << i << ' ';
  }
  std::cout << std::endl;
}

}  // namespace bauyr
template <typename T>
using Deque = bauyr::Deque<T>;

void test1() {
  Deque<int> d(10, 3);
  for (int i : d) std::cout << i << ' ';
  std::cout << '\n';
  d[3] = 5;

  d[7] = 8;

  d[9] = 10;
  std::string s = "33353338310";
  std::string ss;
  Deque<int> dd;

  {
    Deque<int> d2 = d;

    dd = d2;
    dd.print_blocks();
  }

  d[1] = 2;

  d.at(2) = 1;

  try {
    d.at(10) = 0;
    assert(false);
  } catch (std::out_of_range& err) {
    std::cout << err.what() << '\n';
  }

  const Deque<int>& ddd = dd;
  for (size_t i = 0; i < ddd.size(); ++i) {
    ss += std::to_string(ddd[i]);
  }

  assert(s == ss);
}

void test2() {
  Deque<int> d(1);

  d[0] = 0;

  d.print_blocks();

  for (int i = 0; i < 8; ++i) {
    d.push_back(i);
    d.push_front(i);
  }
  d.print_blocks();
  for (int i = 0; i < 12; ++i) {
    d.pop_front();
  }
  d.pop_back();
  d.print_blocks();
  assert(d.size() == 4);

  std::string ss;

  for (size_t i = 0; i < d.size(); ++i) {
    ss += std::to_string(d[i]);
  }

  assert(ss == "3456");
}

int main() {
  // Deque<int> d(10, 2);
  // std::cout << sizeof(d) << "\n";
  // {
  //   Deque<int> d2(d);
  //   int cnt = 0;
  //   for (auto i : d) {
  //     std::cout << cnt++ << ' ' << i << '\n';
  //   }
  //   std::cout << d[0];
  // }
  // test1();
  test2();
}