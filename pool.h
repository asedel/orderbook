#ifndef POOL_H
#define POOL_H

#include <exception>
#include <vector>

using std::vector;

/** A custom pooling allocator of fixed size

    using a non-shrinking vector as the pool source
    using a LIFO stack as the free list

    if there are no free locations return the reserved id 0

    If free location, pop address off free list
    Free objects by pushing address to free list

    Because I won't resize pointers are stable but I won't rely on
    them as such in case we want to grow. ( Though I would generally suggest a skip list in that approach )

    Performance: should be very stable and O(1) since its just pop and decrement and dereference ( which is likely into the cache ).  Deallocation is just a decrement and a write to memory

*/

template <class T, typename ptr_t, size_t SIZE>
  class pool
{
public:
  using size_ptr = typename std::underlying_type<ptr_t>::type;
  vector<T> t_allocated;
  vector<ptr_t> t_free;

  /* CTOR */
  pool() {
    clear();
  }

  /* getters */
  T* get(ptr_t idx) { return &t_allocated[size_ptr(idx)]; }
  T& operator[](ptr_t idx) { return t_allocated[size_ptr(idx)]; }

  ptr_t alloc(void) {
    if ( !t_free.empty() ) {
      auto res = ptr_t( t_free.back() );
      t_free.pop_back();
      return res;
    } else {
      //only insert until we reach size
      if ( t_allocated.size() <= SIZE ) {
        auto res = ptr_t( t_allocated.size() );
        t_allocated.push_back(T());
        return res;
      } else
      {
        throw std::bad_alloc();
      }
    }
  }

  void free( ptr_t idx ) { t_free.push_back(idx); }

  void clear() {
    t_allocated.clear();
    t_allocated.reserve(SIZE);
    t_free.clear();
    t_free.reserve(SIZE);
  }
};

#endif
