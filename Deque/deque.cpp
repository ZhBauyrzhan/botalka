#include "deque.h"

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <random>
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
Deque<T>::base_iterator<IsConst>::base_iterator(const Deque<T>::base_iterator<IsConst>& other)
    : block_ptr(other.block_ptr), current(other.current) {}

template <typename T>
template <bool IsConst>
template <bool B, typename>
Deque<T>::base_iterator<IsConst>::base_iterator(const base_iterator<false>& other)
    : block_ptr(other.block_ptr), current(other.current) {}

template <typename T>
template <bool IsConst>
Deque<T>::base_iterator<IsConst>& Deque<T>::base_iterator<IsConst>::operator++() {
  if (block_ptr == nullptr) {
    return *this;
  }
  ++current;
  if (current == *block_ptr + CHUNK_SIZE) {
    ++block_ptr;
    current = *block_ptr;
  }
  return *this;
}

template <typename T>
template <bool IsConst>
Deque<T>::base_iterator<IsConst>& Deque<T>::base_iterator<IsConst>::operator--() {
  if (block_ptr == nullptr) return *this;
  --current;
  if (current < *block_ptr) {
    --block_ptr;
    current = *block_ptr + CHUNK_SIZE - 1;
  }
  return *this;
}
template <typename T>
template <bool IsConst>
Deque<T>::base_iterator<IsConst>& Deque<T>::base_iterator<IsConst>::operator+=(difference_type n) {
  if (block_ptr == nullptr) {
    return *this;
  }
  difference_type offset = current - *block_ptr;
  difference_type new_offset = offset + n;
  difference_type block_shift = new_offset / (difference_type)CHUNK_SIZE;
  difference_type pos = new_offset % (difference_type)CHUNK_SIZE;

  if (pos < 0) {
    pos += CHUNK_SIZE;
    --block_shift;
  }

  block_ptr += block_shift;
  current = *block_ptr + pos;

  return *this;
}

template <typename T>
template <bool IsConst>
Deque<T>::base_iterator<IsConst>& Deque<T>::base_iterator<IsConst>::operator-=(difference_type n) {
  // TODO : Define this operator
  return this->operator+=(-n);
}

template <typename T>
template <bool IsConst>
Deque<T>::base_iterator<IsConst> Deque<T>::base_iterator<IsConst>::operator+(
    difference_type n) const {
  base_iterator<IsConst> tmp = *this;
  tmp += n;
  return tmp;
}

template <typename T>
template <bool IsConst>
Deque<T>::base_iterator<IsConst> Deque<T>::base_iterator<IsConst>::operator-(
    difference_type n) const {
  base_iterator<IsConst> tmp = *this;
  tmp -= n;
  return tmp;
}

template <typename T>
template <bool IsConst>
Deque<T>::base_iterator<IsConst>::difference_type Deque<T>::base_iterator<IsConst>::operator-(
    const Deque<T>::base_iterator<IsConst>& other) const {
  if (block_ptr == nullptr && other.block_ptr == nullptr) {
    return 0;
  }
  return ((block_ptr - other.block_ptr) * CHUNK_SIZE + (current - *block_ptr) -
          (other.current - *other.block_ptr));
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
bool Deque<T>::base_iterator<IsConst>::operator==(const base_iterator& other) const {
  return current == other.current;
}

template <typename T>
template <bool IsConst>
bool Deque<T>::base_iterator<IsConst>::operator!=(const base_iterator& other) const {
  return current != other.current;
}

template <typename T>
template <bool IsConst>
bool Deque<T>::base_iterator<IsConst>::operator>(const base_iterator& other) const {
  return (block_ptr > other.block_ptr || (block_ptr == other.block_ptr && current > other.current));
}

template <typename T>
template <bool IsConst>
bool Deque<T>::base_iterator<IsConst>::operator<(const base_iterator& other) const {
  return (block_ptr < other.block_ptr || (block_ptr == other.block_ptr && current < other.current));
}

template <typename T>
template <bool IsConst>
bool Deque<T>::base_iterator<IsConst>::operator>=(const base_iterator& other) const {
  return !(operator<(other));
}

template <typename T>
template <bool IsConst>
bool Deque<T>::base_iterator<IsConst>::operator<=(const base_iterator& other) const {
  return !operator>(other);
}

template <typename T>
template <bool IsConst>
Deque<T>::base_iterator<IsConst>::reference Deque<T>::base_iterator<IsConst>::operator*() const {
  return *current;
}

template <typename T>
template <bool IsConst>
Deque<T>::base_iterator<IsConst>::pointer Deque<T>::base_iterator<IsConst>::operator->() const {
  return current;
}

template <typename T>
template <bool IsConst>
void Deque<T>::base_iterator<IsConst>::print() {
  std::cout << "Printing iterator" << std::endl;
  std::cout << "block_ptr " << block_ptr << std::endl;
  std::cout << "current " << current << std::endl;
  std::cout << "val " << *current << '\n' << std::endl;
}

template <typename T>
Deque<T>::Deque() : Deque(0ul, 0ul, Deque::PrivateConstuctorTag()) {}

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
  if (!size_) return;
  for (size_type block_n = first_block_index; block_n <= last_block_index; ++block_n) {
    blocks[block_n] = static_cast<T*>(operator new(sizeof(T) * CHUNK_SIZE));
    size_type end_index = block_n == last_block_index ? last_elem_offset + 1 : CHUNK_SIZE;
    size_type start_index = block_n == first_block_index ? first_elem_offset : 0;
    for (size_t elem_offset = start_index; elem_offset < end_index; ++elem_offset)
      blocks[block_n][elem_offset] = other.blocks[block_n][elem_offset];
  }
}

template <typename T>
Deque<T>::Deque(size_type size_, const T& value)
    : Deque(size_, (size_ / CHUNK_SIZE + 1) * Deque::increase_coefficient,
            Deque::PrivateConstuctorTag()) {
  // std::cout << this->size() << " " << std::endl;
  for (auto& i : *this) {
    i = T{value};
    // std::cout << ++cnt << std::endl;
  }
}
template <typename T>
Deque<T>::Deque(size_t size_)
    : Deque(size_, (size_ / CHUNK_SIZE + 1), Deque::PrivateConstuctorTag()) {}

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
bool Deque<T>::empty() const noexcept {
  return size_ == 0;
}

template <typename T>
Deque<T>::size_type Deque<T>::size() const {
  return size_;
}

template <typename T>
Deque<T>::reference Deque<T>::operator[](size_type index) noexcept {
  auto block_index = first_block_index + (index + first_elem_offset) / CHUNK_SIZE;
  auto elem_offset = (index + first_elem_offset) % CHUNK_SIZE;
  return blocks[block_index][elem_offset];
}

template <typename T>
Deque<T>::const_reference Deque<T>::operator[](size_type index) const noexcept {
  auto block_index = first_block_index + (index + first_elem_offset) / CHUNK_SIZE;
  auto elem_offset = (index + first_elem_offset) % CHUNK_SIZE;
  return blocks[block_index][elem_offset];
}

template <typename T>
Deque<T>::iterator Deque<T>::begin() noexcept {
  if (empty()) return iterator(nullptr, nullptr);
  return iterator(blocks + first_block_index, blocks[first_block_index] + first_elem_offset);
}

template <typename T>
Deque<T>::const_iterator Deque<T>::begin() const noexcept {
  if (empty()) return const_iterator(nullptr, nullptr);
  return const_iterator(blocks + first_block_index, blocks[first_block_index] + first_elem_offset);
}

template <typename T>
Deque<T>::reverse_iterator Deque<T>::rbegin() noexcept {
  return reverse_iterator(end());
}

template <typename T>
Deque<T>::const_reverse_iterator Deque<T>::rbegin() const noexcept {
  return const_reverse_iterator(end());
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
Deque<T>::const_iterator Deque<T>::end() const noexcept {
  if (empty()) return begin();
  auto block_ptr = blocks + last_block_index;
  auto elem_ptr = blocks[last_block_index] + last_elem_offset + 1;
  if (elem_ptr == *block_ptr + CHUNK_SIZE) {
    ++block_ptr;
    elem_ptr = *block_ptr;
  }
  return const_iterator(block_ptr, elem_ptr);
}

template <typename T>
Deque<T>::reverse_iterator Deque<T>::rend() noexcept {
  return reverse_iterator(begin());
}

template <typename T>
Deque<T>::const_reverse_iterator Deque<T>::rend() const noexcept {
  return const_iterator(begin());
}

template <typename T>
Deque<T>::const_iterator Deque<T>::cend() const noexcept {
  if (empty()) return cbegin();
  auto block_ptr = blocks + last_block_index;
  auto elem_ptr = blocks[last_block_index] + last_elem_offset + 1;
  if (elem_ptr == *block_ptr + CHUNK_SIZE) {
    ++block_ptr;
    elem_ptr = *block_ptr;
  }
  return const_iterator(block_ptr, elem_ptr);
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
  if (size_ == 0 && number_of_blocks == 0) {
    reallocate_blocks(3, true);
    blocks[first_block_index][first_elem_offset] = x;
    ++size_;
    return;
  }
  if (first_block_index == 0 && first_elem_offset == 0) {
    reallocate_blocks(number_of_blocks * increase_coefficient, false);
    --first_block_index;
    if (!blocks[first_block_index]) {
      blocks[first_block_index] = allocate_new_block();
    }
    first_elem_offset = CHUNK_SIZE - 1;
  } else if (first_elem_offset != 0) {
    --first_elem_offset;
  } else {
    --first_block_index;
    if (!blocks[first_block_index]) {
      blocks[first_block_index] = allocate_new_block();
    }
    first_elem_offset = CHUNK_SIZE - 1;
  }
  blocks[first_block_index][first_elem_offset] = x;
  ++size_;
}

template <typename T>
void Deque<T>::push_back(const T& x) {
  if (size_ == 0 && number_of_blocks == 0) {
    reallocate_blocks(3, true);
    blocks[first_block_index][first_elem_offset] = x;
    ++size_;
    return;
  }
  if (last_block_index == number_of_blocks - 1 && last_elem_offset == CHUNK_SIZE - 1) {
    reallocate_blocks(number_of_blocks * increase_coefficient, false);
    ++last_block_index;
    if (!blocks[last_block_index]) {
      blocks[last_block_index] = allocate_new_block();
    }
    last_elem_offset = 0;
  } else if (last_elem_offset == CHUNK_SIZE - 1) {
    ++last_block_index;
    if (!blocks[last_block_index]) {
      blocks[last_block_index] = allocate_new_block();
    }
    last_elem_offset = 0;
  } else {
    ++last_elem_offset;
  }
  blocks[last_block_index][last_elem_offset] = x;
  ++size_;
}

template <typename T>
typename Deque<T>::iterator Deque<T>::insert(const_iterator position, const T& x) {
  size_type index = position - cbegin();
  if (index == size_) {
    push_back(x);
    return end() - 1;
  }
  push_back(x);
  for (size_type i = size_ - 1; i > index; --i) {
    (*this)[i] = (*this)[i - 1];
  }
  (*this)[index] = x;
  return begin() + index;
}

template <typename T>
void Deque<T>::pop_back() {
  std::pair<size_type, size_type> prev_pos = previous_position(last_block_index, last_elem_offset);
  last_block_index = prev_pos.first;
  last_elem_offset = prev_pos.second;
  --size_;
}

template <typename T>
void Deque<T>::pop_front() {
  std::pair<size_type, size_type> next_pos = next_position(first_block_index, first_elem_offset);
  first_block_index = next_pos.first;
  first_elem_offset = next_pos.second;
  --size_;
}

template <typename T>
void Deque<T>::erase(Deque<T>::base_iterator<false> it) {
  for (iterator i = it; i + 1 != end(); ++i) {
    *i = *(i + 1);
  }
  pop_back();
}

template <typename T>
void Deque<T>::print_blocks() {
  for (auto& i : *this) {
    std::cout << i << ' ';
  }
  std::cout << std::endl;
}

template <typename T>
void Deque<T>::print_vals() {
  std::cout << "size_ " << size_ << std::endl;
  std::cout << "number_of_blocks " << number_of_blocks << std::endl;
  std::cout << "first_block_index " << first_block_index << std::endl;
  std::cout << "first_elem_offset " << first_elem_offset << std::endl;
  std::cout << "last_block_index " << last_block_index << std::endl;
  std::cout << "last_elem_offset " << last_elem_offset << std::endl;
}

template <typename T>
Deque<T>::Deque(size_type size_, size_type number_of_blocks, Deque::PrivateConstuctorTag)
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
    // this->print_vals();
    for (size_type i = first_block_index; i <= last_block_index; ++i) {
      blocks[i] = allocate_new_block();
    }
  }
}

template <typename T>
void Deque<T>::reallocate_blocks(size_type new_number_of_blocks, bool isFirstTime) {
  chunk_type* new_blocks = new chunk_type[new_number_of_blocks];
  if (isFirstTime) {
    for (size_type i = 0; i < new_number_of_blocks; ++i) {
      new_blocks[i] = allocate_new_block();
    }
    std::swap(new_blocks, blocks);
    number_of_blocks = new_number_of_blocks;
    first_block_index = number_of_blocks / 3;
    first_elem_offset = 0;
    last_block_index = number_of_blocks / 3;
    last_elem_offset = 0;
    size_ = 0;
    return;
  }
  for (size_type i = 0; i < new_number_of_blocks; ++i) {
    new_blocks[i] = nullptr;
  }
  size_type new_first_block_index = new_number_of_blocks / 3;
  for (size_type i = 0; i < last_block_index - first_block_index + 1; ++i) {
    new_blocks[new_first_block_index + i] = blocks[first_block_index + i];
  }
  last_block_index = new_first_block_index + last_block_index - first_block_index;
  std::swap(new_blocks, blocks);
  std::swap(new_first_block_index, first_block_index);
  std::swap(new_number_of_blocks, number_of_blocks);
  delete[] new_blocks;
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
  if constexpr (std::is_default_constructible_v<T>) {
    return new T[CHUNK_SIZE];
  }
  return reinterpret_cast<chunk_type>(new char[(CHUNK_SIZE * sizeof(T))]);
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
  std::cout << "Test1 passed" << std::endl;
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

  std::cout << "Test2 passed" << std::endl;
}

void test3() {
  Deque<int> d;

  for (int i = 0; i < 1000; ++i) {
    for (int j = 0; j < 1000; ++j) {
      if (j % 3 == 2) {
        d.pop_back();
      } else {
        d.push_front(i * j);
      }
    }
  }

  assert(d.size() == 334'000);
  Deque<int>::iterator left = d.begin() + 100'000;
  Deque<int>::iterator right = d.end() - 233'990;
  while (d.begin() != left) d.pop_front();
  while (d.end() != right) {
    d.pop_back();
  }

  assert(d.size() == 10);
  assert(right - left == 10);

  std::string s;
  for (auto it = left; it != right; ++it) {
    ++*it;
  }
  int cnt = 20;
  for (auto it = right - 1; it >= left; --it) {
    if (--cnt == 0) return;
    s += std::to_string(*it);
  }
  assert(s == "51001518515355154401561015695158651595016120162051");
  std::cout << "Test3 passed" << std::endl;
}

struct S {
  int x = 0;
  double y = 0.0;
};

void test4() {
  Deque<S> d(5, {1, 2.0});
  const Deque<S>& cd = d;

  static_assert(!std::is_assignable_v<decltype(*cd.begin()), S>);
  static_assert(std::is_assignable_v<decltype(*d.begin()), S>);
  static_assert(!std::is_assignable_v<decltype(*d.cbegin()), S>);

  static_assert(!std::is_assignable_v<decltype(*cd.end()), S>);
  static_assert(std::is_assignable_v<decltype(*d.end()), S>);
  static_assert(!std::is_assignable_v<decltype(*d.cend()), S>);

  assert(cd.size() == 5);

  auto it = d.begin() + 2;
  auto cit = cd.end() - 3;

  it->x = 5;
  assert(cit->x == 5);
  d.erase(d.begin() + 0);
  d.erase(d.begin() + 3);
  assert(d.size() == 3);

  auto dd = cd;

  dd.pop_back();
  dd.insert(dd.begin(), {3, 4.0});
  dd.insert(dd.begin() + 2, {4, 5.0});
  std::string s;
  for (const auto& x : dd) {
    s += std::to_string(x.x);
  }
  assert(s == "3145");

  std::string ss;
  for (const auto& x : d) {
    ss += std::to_string(x.x);
  }
  assert(ss == "151");
  std::cout << "Test4 passed" << std::endl;
}

void test5() {
  Deque<int> d;

  d.push_back(1);
  d.push_front(2);

  auto left_ptr = &*d.begin();
  auto right_ptr = &*(d.end() - 1);

  d.push_back(3);
  d.push_front(4);
  auto left = *d.begin();
  auto right = *(d.end() - 1);
  d.print_blocks();
  for (int i = 0; i < 20'000; ++i) {
    d.push_front(i);
  }

  std::string s;
  s += std::to_string(left);
  s += std::to_string(right);

  s += std::to_string(*left_ptr);
  s += std::to_string(*right_ptr);
  // for (auto it = left; it <= right; ++it) {
  //     s += std::to_string(*it);
  // }
  assert(s == "4321");
  std::cout << "Test5 passed" << std::endl;
}
struct VerySpecialType {
  int x = 0;

  explicit VerySpecialType(int x) : x(x) {}
};

struct NotDefaultConstructible {
  NotDefaultConstructible() = delete;
  NotDefaultConstructible(const NotDefaultConstructible&) = default;
  NotDefaultConstructible& operator=(const NotDefaultConstructible&) = default;

  NotDefaultConstructible(VerySpecialType v) : x(v.x) {}

 public:
  int x = 0;
};

void test6() {
  Deque<NotDefaultConstructible> d;

  NotDefaultConstructible ndc = VerySpecialType(-1);

  for (int i = 0; i < 1500; ++i) {
    ++ndc.x;
    d.push_back(ndc);
  }

  assert(d.size() == 1500);

  for (int i = 0; i < 1300; ++i) {
    d.pop_front();
  }

  assert(d.size() == 200);

  assert(d[99].x == 1399);

  d[100] = VerySpecialType(0);
  assert(d[100].x == 0);

  std::cout << "Test6 passed" << std::endl;
}

struct Explosive {
  int x = 0;
  Explosive(int x) : x(x) {}
  Explosive(const Explosive&) {
    if (x) throw std::runtime_error("Boom!");
  }
};

void test7() {
  Deque<Explosive> d;
  d.push_back(Explosive(0));

  for (int i = 0; i < 30'000; ++i) {
    auto it = d.begin();
    auto x = it->x;
    size_t sz = d.size();
    try {
      if (i % 2)
        d.push_back(Explosive(1));
      else
        d.push_front(Explosive(1));
    } catch (...) {
      assert(it == d.begin());
      assert(d.begin()->x == x);
      assert(d.size() == sz);
    }

    d.push_back(Explosive(0));
  }
  std::cout << "Test7 passed" << std::endl;
}
namespace TestsByUnrealf1 {
struct Fragile {
  Fragile(int durability, int data) : durability(durability), data(data) {}
  ~Fragile() = default;

  // for std::swap
  Fragile(Fragile&& other) : Fragile() { *this = other; }

  Fragile(const Fragile& other) : Fragile() { *this = other; }

  Fragile& operator=(const Fragile& other) {
    durability = other.durability - 1;
    data = other.data;
    if (durability <= 0) {
      throw 2;
    }
    return *this;
  }

  int durability;
  int data;

 private:
  Fragile() {}
};

struct Explosive {
  struct Safeguard {};

  inline static bool exploded = false;

  Explosive() : should_explode(true) { throw 1; }

  Explosive(Safeguard) : should_explode(false) {}

  Explosive(const Explosive&) : should_explode(true) { throw 2; }

  // TODO: is this ok..?
  Explosive& operator=(const Explosive&) { return *this; }

  ~Explosive() { exploded |= should_explode; }

 private:
  const bool should_explode;
};

struct DefaultConstructible {
  DefaultConstructible() { data = default_data; }

  int data = default_data;
  inline static const int default_data = 117;
};

struct NotDefaultConstructible {
  NotDefaultConstructible() = delete;
  NotDefaultConstructible(int input) : data(input) {}
  int data;

  auto operator<=>(const NotDefaultConstructible&) const = default;
};

struct CountedException : public std::exception {};

template <int when_throw>
struct Counted {
  inline static int counter = 0;

  Counted() {
    ++counter;
    if (counter == when_throw) {
      --counter;
      throw CountedException();
    }
  }

  Counted(const Counted&) : Counted() {}

  ~Counted() { --counter; }
};

template <typename iter, typename T>
struct CheckIter {
  using traits = std::iterator_traits<iter>;

  static_assert(std::is_same_v<std::remove_cv_t<typename traits::value_type>, std::remove_cv_t<T>>);
  static_assert(std::is_same_v<typename traits::pointer, T*>);
  static_assert(std::is_same_v<typename traits::reference, T&>);
  static_assert(
      std::is_same_v<typename traits::iterator_category, std::random_access_iterator_tag>);

  static_assert(std::is_same_v<decltype(std::declval<iter>()++), iter>);
  static_assert(std::is_same_v<decltype(++std::declval<iter>()), iter&>);
  static_assert(std::is_same_v<decltype(std::declval<iter>() + 5), iter>);
  static_assert(std::is_same_v<decltype(std::declval<iter>() += 5), iter&>);

  static_assert(std::is_same_v<decltype(std::declval<iter>() - std::declval<iter>()),
                               typename traits::difference_type>);
  static_assert(std::is_same_v<decltype(*std::declval<iter>()), T&>);

  static_assert(std::is_same_v<decltype(std::declval<iter>() < std::declval<iter>()), bool>);
  static_assert(std::is_same_v<decltype(std::declval<iter>() <= std::declval<iter>()), bool>);
  static_assert(std::is_same_v<decltype(std::declval<iter>() > std::declval<iter>()), bool>);
  static_assert(std::is_same_v<decltype(std::declval<iter>() >= std::declval<iter>()), bool>);
  static_assert(std::is_same_v<decltype(std::declval<iter>() == std::declval<iter>()), bool>);
  static_assert(std::is_same_v<decltype(std::declval<iter>() != std::declval<iter>()), bool>);
};

void testDefault() {
  Deque<int> defaulted;
  assert((defaulted.size() == 0));
  Deque<NotDefaultConstructible> without_default;
  assert((without_default.size() == 0));
  std::cout << "Test default passed" << std::endl;
}

void testCopy() {
  Deque<NotDefaultConstructible> without_default;
  Deque<NotDefaultConstructible> copy = without_default;
  assert((copy.size() == 0));
  std::cout << "Test copy passed" << std::endl;
}

void testWithSize() {
  int size = 17;
  int value = 14;
  Deque<int> simple(size);
  assert((simple.size() == size_t(size)) &&
         std::all_of(simple.begin(), simple.end(), [](int item) { return item == 0; }));
  Deque<NotDefaultConstructible> less_simple(size, value);
  std::cout << (less_simple.size() == size_t(size)) << ' '
            << std::all_of(less_simple.begin(), less_simple.end(),
                           [&](const auto& item) { return item.data == value; })
            << std::endl;
  assert((less_simple.size() == size_t(size)) &&
         std::all_of(less_simple.begin(), less_simple.end(),
                     [&](const auto& item) { return item.data == value; }));
  Deque<DefaultConstructible> default_constructor(size);
  std::cout << std::endl;
  assert(std::all_of(default_constructor.begin(), default_constructor.end(),
                     [](const auto& item)

                     { return item.data == DefaultConstructible::default_data; }));

  std::cout << "Test with size passed" << std::endl;
}

void testAssignment() {
  Deque<int> first(10, 10);
  Deque<int> second(9, 9);
  first = second;
  assert((first.size() == second.size()) && (first.size() == 9) &&
         std::equal(first.begin(), first.end(), second.begin()));
  std::cout << "Test assignment passed" << std::endl;
}

void testStaticAsserts() {
  using T1 = int;
  using T2 = NotDefaultConstructible;

  static_assert(std::is_default_constructible_v<Deque<T1>>, "should have default constructor");
  static_assert(std::is_default_constructible_v<Deque<T2>>, "should have default constructor");
  static_assert(std::is_copy_constructible_v<Deque<T1>>, "should have copy constructor");
  static_assert(std::is_copy_constructible_v<Deque<T2>>, "should have copy constructor");
  static_assert(std::is_constructible_v<Deque<T1>, int>, "should have constructor from int");
  static_assert(std::is_constructible_v<Deque<T2>, int>, "should have constructor from int");
  static_assert(std::is_constructible_v<Deque<T1>, int, const T1&>,
                "should have constructor from int and const T&");
  static_assert(std::is_constructible_v<Deque<T2>, int, const T2&>,
                "should have constructor from int and const T&");

  static_assert(std::is_copy_assignable_v<Deque<T1>>, "should have assignment operator");
  static_assert(std::is_copy_assignable_v<Deque<T2>>, "should have assignment operator");
  std::cout << "Test static asserts passed" << std::endl;
}

void testOperatorSubscript() {
  Deque<size_t> defaulted(1300, 43);
  std::cout << defaulted.size() << std::endl;
  std::cout << defaulted[0] << std::endl;
  assert((defaulted[0] == defaulted[1280]) && (defaulted[0] == 43));
  assert((defaulted.at(0) == defaulted[1280]) && (defaulted.at(0) == 43));
  int caught = 0;
  try {
    defaulted.at(size_t(-1));
  } catch (std::out_of_range& e) {
    ++caught;
  }

  try {
    defaulted.at(1300);
  } catch (std::out_of_range& e) {
    ++caught;
  }

  assert(caught == 2);
  std::cout << "Test testOperatorSubscript  passed" << std::endl;
}

void testStaticAssertsAccess() {
  Deque<size_t> defaulted;
  const Deque<size_t> constant;
  static_assert(std::is_same_v<decltype(defaulted[0]), size_t&>);
  static_assert(std::is_same_v<decltype(defaulted.at(0)), size_t&>);
  static_assert(std::is_same_v<decltype(constant[0]), const size_t&>);
  static_assert(std::is_same_v<decltype(constant.at(0)), const size_t&>);

  static_assert(noexcept(defaulted[0]), "operator[] should not throw");
  static_assert(!noexcept(defaulted.at(0)), "at() can throw");
  std::cout << "Test testStaticAssertsAccess  passed" << std::endl;
}

void testStaticAssertsIterators() {
  CheckIter<Deque<int>::iterator, int> iter;
  std::ignore = iter;
  CheckIter<decltype(std::declval<Deque<int>>().rbegin()), int> reverse_iter;
  std::ignore = reverse_iter;
  CheckIter<decltype(std::declval<Deque<int>>().cbegin()), const int> const_iter;
  std::ignore = const_iter;

  static_assert(std::is_convertible_v<decltype(std::declval<Deque<int>>().begin()),
                                      decltype(std::declval<Deque<int>>().cbegin())>,
                "should be able to construct const iterator from non const iterator");
  static_assert(!std::is_convertible_v<decltype(std::declval<Deque<int>>().cbegin()),
                                       decltype(std::declval<Deque<int>>().begin())>,
                "should NOT be able to construct iterator from const iterator");
  std::cout << "Test testStaticAssertsIterators  passed" << std::endl;
}

void testIteratorsArithmetic() {
  Deque<int> empty;

  assert((empty.end() - empty.begin()) == 0);

  assert((empty.begin() + 0 == empty.end()) && (empty.end() - 0 == empty.begin()));
  Deque<int> one(1);
  auto iter2 = one.end();
  assert(((--iter2) == one.begin()));

  assert((empty.rend() - empty.rbegin()) == 0);
  assert((empty.rbegin() + 0 == empty.rend()) && (empty.rend() - 0 == empty.rbegin()));
  auto r_iter = empty.rbegin();

  assert((r_iter++ == empty.rbegin()));

  assert((empty.cend() - empty.cbegin()) == 0);
  assert((empty.cbegin() + 0 == empty.cend()) && (empty.cend() - 0 == empty.cbegin()));
  auto c_iter = empty.cbegin();

  assert((c_iter++ == empty.cbegin()));

  Deque<int> d(1000, 3);

  assert(size_t((d.end() - d.begin())) == d.size());
  assert((d.begin() + d.size() == d.end()) && (d.end() - d.size() == d.begin()));
  std::cout << "testIteratorsArithmetic passed" << std::endl;
}

void testIteratorsComparison() {
  Deque<int> d(1000, 3);

  assert(d.end() > d.begin());
  assert(d.cend() > d.cbegin());
  assert(d.rend() > d.rbegin());
  std::cout << "testIteratorsComparison passed" << std::endl;
}

void testIteratorsAlgorithms() {
  Deque<int> d(1000, 3);

  std::iota(d.begin(), d.end(), 13);
  std::mt19937 g(31415);
  std::shuffle(d.begin(), d.end(), g);
  std::sort(d.rbegin(), d.rbegin() + 500);
  std::reverse(d.begin(), d.end());
  auto sorted_border = std::is_sorted_until(d.begin(), d.end());
  // std::copy(d.begin(), d.end(), std::ostream_iterator<int>(std::cout, " "));
  // std::cout << std::endl;
  assert(sorted_border - d.begin() == 500);
  std::cout << "testIteratorsAlgorithms passed" << std::endl;
}

void testPushAndPop() {
  Deque<NotDefaultConstructible> d(10000, {1});
  auto start_size = d.size();

  auto middle = &(*(d.begin() + start_size / 2));  // 5000
  auto& middle_element = *middle;
  auto begin = &(*d.begin());
  auto end = &(*d.rbegin());

  auto middle2 = &(*((d.begin() + start_size / 2) + 2000));  // 7000

  // remove 400 elements
  for (size_t i = 0; i < 400; ++i) {
    d.pop_back();
  }

  // begin and middle pointers are still valid
  assert(begin->data == 1);
  assert(middle->data == 1);
  assert(middle_element.data == 1);
  assert(middle2->data == 1);

  end = &*d.rbegin();

  // 800 elemets removed in total
  for (size_t i = 0; i < 400; ++i) {
    d.pop_front();
  }

  // and and middle iterators are still valid
  assert(end->data == 1);
  assert(middle->data == 1);
  assert(middle_element.data == 1);
  assert(middle2->data == 1);

  // removed 9980 items in total
  for (size_t i = 0; i < 4590; ++i) {
    d.pop_front();
    d.pop_back();
  }

  assert(d.size() == 20);
  assert(middle_element.data == 1);
  assert(middle->data == 1 && middle->data == 1);
  assert(std::all_of(d.begin(), d.end(), [](const auto& item) { return item.data == 1; }));

  begin = &*d.begin();
  end = &*d.rbegin();

  for (size_t i = 0; i < 5500; ++i) {
    d.push_back({2});
    d.push_front({2});
  }

  assert((*begin).data == 1);
  assert((*end).data == 1);
  assert(d.begin()->data == 2);
  assert(d.size() == 5500 * 2 + 20);
  assert(std::count(d.begin(), d.end(), NotDefaultConstructible{1}) == 20);
  assert(std::count(d.begin(), d.end(), NotDefaultConstructible{2}) == 11000);
  std::cout << "testPushAndPop passed" << std::endl;
}

void testInsertAndErase() {
  Deque<NotDefaultConstructible> d(10000, {1});
  auto start_size = d.size();

  d.insert(d.begin() + start_size / 2, NotDefaultConstructible{2});
  assert(d.size() == start_size + 1);
  d.erase(d.begin() + start_size / 2 - 1);
  assert(d.size() == start_size);

  assert(size_t(std::count(d.begin(), d.end(), NotDefaultConstructible{1})) == start_size - 1);
  assert(std::count(d.begin(), d.end(), NotDefaultConstructible{2}) == 1);

  Deque<NotDefaultConstructible> copy;
  for (const auto& item : d) {
    copy.insert(copy.end(), item);
  }
  // std::copy(d.cbegin(), d.cend(), std::inserter(copy, copy.begin()));

  assert(d.size() == copy.size());
  assert(std::equal(d.begin(), d.end(), copy.begin()));
  std::cout << "testInsertAndErase passed" << std::endl;
}

void testExceptions() {
  try {
    Deque<Counted<17>> d(100);
  } catch (CountedException& e) {
    assert(Counted<17>::counter == 0);
  } catch (...) {
    // should have caught same exception as thrown by Counted
    assert(false);
  }

  try {
    Deque<Explosive> d(100);
  } catch (...) {
  }

  try {
    Deque<Explosive> d;
  } catch (...) {
    // no objects should have been created
    assert(false);
  }
  assert(Explosive::exploded == false);

  try {
    Deque<Explosive> d;
    auto safe = Explosive(Explosive::Safeguard{});
    d.push_back(safe);
  } catch (...) {
  }

  // Destructor should not be called for an object
  // with no finihshed constructor
  // the only destructor called - safe explosive with the safeguard
  assert(Explosive::exploded == false);
  std::cout << "testExceptions passed" << std::endl;
}

void testStrongGuarantee() {
  const size_t size = 20'000;
  const size_t initial_data = 100;
  Deque<Fragile> d(size, Fragile(size, initial_data));

  auto is_intact = [&] {
    return d.size() == size && std::all_of(d.begin(), d.end(), [initial_data](const auto& item) {
             return item.data == initial_data;
           });
  };
  try {
    d.insert(d.begin() + size / 2, Fragile(0, initial_data + 1));
  } catch (...) {
    // have to throw
    assert(is_intact());
  }
  try {
    // for those who like additional copies...
    d.insert(d.begin() + size / 2, Fragile(3, initial_data + 2));
  } catch (...) {
    // might throw depending on the implementation
    // if it DID throw, then deque should be untouched
    assert(is_intact());
  }
  std::cout << "testStrongGuarantee passed" << std::endl;
}

}  // namespace TestsByUnrealf1

int main() {
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
  test7();
  TestsByUnrealf1::testDefault();
  TestsByUnrealf1::testCopy();
  TestsByUnrealf1::testWithSize();
  TestsByUnrealf1::testAssignment();
  TestsByUnrealf1::testStaticAsserts();
  TestsByUnrealf1::testOperatorSubscript();
  TestsByUnrealf1::testStaticAssertsAccess();
  TestsByUnrealf1::testStaticAssertsIterators();
  TestsByUnrealf1::testIteratorsArithmetic();
  TestsByUnrealf1::testIteratorsComparison();
  TestsByUnrealf1::testIteratorsAlgorithms();
  TestsByUnrealf1::testPushAndPop();
  TestsByUnrealf1::testInsertAndErase();
  TestsByUnrealf1::testExceptions();
  TestsByUnrealf1::testStrongGuarantee();
}
