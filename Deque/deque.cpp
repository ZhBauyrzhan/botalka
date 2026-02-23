#include "deque.h"

#include <cassert>
#include <cstddef>
#include <stdexcept>

namespace bauyr {

template <typename T>
Deque<T>::Deque(const size_type size_, size_type chunk_number)
    : size_(size_), chunk_number(chunk_number), blocks(new T*[chunk_number]) {
  assert(chunk_number > 0UL);
  for (size_type i = 0; i < chunk_number; ++i) {
    blocks[i] = new T[CHUNK_SIZE];
  }
  first_chunk = blocks[chunk_number / 3];
  last_chunk = blocks[chunk_number - 1];
}

template <typename T>
Deque<T>::Deque() : Deque(size_ = 0, chunk_number = 1) {}

template <typename T>
Deque<T>::Deque(const size_type size_, const T& value)
    : Deque(size_, (size_ + CHUNK_SIZE - 1) / CHUNK_SIZE) {
  // TODO: Oprimize this
  // Fill chunks that should be full
  for (size_type i = 0; i < chunk_number - 1; ++i) {
    for (size_t j = 0; j < CHUNK_SIZE; ++j) {
      blocks[i][j] = value;
    }
  }
  // Fill the rest
  for (size_t j = 0; j < size_ % CHUNK_SIZE; ++j) {
    blocks[chunk_number - 1][j] = value;
  }
}

template <typename T>
Deque<T>::~Deque() {
  for (size_type i = 0; i < chunk_number; ++i) {
    delete[] blocks[i];
  }
  delete[] blocks;
}

template <typename T>
T& Deque<T>::at(size_type pos) {
  if (check_range(pos)) return this->operator[](pos);
  throw std::out_of_range("Out of range attempt");
}

template <typename T>
const T& Deque<T>::at(size_type pos) const {
  if (check_range(pos)) return this->operator[](pos);
  throw std::out_of_range("Out of range attempt");
};
;

template <typename T>
size_t Deque<T>::size() const {
  return size_;
};

template <typename T>
bool Deque<T>::check_range(size_t pos) {
  return pos < size_;
}

}  // namespace bauyr
int main() {
  bauyr::Deque<int> d;
}